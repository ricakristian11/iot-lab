# SOP: Flash an ESP32 node (USB and OTA)

| | |
|---|---|
| **Applies to** | ELEGOO ESP32 DevKit (ESP-WROOM-32, CP2102 USB-UART), 30-pin |
| **Time** | 5 min (USB) · 1 min (OTA) |
| **Last verified** | 2026-10-17 on esp32-01 |

## Safety
- Unplug USB before changing any wiring.
- The board's logic is 3.3 V. Never feed 5 V into a GPIO.

## Tools and parts
- USB-C to USB-A **data** cable (charge-only cables power the board but never show a COM port)
- Arduino IDE 2.x with **esp32 by Espressif** 3.3.x

## One-time setup (Windows)
1. Install the **Silicon Labs CP210x** driver. Without it, Device Manager lists "CP2102 USB to UART Bridge Controller" under *Other devices*. **Expect:** a new `COMx` port when the board is plugged in. (COM1 is the PC's own legacy port, never the board.)
2. In the IDE, install the board package **esp32 by Espressif Systems** (3.3.x).

## USB flash
1. Copy `shared/secrets.example.h` into the sketch folder as `secrets.h` and fill it in (gitignored).
2. **Tools → Board:** *ESP32 Dev Module*. **Tools → Port:** the board's `COMx`.
3. Click **Upload**.
4. If it fails with `Wrong boot mode detected (0x13)`: click Upload again and **hold BOOT** while the IDE prints `Connecting...`; release once writing starts.
   - Why: the auto-reset circuit releases EN before GPIO0 is pulled low, so the chip boots normally. Permanent fix: a 10 µF capacitor from EN to GND (RC delay τ ≈ 10 kΩ × 10 µF = 0.1 s).
5. Open the Serial Monitor at **115200**. **Expect:** the sketch's `[BOOT]` line.

## OTA flash (nodes that already run an OTA-enabled sketch)
1. Board on any USB charger, joined to Wi-Fi.
2. **Tools → Port → Network ports:** `<device-id> at <ip>` (e.g. `esp32-01 at 192.168.x.x`).
3. Upload; enter `OTA_PASS` from `secrets.h` if asked. **Expect:** the OLED shows "Firmware update..." with a progress bar, then the node reboots.

## Checks
- [ ] `apartment/<room>/<device>/status` = `online` (retained)
- [ ] `.../meta` shows the new `fw` version

## Troubleshooting
| Symptom | Likely cause | Fix |
|---|---|---|
| No COM port appears | charge-only cable or missing CP210x driver | known-good data cable; install driver |
| `0x13` wrong boot mode | auto-reset timing | hold BOOT during Connecting; or 10 µF EN→GND |
| No network port | node offline, or running a sketch without OTA | check `status` topic; re-flash over USB |
| Network port gone after flashing a tool | `tools/` sketches have no OTA | re-flash the node sketch over USB |
| OTA `Authentication Failed`, IDE keeps reusing a mistyped password | Arduino IDE 2 caches the OTA password | close and reopen the IDE (worked 2026-10-21); if not, switch Tools → Board away and back. Use the `OTA_PASS` the node was last USB-flashed with |
