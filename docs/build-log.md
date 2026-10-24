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

## 2026-10-21 — I2C bring-up of the OLED + BH1750
- Neither new module was detected: both were wired into strips on a separate breadboard piece. Moving their jumpers into the BME280's strips fixed it; scanner now shows 0x23, 0x3C, 0x76. Full write-up: [case study](case-studies/2026-10-21-i2c-modules-not-detected.md).
- New reusable tool: [`tools/i2c-scanner`](../tools/i2c-scanner) (continuous scan, names devices, flags missing ones).
- Repo reorganised: car code moved to **garage-lab**, repair work to **bench-lab**; SOPs and case studies added under `docs/`.

## 2026-10-21 — v0.3 on hardware
- `meta`: fw 0.3.0, bme280/oled/bh1750 all true, RSSI −44 to −55 dBm, heap flat at ~207 KB.
- BH1750: ~10.8 lux facing down, 2816.7 lux under a phone flashlight, 0 covered.
- BME280: 27.7 °C vs 27 °C on the multimeter's thermocouple in the same room (heating on). Humidity jumped to 78.6 % when breathed on, then recovered.
- OLED wakes on motion and sleeps after 60 s.
- Found stale retained topics from an older `livingroom` config; cleared with an empty retained publish (`-r -n`).
- Hub unreachable at first: the terminal was WSL, which can't resolve `hub.local` (mDNS). PowerShell or the Pi's IP works.

## 2026-10-21 — v0.3.1: BH1750 stuck at 0 lux
**Symptom:** light dropped from ~28,000 lux (flashlight) to exactly 0 and stayed there; `meta` still showed `bh1750: true`.
**Diagnosis:** the read still returned 2 bytes, so the sensor was answering on I2C. Exactly 0 with room lights on points to the module having restarted in power-down mode after a supply glitch while the connections were being checked. The firmware only sent power-on + continuous mode once at boot.
**Fix:** v0.3.1 re-arms the sensor after every read (power on + continuous mode, no reset).

## 2026-10-22 — Overnight run: one reboot → v0.3.2
- 17.6 h run (`uptime_s` 63,360), heap flat at ~205–209 KB, RSSI −38 to −57 dBm.
- At 16:57 `uptime_s` restarted from 0: the node **rebooted** (not a Wi-Fi drop). Because it reconnected with the same client ID before the broker had noticed the old connection was dead, the broker published the old connection's `offline` and the new `online` in the same second.
- Cause unknown: the firmware didn't record why it started. v0.3.2 adds `reset` to `meta` (poweron / brownout / panic / watchdog …) to diagnose the next one.
- **Most likely cause: the PC waking from sleep.** Windows logged "returned from low power state" (Power-Troubleshooter, event 1) at 16:57:29; the node booted ~4 s later (≈16:57:33). No restart/shutdown events (1074/6005/6006) in that window. Whether a USB port loses power on resume depends on the motherboard/BIOS. Fix: the node now runs from a USB wall charger.

## 2026-10-23 — Motion event log on the dashboard
- New Motion-page table: Start / End / Duration per motion event, newest first, saved in Node-RED's file-backed context so it survives restarts ([hub/node-red/README.md](../hub/node-red/README.md#motion-log)).
- Bursts within 60 s merge into one event, so the PIR's on/off toggling doesn't flood the table.
- **Gotchas found while building:** ui-table defaulted to *Append*, which stacked a copy of the whole list on every update (switched to *Replace*); the Clear button's Topic was set to "copy msg.topic" instead of the string `clear`; the retained motion value replayed on reconnect is filtered with `msg.retain`.

## 2026-10-23 — v0.3.2 soak test passed
- 26.5 h logged (2026-10-22 18:55 → 2026-10-23 21:27), node on a USB wall charger.
- 0 `status offline`; every `meta` reported `"reset":"poweron"` (the one boot when it was moved to the charger), so no reboots.
- `uptime_s` 480 → 96,000; heap 206,956 → 208,592 bytes (no leak); RSSI settled around −26 dBm next to the extender.
- Cleared for the move to perfboard.
