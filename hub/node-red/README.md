# Node-RED flows

Dashboard 2.0 (`@flowfuse/node-red-dashboard`) with three pages:

| Page | Path | Source topic | Widgets |
|---|---|---|---|
| Climate | `/dashboard/climate` | `apartment/+/+/climate` | temperature + humidity gauges, pressure text, 24 h chart |
| Motion | `/dashboard/motion` | `apartment/+/+/motion` | status text, last-motion time |
| System | `/dashboard/system` | `apartment/+/+/status` | online/offline per node |

## Saving flows to this repo
1. In the editor: **≡ menu → Export → All flows → Download**.
2. Save the file here as `flows.json` and commit it.

`flows.json` holds no passwords: the MQTT broker credentials live in `flows_cred.json` on the Pi (encrypted, gitignored). After importing on a new hub, re-enter the broker username/password in the `hub` broker config node.
