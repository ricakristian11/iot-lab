# SOP: Breadboard a node

| | |
|---|---|
| **Applies to** | ESP32 DevKit + I2C/GPIO modules on solderless breadboards |
| **Time** | 15–30 min |
| **Last verified** | 2026-10-21 on esp32-01 (BME280 + BH1750 + PIR) |

## Safety
- **Unplug USB before every wiring change.**
- After power-up, touch each module for a few seconds: **warm = reversed power → unplug immediately.**
- Soldering headers: ventilate (flux fumes), iron ~330–350 °C for leaded solder.

## How breadboards connect
- Only the **same 5-hole strip** is connected: same row number, same side of the centre gap (a–e or f–j), **same board piece**.
- Snap-together boards do **not** share rows or rails between pieces.
- A header pressed across a–e of one row shorts every pin together. Headers run along the numbered rows.
- The 30-pin ESP32 is wide; spanning two board pieces leaves free holes on both sides.

## Power domains
| Rail | Source | Feeds |
|---|---|---|
| 3.3 V | ESP32 `3V3` | I2C modules (BME280, OLED, BH1750); keeps their pull-ups at 3.3 V |
| 5 V | ESP32 `VIN` (USB 5 V) | HC-SR501 PIR power only (its OUT is 3.3 V) |
| GND | any ESP32 `GND` | everything (common ground) |

## Steps
1. Solder headers onto each module (one pin first, check alignment, then the rest). Inspect joints under the microscope: shiny cones, no bridges.
2. Seat the ESP32; note which strips its 3V3, GND, D21, D22 pins land in.
3. Wire power and ground first, then signals.
4. **Read each module's printed labels.** Pin orders differ (OLEDs come VCC-first or GND-first; BH1750 is VCC-GND-SCL-SDA-ADDR).
5. For a module added to an existing bus, plug its wires into the **same strips as a known-good module's wires**.
6. Read the label next to each ESP32 pin; never count positions (D33 vs D35 mix-up).
7. Pre-power checks (USB unplugged, multimeter on continuity):
   - [ ] 3V3 ↔ GND: **no** beep
   - [ ] each module pin ↔ its ESP32 pin: beep
8. Power up, run [`tools/i2c-scanner`](../../tools/i2c-scanner) or the node sketch.

## Troubleshooting
| Symptom | Likely cause | Fix |
|---|---|---|
| Module silent, others OK | wire in a strip on another board piece / other side of the gap | follow each wire to its strip |
| Whole I2C bus silent | one module pulling SDA/SCL low (reversed power, bridge) | remove modules until the bus returns, see [i2c-bring-up](i2c-bring-up.md) |
| PIR never triggers | OUT wire in the neighbouring pin's row | match the silkscreen label |
