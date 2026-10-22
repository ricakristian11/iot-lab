// 03-bedroom-node.ino
//
// Bedroom sensor node for the apartment IoT hub.
//   - BME280: temperature / humidity / pressure (forced mode to limit self-heating)
//   - HC-SR501 PIR: motion, with a warm-up -> idle -> motion state machine
//   - SSD1306 OLED: local status screen that wakes on motion (prevents burn-in)
//   - BH1750 (optional): ambient light, driven with raw I2C commands, auto-detected
// Publishes JSON over MQTT to Mosquitto on the Raspberry Pi; firmware updates over Wi-Fi (OTA).
//
// Board: ESP32 Dev Module (ELEGOO ESP-WROOM-32), arduino-esp32 core 3.3.x
// Topics: see shared/topics.md. Wiring: see README.md in this folder.

#include <WiFi.h>
#include <esp_system.h>   // esp_reset_reason()
#include <Wire.h>
#include <ArduinoOTA.h>
#include <ArduinoMqttClient.h>
#include <ArduinoJson.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"

#if __has_include("secrets.h")
  #include "secrets.h"
#else
  #error "secrets.h missing: copy shared/secrets.example.h into this folder as secrets.h and fill it in"
#endif

// ---------------------------------------------------------------- globals
WiFiClient      wifiClient;
MqttClient      mqtt(wifiClient);
Adafruit_BME280 bme;
Adafruit_SSD1306 oled(128, 64, &Wire, -1);

bool bmeOk = false, oledOk = false, bh1750Ok = false;
bool otaStarted = false, wifiWasConnected = false;
char baseTopic[64];  // apartment/<ROOM>/<DEVICE_ID>

struct Readings { float t = NAN, h = NAN, p = NAN, lux = NAN; } readings;

enum class PirState : uint8_t { WARMUP, IDLE, MOTION };
PirState pirState    = PirState::WARMUP;
uint32_t lastMotionMs = 0;
bool     motionDirty  = true;  // publish motion state when it changes or after reconnect

enum class ScreenMode : uint8_t { AUTO, ALWAYS_ON, ALWAYS_OFF };
ScreenMode screenMode  = ScreenMode::AUTO;
bool       screenOn    = true;
bool       screenDirty = true;

uint32_t lastClimateMs = 0, lastMetaMs = 0, lastScreenMs = 0;
uint32_t lastWifiTryMs = 0, wifiRetryMs = WIFI_RETRY_MIN_MS;
uint32_t lastMqttTryMs = 0, mqttRetryMs = MQTT_RETRY_MIN_MS;

// ---------------------------------------------------------------- helpers
uint32_t backoff(uint32_t current) {  // 5 s -> 10 s -> 20 s -> 30 s cap
  return (current * 2 > RETRY_MAX_MS) ? RETRY_MAX_MS : current * 2;
}

void topicFor(char* out, size_t len, const char* leaf) {
  snprintf(out, len, "%s/%s", baseTopic, leaf);
}

bool i2cPresent(uint8_t addr) {
  Wire.beginTransmission(addr);
  return Wire.endTransmission() == 0;
}

const char* screenModeName() {
  switch (screenMode) {
    case ScreenMode::ALWAYS_ON:  return "on";
    case ScreenMode::ALWAYS_OFF: return "off";
    default:                     return "auto";
  }
}

// ---------------------------------------------------------------- BH1750 (raw I2C, no library)
// Datasheet commands: 0x01 power on, 0x07 reset data register, 0x10 continuous high-res mode.
// Result is 2 bytes, big-endian; lux = count / 1.2.
bool bh1750Command(uint8_t cmd) {
  Wire.beginTransmission(ADDR_BH1750);
  Wire.write(cmd);
  return Wire.endTransmission() == 0;
}

bool bh1750Begin() {
  return bh1750Command(0x01) && bh1750Command(0x07) && bh1750Command(0x10);
}

// Re-arm after every read. If the module's supply glitches (e.g. a wire is wiggled), the
// BH1750 restarts in power-down mode: it still ACKs on I2C but returns 0 forever.
// Power-on + continuous mode is harmless when it's already running; the next result is
// ready in ~120-180 ms, long before the next 30 s read. (No 0x07 reset here: that would
// clear the result register.)
void bh1750Rearm() {
  bh1750Command(0x01);
  bh1750Command(0x10);
}

float bh1750ReadLux() {
  if (Wire.requestFrom(ADDR_BH1750, (size_t)2) != 2) return NAN;
  const uint8_t hi = Wire.read();  // read order matters: high byte first
  const uint8_t lo = Wire.read();
  return ((uint16_t(hi) << 8) | lo) / 1.2f;
}

// ---------------------------------------------------------------- MQTT publishing
void publish(const char* leaf, const char* payload, bool retain = false, uint8_t qos = 0) {
  if (!mqtt.connected()) return;
  char topic[96];
  topicFor(topic, sizeof topic, leaf);
  mqtt.beginMessage(topic, (unsigned long)strlen(payload), retain, qos);
  mqtt.print(payload);
  mqtt.endMessage();
  LOG("[MQTT] pub %s %s\n", topic, payload);
}

void publishJson(const char* leaf, JsonDocument& doc, bool retain = false) {
  char buf[256];
  serializeJson(doc, buf, sizeof buf);
  publish(leaf, buf, retain);
}

void setOrNull(JsonDocument& doc, const char* key, float value, float scale) {
  if (isnan(value)) doc[key] = nullptr;             // keep JSON valid if a sensor read fails
  else doc[key] = roundf(value * scale) / scale;    // e.g. scale 100 -> 2 decimals
}

void publishClimate() {
  if (bmeOk) {
    JsonDocument d;
    setOrNull(d, "t", readings.t, 100);
    setOrNull(d, "h", readings.h, 10);
    setOrNull(d, "p", readings.p, 10);
    publishJson("climate", d);
  }
  if (bh1750Ok) {
    JsonDocument d;
    setOrNull(d, "lux", readings.lux, 10);
    publishJson("light", d);
  }
}

// Why the chip last started. Captured once in setup() and reported in meta, so an
// unexpected reboot seen on the hub can be told apart: power loss vs crash vs brownout.
// Note: on the ESP32 the EN button is a chip power-on reset, so it reports "poweron".
const char* bootReason = "unknown";

const char* resetReasonName(esp_reset_reason_t r) {
  switch (r) {
    case ESP_RST_POWERON:   return "poweron";    // power applied, USB replugged, or EN pressed
    case ESP_RST_SW:        return "software";   // esp_restart(), e.g. after an OTA update
    case ESP_RST_PANIC:     return "panic";      // crash (exception)
    case ESP_RST_INT_WDT:   return "int_wdt";    // interrupt watchdog
    case ESP_RST_TASK_WDT:  return "task_wdt";   // task watchdog: loop() blocked too long
    case ESP_RST_WDT:       return "wdt";        // other watchdog
    case ESP_RST_DEEPSLEEP: return "deepsleep";
    case ESP_RST_BROWNOUT:  return "brownout";   // supply voltage dipped
    default:                return "other";
  }
}

void publishMeta() {
  JsonDocument d;
  d["fw"]       = FW_VERSION;
  d["ip"]       = WiFi.localIP().toString();
  d["rssi"]     = WiFi.RSSI();
  d["uptime_s"] = millis() / 1000;
  d["heap"]     = ESP.getFreeHeap();
  d["bme280"]   = bmeOk;
  d["oled"]     = oledOk;
  d["bh1750"]   = bh1750Ok;
  d["reset"]    = bootReason;
  publishJson("meta", d, true);
}

void publishScreenState() {
  JsonDocument d;
  d["mode"] = screenModeName();
  d["on"]   = screenOn;
  publishJson("screen", d, true);
}

// ---------------------------------------------------------------- MQTT commands
// apartment/<room>/<device>/cmd/screen  payload: on | off | auto
void onMqttMessage(int size) {
  (void)size;  // payload length; we read until available() is empty
  const String topic = mqtt.messageTopic();
  char payload[16];
  size_t n = 0;
  while (mqtt.available()) {
    const int c = mqtt.read();
    if (n < sizeof(payload) - 1) payload[n++] = (char)c;  // drain anything longer
  }
  payload[n] = '\0';
  LOG("[MQTT] cmd %s -> %s\n", topic.c_str(), payload);

  if (topic.endsWith("/cmd/screen")) {
    if      (strcmp(payload, "on") == 0)   screenMode = ScreenMode::ALWAYS_ON;
    else if (strcmp(payload, "off") == 0)  screenMode = ScreenMode::ALWAYS_OFF;
    else if (strcmp(payload, "auto") == 0) screenMode = ScreenMode::AUTO;
    else { LOG("[MQTT] unknown screen mode '%s'\n", payload); return; }
    publishScreenState();
  }
}

// ---------------------------------------------------------------- connectivity
void wifiEnsure(uint32_t now) {
  const bool connected = WiFi.status() == WL_CONNECTED;
  if (connected != wifiWasConnected) {
    wifiWasConnected = connected;
    if (connected) LOG("[WIFI] connected, ip %s, rssi %d\n", WiFi.localIP().toString().c_str(), WiFi.RSSI());
    else           LOG("[WIFI] lost connection\n");
    screenDirty = true;
  }
  if (connected) { wifiRetryMs = WIFI_RETRY_MIN_MS; return; }
  if (now - lastWifiTryMs < wifiRetryMs) return;
  lastWifiTryMs = now;
  LOG("[WIFI] connecting to %s (next retry in %lu s)\n", WIFI_SSID, (unsigned long)(wifiRetryMs / 1000));
  WiFi.disconnect();
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  wifiRetryMs = backoff(wifiRetryMs);
}

void mqttEnsure(uint32_t now) {
  if (WiFi.status() != WL_CONNECTED || mqtt.connected()) return;
  if (now - lastMqttTryMs < mqttRetryMs) return;
  lastMqttTryMs = now;

  char willTopic[96];
  topicFor(willTopic, sizeof willTopic, "status");
  mqtt.setId(DEVICE_ID);
  mqtt.setUsernamePassword(MQTT_USER, MQTT_PASS);
  mqtt.setKeepAliveInterval(MQTT_KEEPALIVE_MS);
  mqtt.setConnectionTimeout(MQTT_CONNECT_TIMEOUT_MS);
  mqtt.beginWill(willTopic, true, 1);  // broker publishes "offline" (retained) if we vanish
  mqtt.print("offline");
  mqtt.endWill();

  LOG("[MQTT] connecting to %s:%d ... ", MQTT_HOST, MQTT_PORT);
  if (!mqtt.connect(MQTT_HOST, MQTT_PORT)) {
    // 5 = not authorised (check MQTT_USER/MQTT_PASS), -2 = broker unreachable
    LOG("failed (error %d), retry in %lu s\n", mqtt.connectError(), (unsigned long)(mqttRetryMs / 1000));
    mqttRetryMs = backoff(mqttRetryMs);
    return;
  }
  LOG("ok\n");
  mqttRetryMs = MQTT_RETRY_MIN_MS;

  publish("status", "online", true);
  char sub[96];
  topicFor(sub, sizeof sub, "cmd/#");
  mqtt.subscribe(sub, 1);
  motionDirty = true;  // re-send retained state after every reconnect
  publishMeta();
  publishScreenState();
  publishClimate();
  screenDirty = true;
}

void otaEnsure() {
  if (otaStarted || WiFi.status() != WL_CONNECTED) return;
  ArduinoOTA.setHostname(DEVICE_ID);
  ArduinoOTA.setPassword(OTA_PASS);
  ArduinoOTA.onStart([]() {
    LOG("[OTA] update starting\n");
    if (!oledOk) return;
    oled.ssd1306_command(SSD1306_DISPLAYON);
    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setCursor(0, 4);
    oled.print("Firmware update...");
    oled.display();
  });
  ArduinoOTA.onProgress([](unsigned int done, unsigned int total) {
    static uint8_t lastPct = 255;
    const uint8_t pct = total ? (uint8_t)(done * 100UL / total) : 0;
    if (!oledOk || pct == lastPct || pct % 5 != 0) return;  // redraw every 5 %
    lastPct = pct;
    oled.fillRect(0, 30, 128, 12, SSD1306_BLACK);
    oled.drawRect(0, 30, 128, 12, SSD1306_WHITE);
    oled.fillRect(2, 32, (124 * pct) / 100, 8, SSD1306_WHITE);
    oled.display();
  });
  ArduinoOTA.onEnd([]() { LOG("[OTA] done, rebooting\n"); });
  ArduinoOTA.onError([](ota_error_t err) { LOG("[OTA] error %u\n", (unsigned)err); });
  ArduinoOTA.begin();
  otaStarted = true;
  LOG("[OTA] ready: %s at %s\n", DEVICE_ID, WiFi.localIP().toString().c_str());
}

// ---------------------------------------------------------------- sensors
void readSensors() {
  if (bmeOk && bme.takeForcedMeasurement()) {  // one measurement, then the sensor sleeps
    readings.t = bme.readTemperature();
    readings.h = bme.readHumidity();
    readings.p = bme.readPressure() / 100.0f;  // Pa -> hPa
  }
  if (bh1750Ok) {
    readings.lux = bh1750ReadLux();
    bh1750Rearm();
  }
  screenDirty = true;
}

// PIR state machine: WARMUP (ignore output) -> IDLE <-> MOTION
void updatePir(uint32_t now) {
  const bool high = digitalRead(PIN_PIR) == HIGH;
  switch (pirState) {
    case PirState::WARMUP:
      if (now >= PIR_WARMUP_MS) {
        pirState = high ? PirState::MOTION : PirState::IDLE;
        if (high) lastMotionMs = now;
        motionDirty = screenDirty = true;
        LOG("[SENSOR] PIR ready\n");
      }
      break;
    case PirState::IDLE:
      if (high) {
        pirState = PirState::MOTION;
        lastMotionMs = now;
        motionDirty = screenDirty = true;
        LOG("[SENSOR] motion start\n");
      }
      break;
    case PirState::MOTION:
      lastMotionMs = now;  // keep the screen awake while motion continues
      if (!high) {
        pirState = PirState::IDLE;
        motionDirty = screenDirty = true;
        LOG("[SENSOR] motion end\n");
      }
      break;
  }
  digitalWrite(PIN_LED, pirState == PirState::MOTION ? HIGH : LOW);

  if (motionDirty && pirState != PirState::WARMUP && mqtt.connected()) {
    JsonDocument d;
    d["motion"] = pirState == PirState::MOTION ? 1 : 0;
    publishJson("motion", d, true);  // retained: dashboards show current state after restarts
    motionDirty = false;
  }
}

// ---------------------------------------------------------------- display
void drawScreen() {
  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);

  // Status line (yellow band on two-colour modules: top 16 pixel rows)
  oled.setTextSize(1);
  oled.setCursor(0, 4);
  oled.print(DEVICE_ID);
  oled.print(' ');
  if (mqtt.connected())                  oled.print("online");
  else if (WiFi.status() == WL_CONNECTED) oled.print("no mqtt");
  else                                    oled.print("no wifi");

  // Big readings on the left
  oled.setTextSize(2);
  oled.setCursor(0, 20);
  if (isnan(readings.t)) oled.print("--.-C"); else oled.printf("%.1fC", readings.t);
  oled.setCursor(0, 44);
  if (isnan(readings.h)) oled.print("--%");  else oled.printf("%.0f%%", readings.h);

  // Small readings on the right
  oled.setTextSize(1);
  oled.setCursor(76, 20);
  if (!isnan(readings.p)) oled.printf("%.0fhPa", readings.p);
  if (bh1750Ok) {
    oled.setCursor(76, 32);
    if (isnan(readings.lux)) oled.print("-- lx"); else oled.printf("%.0flx", readings.lux);
  }
  oled.setCursor(76, 48);
  switch (pirState) {
    case PirState::WARMUP: oled.print("warmup"); break;
    case PirState::MOTION: oled.print("MOTION"); break;
    case PirState::IDLE:   oled.print("clear");  break;
  }
  oled.display();
}

// Screen policy: AUTO = on during PIR warm-up and for SCREEN_TIMEOUT_MS after the last motion.
void updateScreen(uint32_t now) {
  if (!oledOk) return;
  bool want;
  switch (screenMode) {
    case ScreenMode::ALWAYS_ON:  want = true;  break;
    case ScreenMode::ALWAYS_OFF: want = false; break;
    default:
      want = pirState == PirState::WARMUP || (now - lastMotionMs < SCREEN_TIMEOUT_MS);
  }
  if (want != screenOn) {
    screenOn = want;
    oled.ssd1306_command(want ? SSD1306_DISPLAYON : SSD1306_DISPLAYOFF);
    LOG("[OLED] screen %s\n", want ? "on" : "off");
    publishScreenState();
    screenDirty = true;
  }
  if (!screenOn) return;
  if (!screenDirty && now - lastScreenMs < SCREEN_REFRESH_MS) return;
  lastScreenMs = now;
  screenDirty = false;
  drawScreen();
}

// ---------------------------------------------------------------- setup / loop
void setup() {
  Serial.begin(115200);
  bootReason = resetReasonName(esp_reset_reason());
  LOG("\n[BOOT] %s/%s fw %s (reset: %s)\n", ROOM, DEVICE_ID, FW_VERSION, bootReason);
  snprintf(baseTopic, sizeof baseTopic, "apartment/%s/%s", ROOM, DEVICE_ID);

  pinMode(PIN_PIR, INPUT);
  pinMode(PIN_LED, OUTPUT);
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  Wire.setClock(400000);  // all three I2C parts support 400 kHz fast mode

  bmeOk = bme.begin(ADDR_BME280, &Wire);
  if (bmeOk) {
    LOG("[SENSOR] BME280 found, chip id 0x%02X\n", (unsigned)bme.sensorID());  // 0x60 = genuine BME280
    // Bosch "weather monitoring" settings: forced mode, x1 oversampling, no filter -> minimal self-heating
    bme.setSampling(Adafruit_BME280::MODE_FORCED,
                    Adafruit_BME280::SAMPLING_X1, Adafruit_BME280::SAMPLING_X1,
                    Adafruit_BME280::SAMPLING_X1, Adafruit_BME280::FILTER_OFF);
  } else {
    LOG("[SENSOR] BME280 not found at 0x%02X\n", ADDR_BME280);
  }

  oledOk = oled.begin(SSD1306_SWITCHCAPVCC, ADDR_OLED, true, false);  // Wire already started
  LOG("[OLED] %s\n", oledOk ? "ready" : "not found");

  bh1750Ok = i2cPresent(ADDR_BH1750) && bh1750Begin();
  LOG("[SENSOR] BH1750 %s\n", bh1750Ok ? "found" : "not fitted");

  WiFi.mode(WIFI_STA);
  WiFi.setHostname(DEVICE_ID);
  WiFi.setScanMethod(WIFI_ALL_CHANNEL_SCAN);       // router + extender share one SSID:
  WiFi.setSortMethod(WIFI_CONNECT_AP_BY_SIGNAL);   // scan all channels, join the strongest
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  lastWifiTryMs = millis();

  mqtt.onMessage(onMqttMessage);
  readSensors();
  lastClimateMs = millis();
}

void loop() {
  const uint32_t now = millis();

  wifiEnsure(now);
  otaEnsure();
  if (otaStarted) ArduinoOTA.handle();
  mqttEnsure(now);
  mqtt.poll();  // keep-alive + incoming commands

  updatePir(now);

  if (now - lastClimateMs >= CLIMATE_PERIOD_MS) {
    lastClimateMs = now;
    readSensors();
    publishClimate();
  }
  if (now - lastMetaMs >= META_PERIOD_MS) {
    lastMetaMs = now;
    publishMeta();
  }

  updateScreen(now);
}
