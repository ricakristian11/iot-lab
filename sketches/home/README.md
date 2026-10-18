# Home improvement

Apartment sensing and automation nodes that report to the Pi hub over MQTT (`apartment/<room>/<device>/<metric>`).

| # | Sketch | What it does | Status |
|---|---|---|---|
| 01 | [`01-blink`](01-blink) | toolchain bring-up, non-blocking timing | ✅ |
| 02 | [`02-bme280-bringup`](02-bme280-bringup) | I2C scan, chip-ID check, BME280 readings | ✅ |
| 03 | [`03-bedroom-node`](03-bedroom-node) | climate + motion + light node, motion-wake OLED, MQTT, OTA | ✅ v0.2 · v0.3 in test |

Planned: door/window reed switch, plant and grow-tent node (soil moisture, DS18B20, VPD), desk presence + status display.
