# CLAUDE.md — iot-lab repo

## What this repo is
- Collection of independent ESP32/Arduino sketches, Pi scripts and Node-RED flows. Not one app.
- Owner is a software engineer returning to embedded: explain hardware assumptions in comments + README.

## Layout
- `sketches/<category>/<kebab-name>/<kebab-name>.ino` — Arduino IDE requires folder name == .ino name
- Categories: `home/` (apartment sensing/automation), `robotics/` (actuators, motion, robot), `automotive/` (car data logging and accessories; read-only, never transmit on CAN)
- Each category has a README index; add new sketches to it
- `sketches/<category>/<name>/README.md` — from `templates/sketch-README.md`
- `sketches/<category>/<name>/config.h` — non-secret constants (room, device ID, pins, timing)
- `sketches/<category>/<name>/secrets.h` — gitignored; copy from `shared/secrets.example.h`
- `shared/` — `secrets.example.h`, `topics.md`
- `pi/` — Python scripts; `hub/node-red/flows.json`, `hub/mosquitto/*.conf` (no passwords)
- `cad/` — enclosures (OpenSCAD/STEP/3MF); `docs/` — longer notes, build log, photos

## Build / flash
- Toolchain: Arduino IDE 2.x or `arduino-cli`, core `esp32:esp32@3.3.x`, FQBN `esp32:esp32:esp32`
- Compile: `arduino-cli compile -b esp32:esp32:esp32 sketches/<category>/<name>`
- Upload: `arduino-cli upload -b esp32:esp32:esp32 -p <port> sketches/<category>/<name>`
- Deployed nodes update over Wi-Fi (ArduinoOTA); the network port is `<device-id> at <ip>`
- PlatformIO projects (if any) use the pioarduino platform URL in `platformio.ini`

## Rules
- Never use `delay()` in `loop()`; use `millis()` timers and `enum class` state machines
- Pins are `constexpr` constants (in `config.h` or at file top). Never use GPIO 6–11. Never drive outputs on 34–39. Analog only on ADC1 (32–39).
- MQTT topics: `apartment/<room>/<device>/{status|<metric>|cmd/<target>|meta}`; metric payloads are JSON objects; status retained + LWT
- MQTT lib: ArduinoMqttClient 0.1.8 (official Arduino; in use on esp32-01). PubSubClient3 was considered; ask before switching.
- BME280: Adafruit BME280 2.3.x in forced mode. OLED: Adafruit SSD1306 2.5.x. JSON: ArduinoJson 7.x.
- Never commit secrets. If you create `secrets.h`, confirm it's gitignored first.
- Every new sketch gets a README with a wiring table (pin, GPIO, voltage) and library versions.
- Ask before adding a new library; state its version and maintenance status.
- Don't change files outside the sketch you're working on unless asked.

## Before finishing a task
- Compile the touched sketch with arduino-cli; fix all warnings you introduced
- Update the sketch README and `shared/topics.md` if topics changed
- Summarize: what changed, wiring changes, what I need to test on hardware
