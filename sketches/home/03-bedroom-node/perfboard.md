# esp32-01 perfboard (main board + crimped sensor leads)

| | |
|---|---|
| **Board** | 5 × 7 cm double-sided plated perfboard, 18 × 24 holes (columns **1–18**, rows **A–X**, as printed on the board) |
| **Status** | planned 2026-10-24 · not built |
| **Firmware** | unchanged (v0.3.2): I2C SDA = GPIO21, SCL = GPIO22, PIR = GPIO33 |

## Concept
- The perfboard carries only the **ESP32 (in female headers)**, four **bus lines** and **keyed JST-XH connectors**.
- The OLED, BH1750, BME280 and PIR connect by **crimped leads**, so each can face outward or sit away from the ESP32's heat in the enclosure.
- All three I2C ports are identical. Pin 1 = **GND**, 2 = **3V3**, 3 = **SDA**, 4 = **SCL** (Qwiic order), so any sensor fits any port.

## Safety
- **Unplug USB** before soldering, rewiring or plugging leads in.
- Iron at ~330–350 °C for leaded solder. Ventilate: flux fumes are harmful. Wear eye protection when clipping leads.
- Never connect 5 V to a GPIO or to the 3V3 bus. 5 V exists only on the PIR port.

## Orientation
1. Hold the board **label side up**, with column numbers **1 → 18 running left → right**. Row **A** is then at the **bottom** and row **X** at the **top** (checked against a photo of the board, 2026-10-24).
2. The ESP32 sits **chip-side up**:
   - its **antenna overhangs the left edge**, past column 1, so there's no copper under the antenna;
   - its **USB-C end points right**, toward column 18.
3. With that orientation, the pin line labelled **D23 … 3V3** lands in **row L** and **EN … VIN** in **row B**, both in columns 1–15. The lines are 10 holes apart (25.4 mm).
4. Dry-fit before soldering. Read the ESP32's silkscreen:
   - **L1 = D23, L15 = 3V3**
   - **B1 = EN, B15 = VIN**

## Hole map
```
col:  1   2   3   4   5   6   7   8   9   10  11  12  13  14  15  16  17  18
 X    ·   ·   ·   ·   ·   ·   ·   ·   ·   ·   ·   ·   ·   ·   ·   ·   ·   ·        (top edge)
 W   [P1 GND 3V3 SDA SCL] · [P2 GND 3V3 SDA SCL] · [P3 GND 3V3 SDA SCL] · [PIR GND OUT 5V]
 V    j   j   j   j   ·   j   j   j   j   ·   j   j   j   j   ·   j   ·   ·        jumper start holes (bridged to row W on the back)
 R-U  ·   (empty)
 Q    ═══════════════════════════════ GND bus ═════════════════════════════  C     bare wire, back side, cols 1-18
 P    ═══════════════════════════════ 3V3 bus ═════════════════════════════  C     100 nF between P18 and Q18
 O    ═══════════════════════════════ SDA bus ═════════════════════════════
 N    ═══════════════════════════════ SCL bus ═════════════════════════════
 M    ·   b   ·   ·   t   ·   ·   ·   ·   ·   ·   ·   ·   t   t   ·   ·   ·        tie-in start holes (b = bridge, t = jumper)
 L   D23 D22 TX0 RX0 D21 D19 D18 D5  TX2 RX2 D4  D2  D15 GND 3V3  ·   ·   ·        ESP32 female header (cols 1-15)
 D-K  ·   (empty, under the ESP32)
 C    +   ·   −   ·   ·   ·   ·   ·   ·   ·   ·   ·   ·   ·   ·   ·   ·   ·        10 µF lying flat under the ESP32: + C1, − C3
 B   EN  VP  VN  D34 D35 D32 D33 D25 D26 D27 D14 D12 D13 GND VIN  ·   ·   ·        ESP32 female header (cols 1-15)
 A    ·   (bottom edge, keep clear)
```
- The JST-XH headers in row W start at columns 1, 6, 11 and 16. Their housings are about 12.4 mm wide, so neighbours almost touch.
- **Dry-fit all four XH headers together before soldering any of them.**

## Connection list
**Rules:**
- Bus runs are **bare wire on the back**.
- Every other wire that crosses a bus or runs a long way is **insulated**.
- "Bridge" means a solder bridge between adjacent pads on the back.

| # | From | To | How | Wire | Voltage | Why |
|---|---|---|---|---|---|---|
| 1 | Q1 | Q18 | bus run | bare | 0 V | GND bus |
| 2 | P1 | P18 | bus run | bare | 3.3 V | 3V3 bus |
| 3 | O1 | O18 | bus run | bare | 3.3 V logic | SDA bus |
| 4 | N1 | N18 | bus run | bare | 3.3 V logic | SCL bus |
| 5 | L2 (D22) | N2 | bridge L2–M2–N2 | solder | 3.3 V logic | SCL from the ESP32 |
| 6 | L5 (D21) | O5 | bridge L5–M5, jumper M5→O5 (front) | blue | 3.3 V logic | SDA from the ESP32 |
| 7 | L14 (GND) | Q15 | bridge L14–M14, jumper M14→Q15 (front, diagonal) | black solid | 0 V | ground to the bus |
| 8 | L15 (3V3) | P10 | bridge L15–M15, jumper M15→P10 (front, diagonal) | red solid | 3.3 V | regulator output to the bus |
| 9 | each I2C port pin + W16 (row W) | row V below it | bridge Wx–Vx | solder | — | jumper start points |
| 10 | V1 / V6 / V11 (pin 1) | Q1 / Q6 / Q11 | jumper (front) | black solid | 0 V | port GND |
| 11 | V2 / V7 / V12 (pin 2) | P2 / P7 / P12 | jumper (front) | red solid | 3.3 V | port 3V3 |
| 12 | V3 / V8 / V13 (pin 3) | O3 / O8 / O13 | jumper (front) | blue | 3.3 V logic | port SDA |
| 13 | V4 / V9 / V14 (pin 4) | N4 / N9 / N14 | jumper (front) | yellow | 3.3 V logic | port SCL |
| 14 | P18 | Q18 | 100 nF ceramic ("104") | — | 3.3 V | decoupling near the ports |
| 15 | B1 (EN) | C1 | bridge; 10 µF **+** in C1 | solder | 3.3 V | auto-reset fix (no BOOT button) |
| 16 | C3 (10 µF **−**) | B14 (GND) | wire on the back, soldered to the B14 header joint | black, insulated | 0 V | capacitor to ground |
| 17 | V16 (PIR GND) | Q16 | jumper (front) | black solid | 0 V | PIR ground from the bus |
| 18 | B7 (D33) | W17 (PIR OUT) | wire on the back, header joint to XH pin | green, insulated | 3.3 V logic | PIR output to GPIO33 |
| 19 | B15 (VIN) | W18 (PIR 5V) | wire on the back, header joint to XH pin | orange, insulated | **5 V** | PIR supply |

- The 10 µF electrolytic lies flat under the ESP32. The **stripe is −**.
- On the back, route wires #16, #18 and #19 **around** solder joints, not across them.

## Leads (crimped: JST-XH at the board, Dupont at the module)
**Wire:** stranded, 24–26 AWG, 10–15 cm long.

**JST-XH end, the same on every I2C lead:**

| XH pin | 1 | 2 | 3 | 4 |
|---|---|---|---|---|
| signal | GND | 3V3 | SDA | SCL |
| colour | black | red | blue | yellow |

**Dupont end (module order):**

| Lead | Module pins, Dupont positions 1→4 | Colours 1→4 | Notes |
|---|---|---|---|
| OLED → P1 | GND, VCC, SCL, SDA | black, red, yellow, blue | SDA/SCL cross inside the lead |
| BH1750 → P2 | VCC, GND, SCL, SDA | red, black, yellow, blue | ADDR left unconnected (reads 0x23) |
| BME280 → P3 | VCC, GND, SCL, SDA | red, black, yellow, blue | on the module: link **CSB→VCC** (I2C mode) and **SDO→GND** (0x76) |

**PIR lead (XH-3 at W16–W18 → Dupont 3P on the PIR):** the wires run straight through, no crossing.

| Board pin | Signal | Colour | PIR pin |
|---|---|---|---|
| W16 | GND | black | GND |
| W17 | OUT (GPIO33) | green | OUT (middle) |
| W18 | **5 V** (VIN) | orange | VCC |

Dupont housings aren't keyed: mark pin 1 on every housing with a paint-pen dot. **Continuity-test every lead end to end before first use.**

## Soldering order
0. **Cut the female headers** from one 40-pin strip.
   - Female strips lose one pin at every cut, so 15 + 1 + 15 = 31 of the 40 pins are used.
   - Count 15 positions. Pull the pin out of position **16** with pliers, then cut through the middle of that empty slot with flush cutters.
   - Repeat for the second strip, then smooth both cut ends with a file or sandpaper.
   - **Eye protection:** cut pieces fly.
1. **Female headers**, if available: plug them onto the ESP32 and drop them into rows L and B (cols 1–15), antenna over the left edge.
   - Solder the two end pins of each strip, check the ESP32 sits flat, then solder the rest.
   - Without headers yet, start at step 2. Rows L and B stay reserved.
2. **Bus runs** N, O, P, Q (back, bare). Solder at both ends and every ~3 holes.
3. **XH headers**: P1–P3 and PIR in row W (after dry-fitting all four together). All the same way round; mark pin 1.
4. **Port bridges** (row W→V) and **port jumpers** (#10–13, #17).
5. **ESP32 tie-ins** (#5–8).
6. **100 nF** (#14), **10 µF** + its ground wire (#15–16), **PIR** wires (#18–19).
7. Inspect every joint under the microscope: shiny cones, no bridges between neighbours.

## Continuity checks (no power, no ESP32, no leads)
- [ ] GND bus ↔ 3V3 bus: **no** beep
- [ ] SDA ↔ SCL, and each of them ↔ GND / 3V3: **no** beep
- [ ] W18 (5 V) ↔ any bus: **no** beep
- [ ] Each I2C port: pin 1 → Q, pin 2 → P, pin 3 → O, pin 4 → N: beep. PIR W16 → Q: beep
- [ ] L2 → N, L5 → O, L14 → Q, L15 → P: beep
- [ ] B7 → W17 and B15 → W18: beep
- [ ] B14 ↔ Q: no beep yet. The two ESP32 GND pins (L14, B14) are only joined through the ESP32 itself; with it plugged in, they beep
- [ ] B1 ↔ B14: no steady beep (a short chirp is the 10 µF charging)
- [ ] Neighbouring pins in rows L and B: no beep unless listed above

## First power-up
1. ESP32 plugged in, **no leads**, USB from the wall charger.
2. Measure (DC volts, black probe on a port's pin 1):
   - each port's pin 2 ≈ **3.3 V**;
   - W18 ≈ **5 V** (slightly less: it comes from USB through a diode on the ESP32 board);
   - W17 ≈ 0 V (no PIR plugged in).
3. Plug in the leads **one at a time**, running `tools/i2c-scanner` after each. Expect 0x3C, then 0x23, then 0x76.
4. Re-flash `03-bedroom-node` over USB. Check `meta`: `bme280`, `oled` and `bh1750` all `true`.

## Parts
| Part | Qty | Source |
|---|---|---|
| Perfboard 5 × 7 cm (18 × 24) | 1 | owned |
| Female header strip, 2.54 mm, 15 pins | 2 | owned: cut from one 40-pin strip (see Soldering order step 0) |
| JST-XH 4-pin straight PCB header + housing + terminals | 3 | crimp kit |
| JST-XH 3-pin straight PCB header + housing + terminals | 1 | crimp kit |
| Dupont 4P housing + female terminals | 3 | crimp kit |
| Dupont 3P housing + female terminals | 1 | crimp kit |
| 10 µF electrolytic | 1 | kit |
| 100 nF ceramic ("104") | 1 | kit |
| Solid red/black wire (board jumpers), stranded colours (leads), bare wire or component offcuts (buses) | — | owned |
