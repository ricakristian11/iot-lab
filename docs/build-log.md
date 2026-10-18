# Build log

Problems hit while building the system, how each was diagnosed, and the fix. Newest last.

## 2026-10-14 — Toolchain bring-up
**Symptom:** upload failed with `Failed to connect to ESP32: No serial data received` on COM1.
**Diagnosis:** COM1 is the PC's legacy serial port, not the board. Device Manager listed "CP2102 USB to UART Bridge Controller" under *Other devices* with a warning: the USB-UART driver was missing.
**Fix:** installed the Silicon Labs CP210x Universal Windows Driver; the board appeared as COM3.

**Symptom:** `Wrong boot mode detected (0x13)! The chip needs to be in download mode.`
**Diagnosis:** the board's auto-reset circuit (DTR/RTS → EN/GPIO0) releases EN too early, so the chip samples GPIO0 high and boots normally instead of into the bootloader.
**Fix:** hold BOOT during "Connecting…". Permanent option: 10 µF from EN to GND adds an RC delay (τ ≈ 10 kΩ × 10 µF = 0.1 s). OTA now makes USB uploads rare.

## 2026-10-15 — First I2C sensor
- Soldered the BME280 header using the breadboard as a jig (one pin first, check alignment, then the rest).
- **Mistake caught before power-up:** the header was first placed across breadboard rows a–e, which would have shorted all six pins together (VCC to GND). Header pins must run along the numbered rows.
- I2C scan found 0x76; register 0xD0 returned **0x60**, confirming a genuine BME280 rather than a BMP280 (0x58, no humidity).
- Readings ran ~1–2 °C warm: self-heating from continuous measurement and the nearby ESP32. v0.3 switches the BME280 to forced mode.

## 2026-10-15 — PIR silent
**Symptom:** no motion events after warm-up.
**Diagnosis:** the OUT wire was in the D35 row; the sketch reads GPIO33. The silkscreen labels are tiny and adjacent.
**Fix:** moved the wire to the D33 row. Lesson: read the label next to each pin, never count positions.

## 2026-10-16 — Hub
- Pi 3B+ flashed headless (Raspberry Pi OS Lite 64-bit, Trixie). Mosquitto 2.x with a `listener` + password file; anonymous publish verified as refused.
- Node-RED installed with the official script; heap capped at 256 MB with a systemd override.
- **Wi-Fi quirk:** the router and extender broadcast one SSID (Verizon SON), so the ESP32 saw two access points. Set `WIFI_ALL_CHANNEL_SCAN` + `WIFI_CONNECT_AP_BY_SIGNAL` so it joins the strongest.

**Symptom:** Node-RED's MQTT node stuck on "connecting" while `mosquitto_sub` worked fine.
**Diagnosis:** the broker config node held an old password; Node-RED stores its own encrypted copy, separate from the broker's password file.
**Fix:** re-entered the credentials in the config node.

**Symptom:** dashboard widgets flagged "invalid property: group".
**Diagnosis:** the ui-page had no theme selected, which invalidated the page → group → every widget.
**Fix:** added a ui-theme to the page.

**Symptom:** only temperature reached the dashboard.
**Diagnosis:** the function node returned three messages but was configured with one output.
**Fix:** set *Outputs: 3* and wired each output separately.

## 2026-10-16 — OTA
Added ArduinoOTA, started only after Wi-Fi connects. Firmware now updates from the IDE's network port, and the node runs off a USB charger.

## 2026-10-17 — v0.3 firmware
- OLED status screen that turns on with motion and off after 60 s (prevents burn-in).
- PIR handled as a `WARMUP → IDLE ↔ MOTION` state machine.
- BH1750 driven with raw I2C commands from the datasheet (power on, reset, continuous high-res; lux = count / 1.2), auto-detected at boot.
- `cmd/screen` topic (`on` / `off` / `auto`): first command path from the hub to a node.
- MQTT connect timeout cut from the library's 30 s default to 3 s, and reconnects back off to 30 s, so a missing broker no longer freezes the loop.
