# Case study: OLED and BH1750 not detected on the I2C bus

| | |
|---|---|
| **Date** | 2026-10-21 |
| **System** | esp32-01 (bedroom node), breadboard |
| **Outcome** | fixed: all three devices detected (0x23, 0x3C, 0x76) |
| **Time spent** | ~1.5 h |

## Symptom
After adding an SSD1306 OLED (0x3C) and a BH1750 (0x23) to a bus that already had a working BME280 (0x76):
- A scanner sketch from an older download found **no devices at all**.
- The OLED stayed blank.

## Context
- Both new modules were wired the same evening.
- The breadboard is several snap-together pieces; the ESP32 straddles two of them.
- Both modules were connected with female-to-male jumpers because the board was out of space.

## Hypotheses (in the order considered)
| # | Hypothesis | Test | Result |
|---|---|---|---|
| 1 | Shared wiring broken, or one module holding the bus low | Flashed `tools/i2c-scanner`, removed the OLED, rescanned | BME280 found → shared wiring OK |
| 2 | BH1750 header joints bad | Reflowed all joints | Still missing |
| 3 | BH1750 jumpers not in the bus strips | Continuity module pin → ESP32 pin: no beep; traced the wires | **Confirmed**: strips on a different board piece |
| 4 | OLED: same as #3 | Moved its jumpers into the bus strips | **Confirmed**: 0x3C detected |

## Root cause
Both modules' male jumper ends sat in strips that looked aligned with the bus rows but were on a **separate breadboard piece**, so they connected to nothing.

The first "no devices" result came from a different scanner sketch whose configuration wasn't checked. It couldn't be reproduced with `tools/i2c-scanner`.

## Fix
Plugged every new module's jumpers into the **same strips as the BME280's wires**, which were known to work.

## Lessons
- **Know what firmware is running before trusting its output.** One scanner, with known pins, in the repo.
- **Change one variable at a time.** The scanner sketch was swapped *and* the OLED removed in the same step, so the fix can't be attributed to either. Same rule as a commit that changes two things.
- **Snap-together breadboard pieces don't share rows.** Join a bus by plugging into a known-good module's strips.
- **Test continuity one segment at a time** (joint, strip, wire), not just end to end.
- **One device missing = that device's path only.** All devices missing = a shared fault or one device holding the bus.
- **A blank SSD1306 is normal until firmware initializes it.** No light ≠ no power.
- **Ribbon jumper colours mean nothing** (red was SDA). Label them.

## Evidence
- Serial logs from `tools/i2c-scanner` (before/after).
- Breadboard photos → `docs/img/2026-10-21-i2c-*.jpg`.
