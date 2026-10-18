# MQTT topics

Scheme: `apartment/<room>/<device>/<metric>`

- `<room>`: where the node lives (`bedroom`, `livingroom`, `basement`)
- `<device>`: node ID (`esp32-01`, `robot-01`)
- Payloads are JSON objects, one topic per group of related values
- Lowercase, hyphens in IDs, no leading `/`, no spaces

## Live topics

| Topic | Payload | Retained | QoS | Publisher |
|---|---|---|---|---|
| `apartment/bedroom/esp32-01/status` | `online` / `offline` (Last Will) | yes | 1 | esp32-01 |
| `apartment/bedroom/esp32-01/climate` | `{"t":22.41,"h":45.2,"p":1009.8}` (°C, %RH, hPa) | no | 0 | esp32-01 |
| `apartment/bedroom/esp32-01/motion` | `{"motion":1}` (sent on change) | yes | 0 | esp32-01 |
| `apartment/bedroom/esp32-01/light` | `{"lux":123.3}` (only if a BH1750 is fitted) | no | 0 | esp32-01 |
| `apartment/bedroom/esp32-01/screen` | `{"mode":"auto","on":true}` | yes | 0 | esp32-01 |
| `apartment/bedroom/esp32-01/meta` | `{"fw":"0.3.0","ip":"…","rssi":-45,"uptime_s":3600,…}` | yes | 0 | esp32-01 |

## Commands (to devices)

| Topic | Payload | Effect |
|---|---|---|
| `apartment/bedroom/esp32-01/cmd/screen` | `on` / `off` / `auto` | OLED always on, always off, or wake on motion |

Commands use QoS 1 and are not retained.

## Node-RED subscriptions

- `apartment/+/+/climate`: every room's climate (Climate page)
- `apartment/+/+/motion`: Motion page
- `apartment/+/+/status`: System page
