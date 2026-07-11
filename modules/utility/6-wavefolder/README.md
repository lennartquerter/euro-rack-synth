# WaveFolder

A 6-stage diode wavefolder for Eurorack. A simple sine or triangle in, a buzzy
animated waveform out. Topology is a Lockhart-style diode ladder driven by a
BC847 differential pair, summed by a TL074 op-amp, soft-clipped, and buffered
out.

## Current Version: 1

| Field  | Status                  |
|--------|-------------------------|
| Tested | No                      |
| Built  | Yes — outputs sound, but tone is off vs. the reference; investigation ongoing |
| Length | 110 mm                  |
| Width  | 4 HP (~20 mm)           |
| HP     | 4                       |
| Power  | +12 V / -12 V (no +5 V) |

## Inputs

| Jack | Name          | Range              | Function                                                                 |
|------|---------------|--------------------|--------------------------------------------------------------------------|
| J1   | `WAVE_INPUT`  | ±5 V audio         | Audio input. Drives the base of the BC847 differential pair (Q2/Q3) via R1. |
| J2   | `CTRL_INPUT`  | 0 → +5 V CV        | Folding-amount CV. Sums with the *Range* pot at U1A and biases the diff-pair tail through Q1. |

Panel controls:

- **Input Level** (RV1, B100K) — attenuates the CV at `CTRL_INPUT` before it hits U1A.
- **Range** (RV2, B100K) — sets the static fold amount (DC bias on Q1's base, swept ±12 V).
- **Shape** (RV5, B100K) — variable-resistor tap into the diode-ladder feedback path; controls how aggressively the folded signal is fed back into U2B.
- **Trim RV3** (B100K, Bourns) — DC offset trim for U1A. *Originally B50K.*
- **Trim RV4** (1K, Bourns) — collector-balance trim for the Q2/Q3 diff pair (across R8/R11 to +12 V).

## Output

| Jack | Name     | Range              | Function                                                       |
|------|----------|--------------------|----------------------------------------------------------------|
| J3   | `OUTPUT` | ±5 V audio         | Final folded output. U2D inverting buffer with R36 1 K series resistor. |

Test points along the signal path:

| TP   | Net               | What it shows                                |
|------|-------------------|----------------------------------------------|
| TP1  | +12 V             | Positive rail after D1/R15 protection.       |
| TP2  | GND               | Common ground.                               |
| TP3  | -12 V             | Negative rail after D2/R16 protection.       |
| TP4  | INPUT             | Q2 base — buffered audio input.              |
| TP5  | SHAPE_INPUT       | U1B output — diff-pair difference, pre-fold. |
| TP6  | FOLDER_OUT        | End of the diode-ladder fold chain.          |
| TP7  | FOLDER_HALF_OUT   | Middle tap of the diode-ladder fold chain.   |
| TP8  | OUTPUT            | U2D output — same node as J3 tip.            |
| TP9  | FINAL_STAGE       | U2A output — soft-clipped, pre-output buffer.|

## Signal flow

```
J1 ─[R1 100K]─┬─ Q2 base ──┐
              R5 680R       ├── diff pair Q2/Q3 ── R12/R13 ── U1B ── TP5
              GND          │   tail ─[R9 33K]─ Q1-E ─[R10 15K]─ -12V
                            │   collectors ─ R8/R11 ─ RV4 ─ +12V
J2 ─ RV1 ─ R2 ─ U1A ─ R6 ─┐
                            ├── Q1 base
RV2 (Range) ─ R7 ─────────┘

U1B ── R18 ── U2C  ── R20 fb ──┐
                                ├── diode ladder (D3..D20, R10/R19/R21..R32)
                                │     ├── TP7 (half)
                                │     └── TP6 (end)
                                └── back to U2C inverting input

RV5 (Shape) ── R26 ── U2B ── R29 ──┐
                       │            ├── U2A with D21/D22 anti-parallel feedback ── TP9
                       └── R31 ─────┘
                                          │
                                R33 ── R34 ─ U2D ── R36 ── J3 / TP8
```

## Build Guide

### Bill of Materials

#### Semiconductors

| Ref            | Value     | Footprint   | Qty |
|----------------|-----------|-------------|-----|
| U1             | TL072     | SO-8        | 1   |
| U2             | TL074     | SO-14       | 1   |
| Q1, Q2, Q3     | BC847     | SOT-23      | 3   |
| D1, D2         | 1N5819    | SOD-123 SMD | 2   |
| D3 – D22       | 1N4148W   | SOD-123 SMD | 20  |

#### Resistors (all 0805 SMD; `[1%]` indicates 1 % tolerance is required for matching)

| Ref                                                | Value      | Qty |
|----------------------------------------------------|------------|-----|
| R5                                                 | 680 R      | 1   |
| R36                                                | 1 K        | 1   |
| R24                                                | 2.7 K      | 1   |
| R29, R31                                           | 5.6 K      | 2   |
| R34                                                | 6.8 K      | 1   |
| R12, R13                                           | 10 K [1%]  | 2   |
| R8, R11                                            | 15 K [1%]  | 2   |
| R10, R19, R21, R23, R25, R27, R28, R30, R32        | 15 K       | 9   |
| R6, R7                                             | 22 K       | 2   |
| R26                                                | 27 K       | 1   |
| R9, R22                                            | 33 K       | 2   |
| R33, R35                                           | 56 K       | 2   |
| R2                                                 | 68 K       | 1   |
| R1, R3, R4, R18, R20                               | 100 K      | 5   |
| R14, R17                                           | 100 K [1%] | 2   |
| R15, R16                                           | 10 R       | 2   |

#### Capacitors

| Ref            | Value  | Footprint                | Qty |
|----------------|--------|--------------------------|-----|
| C3, C4, C5, C6 | 100 nF | 0805 SMD                 | 4   |
| C1, C2         | 47 µF  | THT radial 6.3 mm, polarized | 2 |

#### Pots & connectors

| Ref          | Value  | Type                      | Qty |
|--------------|--------|---------------------------|-----|
| RV1, RV2, RV5| B100K  | TT P0915N panel pot       | 3   |
| RV3          | B100K  | Bourns 3296W vertical trim| 1   |
| RV4          | 1 K    | Bourns 3296W vertical trim| 1   |
| J1, J2, J3   | —      | Thonkiconn / WQP-PJ398SM 3.5 mm jack | 3 |
| H1           | —      | 2×5 IDC Eurorack power header | 1 |

#### Test points

TP1..TP9 — single 1 mm pin, 10 mm long. Optional but very helpful for bring-up.

### Soldering order

Solder lowest-profile components first so each new layer rests flat on the
table:

1. **SMD resistors and 0805 caps.** Stencil + reflow if you have it; otherwise hand-solder.
2. **SMD diodes** — D1/D2 (1N5819 Schottkys, polarized) and D3..D22 (1N4148W, polarized). Match the cathode bar on the part to the silkscreen line.
3. **SOT-23 transistors** Q1/Q2/Q3 — pin 1 (base) goes to the silkscreen mark.
4. **TL072 (U1) and TL074 (U2)** — pin-1 dot to the silkscreen notch.
5. **Trim pots** RV3, RV4.
6. **Pin headers** for the test points (TP1..TP9).
7. **Eurorack power header** H1 — note the orientation: `-12 V` (red stripe of the ribbon cable) goes to pins 1–2.
8. **Through-hole electrolytics** C1, C2 — `+` lead to the marked pad.
9. **Panel components last:** the three jacks (J1/J2/J3) and three panel pots (RV1, RV2, RV5). Mount them through the panel before soldering, so the panel sets the alignment.

### First-power-up checklist

1. With nothing plugged in, plug in the Eurorack ribbon cable. Check that the
   3.3 mm fuse on your bus board doesn't pop and that the rails are present.
2. Probe TP1 and TP3 with a multimeter — should read +12 V and -12 V to ground.
3. Inject ~2 Vpp of a 100 Hz triangle into J1.
4. Probe TP4 — should track the input.
5. Probe TP5 — should be a scaled and inverted version of the input (sub-fold).
6. Probe TP6 / TP7 — should show the folded waveform.
7. Probe TP9 / TP8 — should show the final, soft-clipped output.

## Differences from the Yusynth reference

This module is adapted from Yves Usson's wave-multiplier (Yusynth wave-folder).
The signal-path topology is faithfully copied: same diff-pair input, same
Q1-collector-to-GND emitter-follower bias generator, same diode ladder with
two summing rails, same SHAPE rheostat with one terminal floating. Only the
items below are intentionally different.

| Item                | Reference | v1     | Why / effect                                               |
|---------------------|-----------|--------|------------------------------------------------------------|
| Power rails         | ±15 V     | ±12 V  | Eurorack adaptation. ~20 % less headroom; soft-clip and saturation thresholds scale with the rail. |
| BIAS trim           | A2 = 50 K | RV3 = 100 K  | Wider sweep; behaves the same at centre.             |
| BIAS trim path      | A2 → R34 → U2b non-inv input | RV3 → R7 → Q1 base directly, summed with U1A output | Same DC effect on Q1 base; BIAS knob direction is *inverted* relative to the reference. |
| SHAPE pot           | P2 = 47 K | RV5 = 100 K  | ~2× wider rheostat range; otherwise identical.       |
| CV1 input resistor  | R30 = 22 K| R2 = 68 K    | CV1 modulation depth ~3× weaker than reference.       |
| Output series R     | 470 Ω     | R36 = 1 K    | ~0.5 dB more attenuation into a 100 K Eurorack load — inaudible. |
| Bulk cap            | C1, C2 = 22 µF | C1, C2 = 47 µF | Bigger reservoir; no audible effect.            |
| Reverse-polarity    | (none)    | D1, D2 = 1N5819 | Eurorack-specific addition; standard practice.   |

## Build / debug checklist if v1 sounds wrong

If the module powers up but the sound is off, these are easier to chase than
schematic differences:

- **Eurorack ribbon orientation** — red stripe (-12 V) on pins 1–2 of H1.
- **D1, D2 (1N5819)** — cathode bar matches silkscreen, otherwise no power.
- **C1, C2** electrolytic polarity matches the silkscreen `+`.
- **U1, U2** pin-1 dot to the silkscreen notch.
- **Q1, Q2, Q3 (BC847 SOT-23)** — pin-1 (base) on the silkscreen mark; SOT-23
  packages are easy to put down rotated 180°.
- **Pots** — B100K Alps clones occasionally ship with a broken wiper or
  reversed taper. Verify each with an ohmmeter: end-to-end ≈ 100 K, end-to-
  wiper sweeps smoothly from 0 to 100 K as you turn.
- **Probe in order:** TP4 (input at Q1 emitter node) → TP5 (U1B output, post-
  diff-pair) → TP6/TP7 (note: these are op-amp virtual grounds, so you'll
  see ~0 V — that's normal) → TP9 (U2A output, post-soft-clip) → TP8
  (final output).
- With no input plugged in, slowly sweep RV2 (Range): you should hear the
  diff-pair operating point shift — silence at one end, noise/oscillation at
  the other. If nothing moves, the BIAS path through Q1 is broken.
