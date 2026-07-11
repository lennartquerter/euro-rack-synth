# CEM3320 High-Pass Filter Signal Path Analysis

## Signal Chain: SIGNAL_IN → SIGNAL_OUT

### Entry Point
**SIGNAL_IN** at position (91.44, 68.58)

---

## Stage 1: Input Impedance Network
**Wire 1:** (91.44, 68.58) → (96.52, 68.58)
- Signal passes through wire to the first input stage

**R4 (100K)** - vertical resistor at (113.03, 68.58, rotated 270°)
- **Connection:** Input signal travels through R4 (resistor between the signal path)
- **Function:** Input impedance/coupling resistor; provides input buffering and determines input impedance

**C7 (300pF)** - small capacitor at (100.33, 68.58, rotated 270°)
- **Connection:** Shunt capacitor from signal path to ground
- **Function:** Input AC coupling / high-frequency bypass to ground; works with R4 to form an RC high-pass section

**Wire 2:** (106.68, 68.58) → (109.22, 68.58)
- Signal continues downstream to next stage

---

## Stage 2: First Pole Input to CEM3320
The signal feeds into **U2** (CEM3320 4-pole VCF) at position (151.13, 87.63)

**R5 (100K)** - vertical resistor at (139.7, 68.58, rotated 270°)
- **Connection:** Second series resistor in signal path before the chip input
- **Function:** Further input isolation and impedance matching to the VCF input stage

**C8 (300pF)** - small capacitor at (125.73, 68.58, rotated 270°)
- **Connection:** Shunt capacitor between signal path and ground
- **Function:** Additional AC coupling / bypass network; part of frequency-shaping network before the VCF

**Wire 3:** (116.84, 68.58) → (120.65, 68.58)
- Signal path continues toward VCF input

**Wire 4:** (129.54, 68.58) → (130.81, 68.58)
- Signal routing continues to VCF input pins

**Wire 5:** (133.35, 68.58) → (135.89, 68.58)
- Final approach to VCF input

**Wire 6:** (143.51, 68.58) → (146.05, 68.58)
- Signal enters the CEM3320 VCF

**U2 - CEM3320 (4-pole VCF) Input**
- **Chip reference:** U2 at (151.13, 87.63)
- **Input connections:** Signal path routed to input pins of the high-pass filter variant
- **Function:** Core filtering stage—the CEM3320 high-pass filter mode configuration with:
  - 4-pole high-pass resonant filter
  - Frequency control via FREQ_CTRL input (connected to R10, R11, FREQ_CTRL voltage divider)
  - Resonance control via RES_CTRL input

---

## Stage 3: Frequency Control Network
**R10 (100K)** - horizontal resistor at (83.82, 96.52, rotated 90°)
- **Connection:** From FREQ_CTRL input, in series to frequency control divider
- **Function:** Frequency control input scaling resistor

**R11 (1.8K)** - horizontal resistor at (90.17, 102.87, rotated 180°)
- **Connection:** Connected between frequency control node and +12V
- **Function:** Pullup resistor for frequency control; sets control voltage biasing

**FREQ_CTRL** at (77.47, 96.52)
- **Function:** External frequency control CV input
- **Wire connection:** (77.47, 96.52) → (80.01, 96.52) → through R10 network → (90.17, 96.52) → (100.33, 96.52) to U2 frequency input

**Power supplies for U2:**
- **+12V:** Connected at (194.31, 102.87) via U2 pin connection
- **-12V:** Connected at (198.12, 124.46)

---

## Stage 4: Resonance Control Network
**RES_CTRL** at (215.9, 92.71)
- **Function:** External resonance (feedback/Q) control CV input
- **Wire connection:** (215.9, 92.71) → (210.82, 92.71) through R9

**R9 (100K)** - vertical resistor at (207.01, 92.71, rotated 270°)
- **Connection:** Resonance control input scaling resistor
- **Function:** RES_CTRL input impedance and gain setting

**R8 (51K)** - horizontal resistor at (210.82, 77.47, rotated 180°)
- **Connection:** From U2 resonance input node
- **Function:** Resonance feedback network resistor; sets resonance input gain

---

## Stage 5: Output Stage and Filtering
**U2 Output pins** from CEM3320 (high-pass configuration)
- The four filter poles output from U2

**C9 (300pF)** - small capacitor at (151.13, 68.58, rotated 270°)
- **Connection:** Shunt capacitor on first filter pole output from U2
- **Function:** Output decoupling / high-frequency compensation

**C10 (300pF)** - small capacitor at (176.53, 68.58, rotated 270°)
- **Connection:** Shunt capacitor on later filter pole output from U2
- **Function:** Additional output pole filtering and compensation

**R6 (100K)** - vertical resistor at (163.83, 68.58, rotated 270°)
- **Connection:** Between filter poles
- **Function:** Inter-pole coupling resistor; maintains impedance between cascaded filter stages

**Wire 7:** (167.64, 68.58) → (171.45, 68.58)
- Signal continues through filter cascade

**Wire 8:** (172.72, 68.58) → (180.34, 68.58)
- Signal progresses through remaining poles

**Wire 9:** (180.34, 68.58) → (184.15, 68.58)
- Final filter output routing

**R7 (100K)** - vertical resistor at (190.5, 68.58, rotated 270°)
- **Connection:** Output coupling/buffering resistor on final stage
- **Function:** Final output impedance buffering before output capacitor

---

## Stage 6: Output Coupling and Buffer
**C11 (1uF)** - polarized capacitor at (207.01, 68.58, rotated 90°)
- **Connection:** Output AC coupling capacitor
- **Function:** Blocks DC from the filter output while passing AC signal; provides output coupling to external loads

**R12 (1K)** - horizontal resistor at (198.12, 106.68)
- **Connection:** From U2 output node, going to trim pot
- **Function:** Output scaling/attenuation resistor

**RV1 (1.5K Trim Potentiometer)** - at (198.12, 119.38, rotated 180°)
- **Connection:** Between output and bias/trim network
- **Function:** Trim potentiometer for fine-tuning filter response or output gain calibration; connected to ±12V supply

**Wire 10:** (196.85, 68.58) (junction at output node)
- Main signal collection point before output coupling

**Wire 11:** (203.2, 68.58) → (210.82, 68.58)
- Signal path through output coupling capacitor C11

**Wire 12:** (210.82, 68.58) → (218.44, 68.58)
- **SIGNAL_OUT** at (218.44, 68.58)

---

## Signal Flow Summary

```
SIGNAL_IN (91.44, 68.58)
    ↓
[R4 100K + C7 300pF] — Input impedance network
    ↓
[R5 100K + C8 300pF + C9 300pF + C10 300pF] — Pre-filter coupling
    ↓
[U2 CEM3320 High-Pass VCF] — Core 4-pole filter
    ├─ Frequency control: FREQ_CTRL → R10 → R11 (+12V bias)
    └─ Resonance control: RES_CTRL → R9 → R8
    ↓
[R6 100K] — Inter-pole coupling
    ↓
[R7 100K + C11 1uF] — Output coupling network
    ↓
[R12 1K + RV1 1.5K trim pot] — Output trim/attenuation
    ↓
SIGNAL_OUT (218.44, 68.58)
```

---

## Component Reference Summary

| Component | Value | Type | Position | Function |
|-----------|-------|------|----------|----------|
| R4 | 100K | Input impedance | 113.03, 68.58 | Input buffering |
| C7 | 300pF | Input coupling | 100.33, 68.58 | AC coupling to ground |
| R5 | 100K | Pre-filter | 139.7, 68.58 | VCF input isolation |
| C8 | 300pF | Pre-filter | 125.73, 68.58 | Frequency shaping |
| C9 | 300pF | Pole coupling | 151.13, 68.58 | Pole 1 decoupling |
| C10 | 300pF | Pole coupling | 176.53, 68.58 | Pole 3 decoupling |
| U2 | CEM3320 | VCF chip | 151.13, 87.63 | 4-pole high-pass filter |
| R6 | 100K | Inter-pole | 163.83, 68.58 | Pole coupling impedance |
| R7 | 100K | Output buffer | 190.5, 68.58 | Output impedance buffering |
| R8 | 51K | Resonance feedback | 210.82, 77.47 | Q feedback gain |
| R9 | 100K | Resonance input | 207.01, 92.71 | RES_CTRL impedance |
| R10 | 100K | Frequency input | 83.82, 96.52 | FREQ_CTRL impedance |
| R11 | 1.8K | Frequency bias | 90.17, 102.87 | CV bias to +12V |
| R12 | 1K | Output trim | 198.12, 106.68 | Output scaling |
| C11 | 1uF | Output coupling | 207.01, 68.58 | Output AC coupling |
| RV1 | 1.5K | Trim potentiometer | 198.12, 119.38 | Fine-tune calibration |

---

## Notes

1. **High-Pass Configuration:** This is a high-pass filter variant of the CEM3320, configured to pass high frequencies and attenuate low frequencies.

2. **4-Pole Design:** The CEM3320 provides four cascaded poles of filtering (4-pole = 24 dB/octave rolloff).

3. **Control Inputs:** The filter includes voltage-controlled frequency and resonance/Q control for dynamic filter sweeping.

4. **Signal Conditioning:** The three 300pF capacitors (C7, C8, C9, C10) provide input conditioning and pole-to-pole AC coupling, shaping the frequency response.

5. **Output Buffering:** The output stage uses R7, C11, and trim network (R12 + RV1) to provide level-adjusted, DC-blocked output suitable for driving other modules.
