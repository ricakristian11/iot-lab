// config.h — non-secret configuration for the bedroom node.
// Change ROOM / DEVICE_ID here to reuse this sketch for another room.
#pragma once
#include <Arduino.h>

// ---------- Identity (drives every MQTT topic: apartment/<ROOM>/<DEVICE_ID>/...) ----------
constexpr char ROOM[]       = "bedroom";
constexpr char DEVICE_ID[]  = "esp32-01";
constexpr char FW_VERSION[] = "0.3.1";

// ---------- Pins (ELEGOO ESP-WROOM-32 30-pin dev board) ----------
constexpr uint8_t PIN_I2C_SDA = 21;  // shared I2C bus: BME280, OLED, BH1750
constexpr uint8_t PIN_I2C_SCL = 22;
constexpr uint8_t PIN_PIR     = 33;  // HC-SR501 OUT (3.3 V logic). ADC1 pin, not a strapping pin
constexpr uint8_t PIN_LED     = 2;   // onboard LED (strapping pin, but safe as an output)

// ---------- I2C addresses ----------
constexpr uint8_t ADDR_BME280 = 0x76;  // SDO tied to GND
constexpr uint8_t ADDR_OLED   = 0x3C;  // SSD1306 128x64
constexpr uint8_t ADDR_BH1750 = 0x23;  // ADDR pin to GND (or floating)

// ---------- Timing (all non-blocking, millis()-based) ----------
constexpr uint32_t CLIMATE_PERIOD_MS   = 30000;   // sensor read + publish
constexpr uint32_t META_PERIOD_MS      = 60000;   // uptime / RSSI heartbeat
constexpr uint32_t PIR_WARMUP_MS       = 60000;   // HC-SR501 settles after power-on
constexpr uint32_t SCREEN_TIMEOUT_MS   = 60000;   // OLED stays on this long after last motion
constexpr uint32_t SCREEN_REFRESH_MS   = 5000;    // plus an immediate redraw whenever state changes
constexpr uint32_t WIFI_RETRY_MIN_MS   = 10000;   // retries back off: min, x2, ... up to max
constexpr uint32_t MQTT_RETRY_MIN_MS   = 5000;
constexpr uint32_t RETRY_MAX_MS        = 30000;
constexpr uint32_t MQTT_KEEPALIVE_MS   = 30000;   // broker declares us offline after ~1.5x this
constexpr uint32_t MQTT_CONNECT_TIMEOUT_MS = 3000;  // library default is 30 s, which blocks loop()

// ---------- Logging ----------
#define DEBUG 1
#if DEBUG
  #define LOG(...) Serial.printf(__VA_ARGS__)
#else
  #define LOG(...)
#endif
