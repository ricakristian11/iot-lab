# 03-bedroom-node

- **Goal:** permanent bedroom sensor node: climate, motion, light, and a status screen that wakes on motion. Reports to the Pi hub over MQTT and updates over Wi-Fi.
- **Board:** ELEGOO ESP32 (ESP-WROOM-32, 30-pin, CP2102 USB-UART), arduino-esp32 core 3.3.x
- **Status:** v0.3.2 running on hardware (breadboard, USB wall charger): OLED motion-wake, forced-mode BME280, BH1750 light, `cmd/screen`, reset reason in `meta`. `cmd/screen`, Last Will and OTA verified; 26.7 h soak passed 2026-10-23 (0 offline, no reboots, heap flat). Next: perfboard.

## Libraries (versions)
| Library | Version | Notes |
|---|---|---|
| esp32 by Espressif (board core) | 3.3.12 | don't use 4.0 pre-releases |
| ArduinoMqttClient (Arduino) | 0.1.8 | official Arduino library |
| ArduinoJson (Benoît Blanchon) | 7.4.3 | |
| Adafruit BME280 Library | 2.3.0 | + Adafruit Unified Sensor 1.1.15, Adafruit BusIO 1.17.4 |
| Adafruit SSD1306 | 2.5.17 | + Adafruit GFX Library 1.12.6 |
| ArduinoOTA | bundled with core | |
| BH1750 | none | driven with raw I2C commands from the datasheet |

## Wiring

Unplug USB before rewiring. All sensors share one I2C bus (SDA/SCL).

| Part pin | ESP32 pin | Voltage | Wire | Notes |
|---|---|---|---|---|
| BME280 VCC | 3V3 | 3.3 V | orange | 3.3 V-only module |
| BME280 GND | GND | 0 V | black | |
| BME280 SCL | GPIO22 (D22) | 3.3 V logic | gray | I2C clock |
| BME280 SDA | GPIO21 (D21) | 3.3 V logic | green | I2C data |
| BME280 CSB | 3V3 | 3.3 V | red | high = I2C mode |
| BME280 SDO | GND | 0 V | blue | low = address 0x76 |
| OLED VCC | 3V3 | 3.3 V | red | check your module's printed pin order: some swap VCC/GND |
| OLED GND | GND | 0 V | black | |
| OLED SCL | GPIO22 | 3.3 V logic | | shares the I2C bus, address 0x3C |
| OLED SDA | GPIO21 | 3.3 V logic | | |
| BH1750 VCC | 3V3 | 3.3 V | | optional; auto-detected at 0x23 |
| BH1750 GND / ADDR | GND | 0 V | | ADDR low = 0x23 |
| BH1750 SCL / SDA | GPIO22 / GPIO21 | 3.3 V logic | | |
| PIR VCC | VIN (5V) | 5 V | black | HC-SR501 needs 4.5–20 V |
| PIR OUT | GPIO33 (D33) | 3.3 V logic | white | the PIR's regulator makes OUT 3.3 V, so it's safe for the ESP32 |
| PIR GND | GND | 0 V | gray | |

Expected I2C scan: `0x23` (BH1750), `0x3C` (OLED), `0x76` (BME280).

Perfboard build (main board + crimped sensor leads): [perfboard.md](perfboard.md).

## MQTT topics
Base: `apartment/bedroom/esp32-01`. Full table in [`shared/topics.md`](../../../shared/topics.md).
- **pub:** `status` (LWT, retained), `climate`, `motion` (retained), `light`, `screen` (retained), `meta` (retained)
- **sub:** `cmd/#`, currently `cmd/screen` = `on` | `off` | `auto`

```bash
# watch everything from this node (on the Pi)
mosquitto_sub -h localhost -u hubuser -P '<password>' -t 'apartment/bedroom/esp32-01/#' -v
# force the screen on
mosquitto_pub -h localhost -u hubuser -P '<password>' -t apartment/bedroom/esp32-01/cmd/screen -q 1 -m on
```

## How it works
- **Non-blocking main loop:** every task is a `millis()` timer or state check, so Wi-Fi, MQTT, OTA, the PIR and the screen are all serviced each pass.
- **PIR state machine:** `WARMUP` (first 60 s: output ignored), then `IDLE` ↔ `MOTION`. Motion is published only on transitions, and as a retained message so a restarted dashboard shows the current state.
- **Screen policy (`AUTO`):** on during warm-up and for 60 s after the last motion, otherwise the panel is switched off (`0xAE`) to prevent OLED burn-in.
- **BME280 in forced mode:** one measurement every 30 s, after which the sensor sleeps. Bosch's recommended "weather monitoring" setting; it reduces self-heating compared with the library's default continuous mode.
- **Reconnects** back off 5 → 10 → 20 → 30 s. The MQTT connect timeout is 3 s (the library default is 30 s, which would freeze the loop).
- **Last Will:** if the node disappears, the broker publishes `offline` on `status` itself.

## How to run
1. Copy `shared/secrets.example.h` into this folder as `secrets.h` and fill it in (it's gitignored).
2. Install the libraries above (Library Manager).
3. First upload over USB (hold **BOOT** during "Connecting…" if needed). After that, use the network port `esp32-01 at <ip>` under *Tools → Port* (OTA).
4. Serial Monitor at 115200 shows tagged logs: `[WIFI] [MQTT] [SENSOR] [OLED] [OTA]`.

## Lessons / gotchas
- The board wouldn't show a COM port until the Silicon Labs **CP210x** driver was installed. COM1 is the PC's own legacy port.
- `Wrong boot mode (0x13)`: hold BOOT during upload, or fit 10 µF from EN to GND.
- Header pins must run along numbered breadboard rows. Across a–e they short together.
- `D33` and `D35` look alike on the silkscreen. The PIR stayed silent until the wire was moved.
- Verify a "BME280" by chip ID: `0x60` = BME280, `0x58` = BMP280 (no humidity).
- **v0.3.1:** the BH1750 is re-armed (power on + continuous mode) after every read. A supply glitch from wiggling a wire restarts it in power-down mode, where it still answers on I2C but reads 0 lux forever.
- **v0.3.2:** `meta` reports `reset` (why the node last started), so an unexpected reboot seen on the hub can be told apart: power loss, brownout or crash. On the ESP32 the EN button reports `poweron`.
