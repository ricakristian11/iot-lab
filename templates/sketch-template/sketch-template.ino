// sketch-template.ino
// Starting point for a new MQTT node. Copy the folder, rename folder + .ino to match,
// set ROOM / DEVICE_ID, add sensors, and copy shared/secrets.example.h in as secrets.h.
// Board: ESP32 Dev Module, arduino-esp32 core 3.3.x

#include <WiFi.h>
#include <ArduinoOTA.h>
#include <ArduinoMqttClient.h>
#include <ArduinoJson.h>

#if __has_include("secrets.h")
  #include "secrets.h"
#else
  #error "secrets.h missing: copy shared/secrets.example.h into this folder as secrets.h"
#endif

constexpr char     ROOM[]          = "room";
constexpr char     DEVICE_ID[]     = "esp32-xx";
constexpr uint8_t  PIN_LED         = 2;
constexpr uint32_t PUBLISH_MS      = 30000;
constexpr uint32_t RETRY_MS        = 5000;

#define LOG(...) Serial.printf(__VA_ARGS__)

WiFiClient wifiClient;
MqttClient mqtt(wifiClient);
char baseTopic[64];
uint32_t lastPublishMs = 0, lastMqttTryMs = 0;
bool otaStarted = false;

void publish(const char* leaf, const char* payload, bool retain = false) {
  char topic[96];
  snprintf(topic, sizeof topic, "%s/%s", baseTopic, leaf);
  mqtt.beginMessage(topic, (unsigned long)strlen(payload), retain, 0);
  mqtt.print(payload);
  mqtt.endMessage();
}

void onMqttMessage(int) {
  const String topic = mqtt.messageTopic();
  String payload;
  while (mqtt.available()) payload += (char)mqtt.read();
  LOG("[MQTT] cmd %s -> %s\n", topic.c_str(), payload.c_str());
  if (topic.endsWith("/cmd/led")) digitalWrite(PIN_LED, payload == "on" ? HIGH : LOW);
}

void mqttEnsure(uint32_t now) {
  if (WiFi.status() != WL_CONNECTED || mqtt.connected()) return;
  if (now - lastMqttTryMs < RETRY_MS) return;
  lastMqttTryMs = now;
  char status[96];
  snprintf(status, sizeof status, "%s/status", baseTopic);
  mqtt.setId(DEVICE_ID);
  mqtt.setUsernamePassword(MQTT_USER, MQTT_PASS);
  mqtt.setConnectionTimeout(3000);
  mqtt.beginWill(status, true, 1);
  mqtt.print("offline");
  mqtt.endWill();
  if (!mqtt.connect(MQTT_HOST, MQTT_PORT)) { LOG("[MQTT] connect failed (%d)\n", mqtt.connectError()); return; }
  publish("status", "online", true);
  char sub[96];
  snprintf(sub, sizeof sub, "%s/cmd/#", baseTopic);
  mqtt.subscribe(sub, 1);
  LOG("[MQTT] connected\n");
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_LED, OUTPUT);
  snprintf(baseTopic, sizeof baseTopic, "apartment/%s/%s", ROOM, DEVICE_ID);
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(DEVICE_ID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  mqtt.onMessage(onMqttMessage);
}

void loop() {
  const uint32_t now = millis();
  if (!otaStarted && WiFi.status() == WL_CONNECTED) {
    ArduinoOTA.setHostname(DEVICE_ID);
    ArduinoOTA.setPassword(OTA_PASS);
    ArduinoOTA.begin();
    otaStarted = true;
  }
  if (otaStarted) ArduinoOTA.handle();
  mqttEnsure(now);
  mqtt.poll();

  if (mqtt.connected() && now - lastPublishMs >= PUBLISH_MS) {
    lastPublishMs = now;
    JsonDocument d;
    d["uptime_s"] = now / 1000;  // replace with sensor readings
    char buf[128];
    serializeJson(d, buf, sizeof buf);
    publish("uptime", buf);
  }
}
