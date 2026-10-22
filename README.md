# iot-lab

A home IoT system built from scratch: ESP32 sensor nodes publish over MQTT to a Raspberry Pi hub, which runs Mosquitto and Node-RED and serves a live multi-page dashboard. This repo holds the firmware, hub configuration, SOPs and case studies as the system grows toward robotics.

**Sister repos:** **garage-lab** (car repair SOPs, OBD-II and read-only CAN tools) · **bench-lab** (repair bench: SOPs, scripts and case studies for hardware fixes).

> Photos of the hardware and dashboard go in [`docs/img/`](docs/img/).

## Architecture

```mermaid
flowchart LR
  subgraph Nodes["ESP32 nodes"]
    N1["esp32-01 (bedroom)<br/>BME280 · PIR · OLED · BH1750"]
    N2["future nodes<br/>door · plants · robot"]
  end
  subgraph Hub["Raspberry Pi 3 B+ hub"]
    M[("Mosquitto<br/>MQTT broker :1883")]
    R["Node-RED 5 +<br/>Dashboard 2.0 :1880"]
  end
  N1 -- "apartment/bedroom/esp32-01/climate · motion · status" --> M
  N2 -.-> M
  M <--> R
  R -- "cmd/screen" --> M --> N1
  B["Browser / wall-mounted phone"] --> R
```

- **Nodes** do the real-time work: read sensors, drive outputs, react locally. Each publishes JSON to `apartment/<room>/<device>/<metric>`.
- **Hub** routes messages (Mosquitto, authenticated, Last Will for offline detection) and runs the logic and dashboards (Node-RED, pages for Climate, Motion and System).
- **Firmware updates** go over Wi-Fi (OTA), so mounted nodes never need a cable.

## Sketches

| Category | Folder | Focus |
|---|---|---|
| 🏠 Home improvement | [`sketches/home/`](sketches/home) | apartment sensing and automation on the MQTT hub |
| 🤖 Robotics | [`sketches/robotics/`](sketches/robotics) | actuators, motion, sensing, a dashboard-driven robot |
| 🧪 Tools | [`tools/`](tools) | diagnostic and bring-up sketches (I2C scanner), not deployed firmware |

| # | Sketch | What it shows | Status |
|---|---|---|---|
| home/01 | [`01-blink`](sketches/home/01-blink) | toolchain bring-up, non-blocking timing | ✅ |
| home/02 | [`02-bme280-bringup`](sketches/home/02-bme280-bringup) | I2C bus scan, chip-ID verification (BME280 vs BMP280), sensor reads | ✅ |
| home/03 | [`03-bedroom-node`](sketches/home/03-bedroom-node) | multi-sensor MQTT node: state machines, Last Will, backoff reconnects, OTA, motion-wake OLED, register-level BH1750 driver | ✅ v0.3 on hardware (soak test pending) |

Robotics sketches are planned; see [`sketches/robotics/`](sketches/robotics).

## Documentation
SOPs (flashing, breadboarding, I2C bring-up), case studies and the build log: [`docs/`](docs/README.md).

## Hardware

| Part | Role |
|---|---|
| ELEGOO ESP32 DevKit (ESP-WROOM-32, CP2102) | sensor/actuator nodes |
| Raspberry Pi 3 Model B+ (Raspberry Pi OS Lite 64-bit) | hub: Mosquitto + Node-RED |
| Bosch BME280 (I2C) | temperature, humidity, pressure |
| HC-SR501 PIR | motion |
| SSD1306 0.96" OLED (I2C) | on-node status screen |
| BH1750 (I2C) | ambient light |

## Skills demonstrated
- **Embedded C++ on ESP32:** non-blocking super-loop, `enum class` state machines, compile-time config, secrets kept out of source
- **Buses and bring-up:** I2C scanning, register reads with repeated start, verifying parts by chip ID, sharing one bus across three devices at 400 kHz
- **Networked firmware:** Wi-Fi with exponential backoff, MQTT with Last Will, retained state, command topics, OTA with on-screen progress
- **Linux / hub ops:** headless Pi setup, Mosquitto auth config, systemd overrides, Node-RED flows and dashboards
- **Hardware:** soldering, breadboard layout, power-domain rules (3.3 V logic vs 5 V supply)

## Repo layout
```
sketches/   home/, robotics/ — one folder per sketch (folder name == .ino name)
tools/      diagnostic sketches (i2c-scanner); flash, use, then re-flash the node
shared/     secrets template + MQTT topic reference
templates/  new sketch, SOP and case-study templates
hub/        Mosquitto config and Node-RED flows for the Pi
pi/         Python scripts for the hub (planned)
cad/        enclosure designs (planned)
docs/       SOPs, case studies, build log, photos
```

## Getting started
1. Install Arduino IDE 2.x and the **esp32 by Espressif** board package (3.3.x).
2. Copy `shared/secrets.example.h` into a sketch folder as `secrets.h` and fill it in.
3. Open the sketch, select **ESP32 Dev Module**, upload.
4. Hub setup: see [`hub/README.md`](hub/README.md).

## Build log
Debugging stories and lessons: [`docs/build-log.md`](docs/build-log.md).

## Roadmap
- [x] Sensor node → MQTT → dashboard
- [x] OTA updates
- [ ] Move node 01 from breadboard to soldered perfboard + enclosure
- [ ] Door/window reed switch node
- [ ] Plant / grow-tent node (soil moisture, DS18B20, VPD in Node-RED)
- [ ] Servo and distance sensing → two-wheel robot controlled over MQTT

### Later (not started)
- Port a node from Arduino C++ to C on ESP-IDF (FreeRTOS tasks, native drivers)
- Replace Node-RED Dashboard with a self-built dashboard (own backend subscribing to MQTT)
