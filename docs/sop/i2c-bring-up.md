# SOP: I2C bring-up (adding a module to a bus)

| | |
|---|---|
| **Applies to** | ESP32 I2C bus on GPIO21 (SDA) / GPIO22 (SCL), 3.3 V modules |
| **Time** | 10 min per module |
| **Last verified** | 2026-10-21 on esp32-01 |

## Concept
- I2C is a shared two-wire bus; every device has an address (like hostnames on one network).
- Both lines idle **high** (pull-up resistors to 3.3 V). Any device can pull them low. A faulty device that holds a line low silences **every** device: like one client holding a lock on a shared resource.

## Safety
- Unplug USB before rewiring. Power modules from **3V3**, not 5 V, so their pull-ups stay at 3.3 V.

## Steps
1. Wire the new module into the bus strips ([breadboard SOP](breadboard-a-node.md)).
2. Flash [`tools/i2c-scanner`](../../tools/i2c-scanner); add the module's address to `EXPECTED[]`.
3. Serial Monitor 115200. **Expect:** the new address plus all previous ones.
4. Where the part has an ID register, confirm identity (e.g. BME280 register `0xD0` = `0x60`; BMP280 = `0x58`). See `sketches/home/02-bme280-bringup`.
5. Re-flash the node sketch over USB (the scanner has no OTA).

## When it fails: bisect
1. **Everything vanished** → the new module (or a disturbed shared wire) is holding the bus. Remove modules one at a time, rescanning each time, until the bus returns. The last one removed is the culprit.
2. **Only the new module is missing** → its own path:
   1. Continuity (USB unplugged), one segment at a time: module pad → header pin (joint) → jumper tip in the strip (breadboard) → ESP32 pin.
   2. Voltages (USB plugged in, black probe on GND): module VCC ≈ 3.3 V; SDA and SCL ≈ 3.3 V idle. ~0 V on SDA/SCL = something shorting it low.
   3. Address pins: BH1750 ADDR high → 0x5C; BME280 SDO high → 0x77.
   4. Swap in a spare module.
3. Multimeter (KAIWEETS HT118A): black lead COM, red VΩHz. Continuity = `Ω·)))` + FUNC until `·)))` shows (touch the tips together first: it must beep). DC volts = `V` position, display shows DC.

## Known addresses
| Part | Address |
|---|---|
| BH1750 | 0x23 (ADDR low) / 0x5C (high) |
| SSD1306 OLED | 0x3C (some 0x3D) |
| BME280 / BMP280 | 0x76 (SDO low) / 0x77 |
| VL53L0X | 0x29 |
| MPU-6050 | 0x68 |
