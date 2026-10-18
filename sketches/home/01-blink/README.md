# 01-blink

- **Goal:** prove the toolchain end to end (IDE → USB driver → bootloader → board) with the simplest possible output.
- **Board:** ELEGOO ESP32 (ESP-WROOM-32), core esp32 3.3.x
- **Libraries:** none
- **Status:** working

## Wiring
| Part pin | ESP32 GPIO | Voltage | Notes |
|---|---|---|---|
| onboard LED | GPIO2 | 3.3 V | no external wiring |

## How to run
1. Board **ESP32 Dev Module**, select the COM port that appears when the board is plugged in.
2. Upload (hold **BOOT** during "Connecting…" if the upload stalls). The LED blinks at 1 Hz.

## Lessons / gotchas
- No COM port → install the Silicon Labs CP210x driver; use a USB **data** cable.
- Uses `millis()` instead of `delay()` so the same pattern scales to sketches that do several things at once.
