# Documentation

| Type | What it is | Index |
|---|---|---|
| **SOPs** | Repeatable procedures: follow them step by step | below |
| **Case studies** | One problem, how it was diagnosed, root cause, lessons | below |
| **Build log** | Dated running diary of the whole system | [build-log.md](build-log.md) |
| **Photos** | Images referenced by the pages | [img/](img/) |

New pages start from [`templates/sop-template.md`](../templates/sop-template.md) or [`templates/case-study-template.md`](../templates/case-study-template.md).

## SOPs
| SOP | Covers |
|---|---|
| [flash-a-node](sop/flash-a-node.md) | First USB flash, BOOT-button download mode, OTA updates, recovering OTA |
| [breadboard-a-node](sop/breadboard-a-node.md) | Breadboard rules, power domains, wiring order, pre-power checks |
| [i2c-bring-up](sop/i2c-bring-up.md) | Adding an I2C module: scan, chip ID, bisecting a dead bus |
| [hub setup](../hub/README.md) | Raspberry Pi hub: Mosquitto + Node-RED |

## Case studies
| Date | Case study | Root cause |
|---|---|---|
| 2026-10-21 | [OLED and BH1750 not detected on the I2C bus](case-studies/2026-10-21-i2c-modules-not-detected.md) | jumpers in strips on a separate breadboard piece |

Hardware repairs (printers, GPUs, mains gear) live in the separate **bench-lab** repo; car work lives in **garage-lab**.
