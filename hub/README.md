# Hub (Raspberry Pi 3 B+)

| Item | Value |
|---|---|
| OS | Raspberry Pi OS Lite 64-bit (Debian 13 "Trixie"), headless, flashed with Raspberry Pi Imager 2.x |
| Hostname | `hub` (`hub.local` from PCs; nodes use the IP) |
| Broker | Mosquitto 2.x on :1883, password auth, no anonymous access |
| Flows | Node-RED 5 on :1880 (editor behind adminAuth), Dashboard 2.0 at `/dashboard` |

## Setup summary
1. Flash with Raspberry Pi Imager: hostname, user, Wi-Fi (country US), SSH enabled.
2. `sudo apt update && sudo apt full-upgrade -y`
3. Mosquitto: `sudo apt install -y mosquitto mosquitto-clients`, create users with `mosquitto_passwd`, copy [`mosquitto/apartment.conf`](mosquitto/apartment.conf) to `/etc/mosquitto/conf.d/`, then `sudo systemctl enable --now mosquitto`.
4. Node-RED: official installer
   `bash <(curl -sL https://github.com/node-red/linux-installers/releases/latest/download/install-update-nodered-deb)`,
   answer yes to user security, apply [`node-red/nodered-override.conf`](node-red/nodered-override.conf) via `sudo systemctl edit nodered.service`, then `sudo systemctl enable nodered`.
5. Install `@flowfuse/node-red-dashboard` from **Manage palette**, then import `node-red/flows.json`.

## Checks
```bash
systemctl status mosquitto nodered --no-pager
vcgencmd get_throttled     # 0x0 = never under-volted
free -h
```
