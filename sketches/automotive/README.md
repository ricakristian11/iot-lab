# Automotive accessories & improvements

Projects for a 2007 Toyota Prius (Gen 2): data logging and cabin accessories.

| # | Sketch | What it does | Status |
|---|---|---|---|
| — | `cabin-climate-logger` | BME280 + deep sleep, battery powered; logs parked-cabin heat, uploads to the hub over home Wi-Fi | planned |
| — | `obd-ble-logger` | ESP32 ↔ ELM327 over Bluetooth; reads standard and hybrid-battery PIDs, uploads when parked | planned |
| — | `can-listen-only` | ESP32 TWAI + SN65HVD230 transceiver in **listen-only** mode | planned (advanced) |

## Safety rules
- **Read-only.** Nothing in this folder transmits on the vehicle CAN bus or writes to any ECU.
- **Never touch the high-voltage system** (orange cables, inverter, hybrid battery).
- **12 V auxiliary battery is small:** anything plugged in must sleep or run from switched power, or the car may not start. Measure sleep current before leaving a device installed.
- No LiPo batteries left in a hot parked car.
