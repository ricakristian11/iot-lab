// i2c-scanner.ino
// Scans the I2C bus every 5 s, names each device it finds, and flags the
// ones esp32-01 expects but can't see. Use it after any wiring change.
// Board: ESP32 Dev Module (ELEGOO ESP-WROOM-32), arduino-esp32 core 3.3.x
// No Wi-Fi/OTA in this sketch: re-flash 03-bedroom-node over USB afterwards.

#include <Wire.h>

constexpr uint8_t  PIN_I2C_SDA    = 21;
constexpr uint8_t  PIN_I2C_SCL    = 22;
constexpr uint32_t I2C_CLOCK_HZ   = 100000;  // 100 kHz: slow and forgiving for bring-up
constexpr uint32_t SCAN_PERIOD_MS = 5000;

struct KnownDevice { uint8_t addr; const char* name; };

constexpr KnownDevice KNOWN[] = {
  {0x23, "BH1750 light (ADDR low/unconnected)"},
  {0x5C, "BH1750 light (ADDR high)"},
  {0x3C, "SSD1306 OLED"},
  {0x3D, "SSD1306 OLED (alt address)"},
  {0x76, "BME280/BMP280 (SDO low)"},
  {0x77, "BME280/BMP280 (SDO high)"},
  {0x29, "VL53L0X distance"},
  {0x68, "MPU-6050 IMU"},
};

// Devices wired to esp32-01's bus
constexpr uint8_t EXPECTED[] = {0x23, 0x3C, 0x76};

uint32_t lastScanMs = 0;
bool     firstScan  = true;

const char* nameFor(uint8_t addr) {
  for (const auto& d : KNOWN) {
    if (d.addr == addr) return d.name;
  }
  return "unknown device";
}

void scanBus() {
  bool present[128] = {};
  uint8_t found = 0;

  Serial.printf("\n[I2C] scan at %lu s\n", (unsigned long)(millis() / 1000));
  for (uint8_t addr = 0x08; addr <= 0x77; addr++) {  // valid 7-bit address range
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {                 // 0 = device ACKed its address
      present[addr] = true;
      found++;
      Serial.printf("  0x%02X  %s\n", addr, nameFor(addr));
    }
  }

  if (found == 0) {
    Serial.println("  nothing found: check 3V3 + GND, SDA(21)/SCL(22) not swapped, solder joints");
    return;
  }

  Serial.printf("[I2C] %u device(s)\n", found);
  for (uint8_t addr : EXPECTED) {
    if (!present[addr]) {
      Serial.printf("  MISSING 0x%02X (%s): check that module's 4 wires\n", addr, nameFor(addr));
    }
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL, I2C_CLOCK_HZ);
  Serial.println("\n[BOOT] i2c-scanner");
}

void loop() {
  const uint32_t now = millis();
  if (firstScan || now - lastScanMs >= SCAN_PERIOD_MS) {
    firstScan  = false;
    lastScanMs = now;
    scanBus();
  }
}
