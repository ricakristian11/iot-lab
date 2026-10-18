// 01-blink.ino
// First toolchain check: blink the onboard LED without delay(), so loop() never blocks.
// Board: ESP32 Dev Module (ELEGOO ESP-WROOM-32), arduino-esp32 core 3.3.x

#include <Arduino.h>

constexpr uint8_t  PIN_LED   = 2;    // onboard LED on this board
constexpr uint32_t PERIOD_MS = 500;  // half-period: 1 Hz blink

uint32_t lastToggleMs = 0;
bool ledOn = false;

void setup() {
  Serial.begin(115200);
  pinMode(PIN_LED, OUTPUT);
  Serial.println("[BOOT] 01-blink");
}

void loop() {
  const uint32_t now = millis();
  if (now - lastToggleMs >= PERIOD_MS) {  // unsigned subtraction is rollover-safe
    lastToggleMs = now;
    ledOn = !ledOn;
    digitalWrite(PIN_LED, ledOn ? HIGH : LOW);
  }
  // other non-blocking work would go here
}
