# CEM3320 High-Pass Filter Signal Path Analysis

## Signal Path Summary

The signal flows through a single 4-pole cascaded high-pass filter architecture implemented by the CEM3320 VCF chip (U2). The design uses AC-coupling at input and output, with independent resonance and frequency control inputs.

## Stage-by-Stage Breakdown

### Input Stage: AC-Coupling
- **SIGNAL_IN** (external) → **C7** (300pF) → **U2 pin 1** (INPUT_A)
  - C7 is the first AC-coupling capacitor that blocks DC and passes the audio signal into the filter's first input.

### Filter Core: CEM3320 (U2)
The chip implements a 4-pole cascaded filter with internal state:

**Stage A (Pole 1):**
- Input: **U2 pin 1** (INPUT_A)
- Pole capacitor: **C7** (300pF, coupled at pin 5 CAP_A)
- Output: **U2 pin 7** (OUT_A)

**Stage B (Pole 2):**
- Input: **U2 pin 2** (INPUT_B) ← receives **U2 pin 7** (OUT_A) via **R5** (100K)
- Pole capacitor: **C9** (300pF, coupled at pin 16 CAP_C)  
- Output: **U2 pin 6** (OUT_B)

**Stage C (Pole 3):**
- Input: **U2 pin 17** (INPUT_C) ← receives **U2 pin 15** (OUT_C) via **R6** (100K)
- Pole capacitor: **C10** (300pF, coupled at pin 11 CAP_D)
- Output: **U2 pin 15** (OUT_C)

**Stage D (Pole 4):**
- Input: **U2 pin 18** (INPUT_D) ← receives **U2 pin 10** (OUT_D) via **R7** (100K)
- Pole capacitor: **C10** (300pF, coupled at pin 11 CAP_D)
- Output: **U2 pin 10** (OUT_D)

### Control Inputs

**Frequency Control:**
- **FREQ_CTRL** (external) → **R10** (100K) → **U2 pin 12** (FREQ_CTRL)
- A voltage divider via **R11** (1.8K) to GNDREF sets the base frequency response

**Resonance Control:**
- **RES_CTRL** (external) → **R9** (100K) → **U2 pin 9** (RES_CTRL)
- Controls feedback depth (Q) of the filter cascade

**Bias Current Supply:**
- **U2 pin 13** (IEE) ← **R12** (1K) + **RV1** (1.5K trim pot)
- The bias network (RV1 between +12V and -12V, with R12 providing the current source) sets the VCF's operating point and overall gain

### Output Stage: Post-Filter Processing
- **U2 pin 8** (FILTER_OUT) → **R8** (51K) → **SIGNAL_OUT** and **C11** (1µF)
  - R8 is an output impedance buffer
  - C11 provides AC-coupling at the final output node

### Power Supply
- **U2 pin 14** (VCC) ← +12V rail
- **U2 pin 3** (GND) ← GNDREF (0V reference)
- Bias current pin (pin 13) taps into the -12V supply via the trim pot network

## Key Design Features

1. **High-pass characteristic:** All four pole capacitors (C7, C8, C9, C10) are small film caps (300pF), creating a high-pass response with corner frequency set by FREQ_CTRL.

2. **Cascaded topology:** Each stage feeds into the next via 100K resistors (R4, R5, R6, R7), creating a unified 4-pole response.

3. **Resonance feedback:** The RES_CTRL pin modulates the internal feedback path to add peaks near the cutoff (classic Moog-style response).

4. **Bias trim:** RV1 allows fine-tuning of the VCF's current bias for optimal frequency tracking and gain.

5. **AC-coupled I/O:** Both input (C7) and output (C11) use capacitive coupling to isolate DC levels while passing audio.
