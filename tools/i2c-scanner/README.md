# i2c-scanner

- **Goal:** prove which I2C devices are actually reachable, continuously, while you rewire.
- **Board:** ELEGOO ESP32 (ESP-WROOM-32), core esp32 3.3.x
- **Libraries:** none beyond `Wire` (built into the core)
- **Status:** working (used to debug esp32-01's bus on 2026-10-21)

## Behaviour
- Scans addresses 0x08–0x77 at 100 kHz every 5 s.
- Names known parts (BH1750, SSD1306, BME280/BMP280, VL53L0X, MPU-6050).
- Flags devices listed in `EXPECTED[]` that didn't answer. Edit that array per node.

## Wiring
Uses the node's existing bus: SDA = GPIO21, SCL = GPIO22, all modules powered from 3V3 (3.3 V).

## Expected output (esp32-01)
```
[I2C] scan at 5 s
  0x23  BH1750 light (ADDR low/unconnected)
  0x3C  SSD1306 OLED
  0x76  BME280/BMP280 (SDO low)
[I2C] 3 device(s)
```

## Reading the result
| Symptom | Likely cause |
|---|---|
| Nothing found | shared fault: 3V3/GND, SDA/SCL swapped, or one module holding the bus low (reversed power, solder bridge) |
| One device missing, others fine | that module only: unsoldered/cold joints, its wires in the wrong breadboard strip, SDA/SCL swapped on it, ADDR pin |
| Device at an unexpected address | address pin (BH1750 ADDR high → 0x5C; BME280 SDO high → 0x77; some OLEDs 0x3D) |

Full procedure: [docs/sop/i2c-bring-up.md](../../docs/sop/i2c-bring-up.md).
