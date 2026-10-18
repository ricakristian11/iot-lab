// secrets.example.h — committed template.
// Copy this file into a sketch folder as `secrets.h` and fill in real values.
// `secrets.h` is gitignored; never commit it.
#pragma once

#define WIFI_SSID  "your-ssid"          // 2.4 GHz network (ESP32 has no 5 GHz radio)
#define WIFI_PASS  "your-password"

#define MQTT_HOST  "192.168.1.10"       // hub IP; ESP32 can't resolve .local names by default
#define MQTT_PORT  1883
#define MQTT_USER  "esp32node"
#define MQTT_PASS  "change-me"

#define OTA_PASS   "change-me"          // password the Arduino IDE asks for on network uploads
