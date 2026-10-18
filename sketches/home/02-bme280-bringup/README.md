# 02-bme280-bringup

- **Goal:** bring up a new I2C sensor methodically: find it on the bus, confirm its identity from a register, then read it. Cheap "BME280" modules are often BMP280s (no humidity), so the chip ID matters.
- **Board:** ELEGOO ESP32 (ESP-WROOM-32), core esp32 3.3.x
- **Libraries:** Adafruit BME280 Library 2.3.0 (+ Adafruit Unified Sensor 1.1.15, Adafruit BusIO 1.17.4)
- **Status:** working; module verified as a genuine BME280 (chip ID 0x60)

## Wiring
Unplug USB before wiring. Header soldered onto the module first.

| Part pin | ESP32 GPIO | Voltage | Notes |
|---|---|---|---|
| VCC | 3V3 | 3.3 V | 3.3 V-only module |
| GND | GND | 0 V | |
| SCL | GPIO22 | 3.3 V logic | I2C clock |
| SDA | GPIO21 | 3.3 V logic | I2C data |
| CSB | 3V3 | 3.3 V | high selects I2C (low = SPI) |
| SDO | GND | 0 V | low = address 0x76, high = 0x77; never leave floating |

## Expected output
```
[I2C] device at 0x76
[SENSOR] 0x76: chip id 0x60 -> BME280 (temperature, humidity, pressure)
[SENSOR] 0x77: no response
[SENSOR] T=26.94 C  RH=36.0 %  P=1019.8 hPa
```

## Lessons / gotchas
- The chip-ID read uses a repeated start (`endTransmission(false)`), so the register pointer write and the read happen in one bus transaction.
- Breathing on the sensor should make humidity jump within seconds, which is a quick functional test.
