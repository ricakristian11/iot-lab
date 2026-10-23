# Node-RED flows

Dashboard 2.0 (`@flowfuse/node-red-dashboard`) with three pages:

| Page | Path | Source topic | Widgets |
|---|---|---|---|
| Climate | `/dashboard/climate` | `apartment/+/+/climate` | temperature + humidity gauges, pressure text, 24 h chart |
| Motion | `/dashboard/motion` | `apartment/+/+/motion` | status text, last-motion time, motion log table + Clear log button |
| System | `/dashboard/system` | `apartment/+/+/status` | online/offline per node |

## Motion log
A table of motion events (Start, End, Duration), newest first, kept across Node-RED restarts.

```
[mqtt in: .../motion] ─► [function: motion events] ─► [ui-table: Action = Replace]
[inject: once on start, topic "load"] ─┘
[ui-button: Clear log, topic "clear" (string)] ─┘
```
- Function source: [`motion-events.js`](motion-events.js).
- Motion within **60 s** of the last event's end extends that event (Start stays, End/Duration grow); a new row starts after >60 s without motion. Change `MERGE_GAP_MS` for finer rows.
- Keeps the newest 300 events. Ignores the retained motion value the broker replays when Node-RED reconnects (`msg.retain`), so restarts don't create fake events.
- **ui-table Action must be Replace:** the function sends the full list every time; Append stacks copies.

### Context storage (one-time, on the Pi)
The log is saved with Node-RED's file-backed context store. In `~/.node-red/settings.js`, inside `module.exports = { ... }`:
```js
contextStorage: {
    default: { module: "memory" },          // fast, lost on restart
    file:    { module: "localfilesystem" }  // saved under ~/.node-red/context/
},
```
Then `sudo systemctl restart nodered`; the log shows `Context store: 'file' [module=localfilesystem]`.

## Saving flows to this repo
1. In the editor: **≡ menu → Export → All flows → Download**.
2. Save the file here as `flows.json` and commit it.

`flows.json` holds no passwords: the MQTT broker credentials live in `flows_cred.json` on the Pi (encrypted, gitignored). After importing on a new hub, re-enter the broker username/password in the `hub` broker config node.
