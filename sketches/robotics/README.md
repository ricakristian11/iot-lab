# Robotics

Actuators, motion and sensing, building toward a two-wheel robot driven from the dashboard over MQTT (`apartment/robot/robot-01/cmd`).

| # | Sketch | What it does | Status |
|---|---|---|---|
| — | `servo-sweep` | SG90 servo from a potentiometer, then from a dashboard slider | planned |
| — | `tof-distance` | VL53L0X time-of-flight ranging, filtered readings | planned |
| — | `imu-tilt` | MPU-6050 accelerometer/gyro tilt estimate | planned |
| — | `robot-01` | 2WD chassis, TB6612FNG motor driver, failsafe stop on lost commands | planned |

**Power rule:** servos and motors get their own 5 V supply with a ground shared with the ESP32. Never power them from the ESP32's 5V pin: stall and start-up current spikes reset the board.
