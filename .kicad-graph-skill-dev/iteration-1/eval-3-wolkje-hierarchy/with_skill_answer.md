# Wolkje Brain Board Architecture Overview

## Summary
The Wolkje brain board is a 4-sheet hierarchical design centered on an **STM32F405RGT6 microcontroller**. The STM32 orchestrates a **WM8731 audio codec** (12-bit stereo I2S) and connects to an external **signal connector interface** that exposes 16 control inputs (CV, triggers, switches) for patch-bay integration.

**Components across sheets:** 46 total | **Nets:** 87 total

---

## Sheet Organization

| Sheet | File | Components | Purpose |
|-------|------|-----------|---------|
| **Root `/`** | wolkje-brain.kicad_sch | 5 | Power distribution, I/O jacks (audio in/out, LED driver) |
| **`/STM32`** | stm32.kicad_sch | 24 | Microcontroller + clock, reset, UART, SWD debug, power sequencing |
| **`/signal_connector`** | signal_connector.kicad_sch | 4 | 4× 6/8-pin headers for CV inputs, triggers, and switches |
| **`/audio-codec`** | audio-codec.kicad_sch | 13 | WM8731 + crystal clock, bias resistors, coupling capacitors |

---

## Cross-Sheet Signal Flow

### Power Rails (span all or most sheets)
- **`GNDREF`**: Ground reference — 82 connections across all 4 sheets
- **`+3.3V`**: Main logic rail — STM32, codec, signal conditioning
- **`+3.3VA`**: Isolated analog rail — codec audio supplies + STM32 VDDA pin (filtered through ferrite bead FB1)

### Data Signals: STM32 ↔ Audio Codec
The STM32 **does not directly drive audio**. Instead, it communicates via an I²S interface (likely on shared power pins; detailed I²S pins not exposed in cross-sheet nets). The codec receives:
- **`+3.3V`** to DBVDD (digital power), DCVDD (digital core)
- **`+3.3VA`** to AVDD (analog), HPVDD (headphone amp)
- **Clock:** 12.288 MHz crystal (Y2) drives codec; STM32 likely derives timing from this or generates it internally

### Control Signals: STM32 ↔ Signal Connector
The STM32 exposes **17 signal-level I/O pins** to the external connector interface (across J8, J9, J10, J11):

**Analog Control Inputs (CV, 6 inputs):**
- PA0 → J8 pin 7: `TEXTURE_CV`
- PA1 → J8 pin 6: `SIZE_CV`
- PA2 → J8 pin 5: `POSITION_CV`
- PA3 → J8 pin 4: `DENSITY_CV`
- PA4 → J8 pin 3: `V_OCT` (pitch quantization)
- PA5 → J8 pin 2: `BLEND_CV`

**Digital/Trigger Inputs (switches & gates, 7 inputs):**
- PA6 → J9 pin 4: `TEXTURE` (mode/knob)
- PA7 → J9 pin 5: `BLEND` (mode/knob)
- PC0 → J9 pin 2: `SIZE` (mode/knob)
- PC1 → J9 pin 3: `PITCH` (mode/knob)
- PB6 → J10 pin 4: `PLAY_TRIGGER` (gate in)
- PB7 → J10 pin 3: `FREEZE_TRIGGER` (gate in)
- PC10 → J11 pin 2: `SW_LOAD` (switch)

**LED Outputs (3 outputs):**
- PB8 → J11 pin 4: `SW_FREEZE`
- PB9 → J11 pin 5: `FREEZE_LED` (status)
- PC11 → J11 pin 3: `SW_MODE`

---

## Key Design Patterns

1. **Processor-Centric**: STM32 is the sole arbiter of all signal flow — it reads CV and gates from the signal connector, drives LEDs, and controls the audio codec (likely via I²S + SPI/I²C for codec configuration).

2. **Power Sequencing**: Analog rail (`+3.3VA`) is filtered separately for the codec (ferrite bead + capacitors) to isolate audio from digital noise.

3. **Debug & Bootstrap**: Two separate connectors (`J5` BOOT/RUN, `J7` SWD) allow firmware updates and in-circuit debugging without disturbing the audio path.

4. **External Interface**: All Eurorack-style I/O (audio in/out, CV, gates, triggers) is consolidated on J1–J3 on the root sheet; the signal connector headers (J8–J11) are the internal breakout for the modular control.

---

## STM32 I/O Summary
- **Analog inputs:** PA0–PA7, PC1 (3.3V sampled by internal ADC)
- **Digital outputs:** PB6–B9, PC10–PC11, PC0 (GPIO push-pull)
- **Clocking:** 8 MHz crystal (Y1) via PC15/PC14; codec clock derived separately
- **Debug:** UART (J6), SWD (J7) for development

This is a **classic DSP board design**: fixed hardware audio path (STM32 → codec → jacks) with flexible control surface exposed via header connectors for external patch-bay systems.
