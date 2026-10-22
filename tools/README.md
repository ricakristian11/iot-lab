# tools/

Diagnostic and bring-up sketches. Not deployed firmware: flash one, use it, then re-flash the node's real sketch.

| Tool | What it does | When to use it |
|---|---|---|
| [`i2c-scanner`](i2c-scanner) | Rescans the I2C bus every 5 s, names known devices, flags expected devices that are missing | After any wiring change on an I2C node; first step of [I2C bring-up](../docs/sop/i2c-bring-up.md) |

**Heads-up:** tools have no Wi-Fi or OTA. Flashing one over USB removes OTA from that board until the node sketch is re-flashed over USB.

Folder rule (same as sketches): `tools/<kebab-name>/<kebab-name>.ino` plus a README.
