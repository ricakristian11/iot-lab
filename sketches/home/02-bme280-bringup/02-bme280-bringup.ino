// 02-bme280-bringup.ino
// Hardware bring-up for a new I2C sensor, in three steps:
//   1. scan the bus and list every responding address
//   2. read the BME280/BMP280 chip-ID register (0xD0) to confirm what was actually shipped
//   3. stream readings every 2 s
// Board: ESP32 Dev Module (ELEGOO ESP-WROOM-32), arduino-esp32 core 3.3.x

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

constexpr uint8_t  PIN_I2C_SDA     = 21;
constexpr uint8_t  PIN_I2C_SCL     = 22;
constexpr uint8_t  REG_CHIP_ID     = 0xD0;  // Bosch datasheet: "id" register
constexpr uint8_t  ID_BME280       = 0x60;
constexpr uint8_t  ID_BMP280       = 0x58;
constexpr uint32_t READ_PERIOD_MS  = 2000;

Adafruit_BME280 bme;
uint8_t  bmeAddr = 0;
uint32_t lastReadMs = 0;

void scanBus() {
  Serial.println("[I2C] scanning...");
  uint8_t found = 0;
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.printf("[I2C] device at 0x%02X\n", addr);
      found++;
    }
  }
  if (!found) Serial.println("[I2C] nothing found: check 3V3/GND, SDA/SCL swap, solder joints");
}

int readRegister(uint8_t addr, uint8_t reg) {
  Wire.beginTransmission(addr);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return -1;  // repeated start, keep the bus
  if (Wire.requestFrom(addr, (size_t)1) != 1) return -1;
  return Wire.read();
}

void identify() {
  for (uint8_t addr : {uint8_t(0x76), uint8_t(0x77)}) {
    const int id = readRegister(addr, REG_CHIP_ID);
    if (id < 0) { Serial.printf("[SENSOR] 0x%02X: no response\n", addr); continue; }
    Serial.printf("[SENSOR] 0x%02X: chip id 0x%02X -> ", addr, id);
    if (id == ID_BME280)      { Serial.println("BME280 (temperature, humidity, pressure)"); bmeAddr = addr; }
    else if (id == ID_BMP280) Serial.println("BMP280 (no humidity!)");
    else                      Serial.println("unknown device");
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  Serial.println("\n[BOOT] 02-bme280-bringup");
  scanBus();
  identify();
  if (bmeAddr && !bme.begin(bmeAddr, &Wire)) {
    Serial.println("[SENSOR] BME280 init failed");
    bmeAddr = 0;
  }
}

void loop() {
  const uint32_t now = millis();
  if (!bmeAddr || now - lastReadMs < READ_PERIOD_MS) return;
  lastReadMs = now;
  Serial.printf("[SENSOR] T=%.2f C  RH=%.1f %%  P=%.1f hPa\n",
                bme.readTemperature(), bme.readHumidity(), bme.readPressure() / 100.0f);
}
