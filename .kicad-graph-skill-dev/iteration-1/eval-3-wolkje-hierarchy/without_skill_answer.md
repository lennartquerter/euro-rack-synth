# Wolkje Brain Board - Architectural Overview

## Hierarchy Summary

The **wolkje-brain** is a 3-level hierarchical KiCad project (root + 3 sub-sheets) cloning the Mutable Instruments Clouds processor:

### Root Sheet: `wolkje-brain.kicad_sch` (Page 1)
- **Components**: 10 on root (J1–J3, R1, D1, power flags & planes)
- **Role**: Power distribution & connectors; instantiates the three sub-sheets

### Sub-Sheets

#### 1. **stm32.kicad_sch** (Page 2) – Microcontroller Core
- **Components**: 41 actual + 29 power nodes = ~70 references
  - **U1**: STM32F405RGT6 (Arm Cortex-M4, 1024 KB flash, 192 KB RAM, 168 MHz)
  - **J4–J7**: Debug/SPI/programming connectors
  - **Y1**: Crystal oscillator
  - **C1–C12, R2–R5**: Support capacitors & resistors (~12 caps, 5 resistors)
  - **FB1, D2**: Ferrite bead & diode (power conditioning)

#### 2. **signal_connector.kicad_sch** (Page 3) – Analog Input Stage
- **Components**: 8 connectors (J8–J11) + power decoupling
  - **J8–J11**: Front-panel CV/trigger inputs for user control
  - Role: Routes analog signals from Eurorack module interface to STM32

#### 3. **audio-codec.kicad_sch** (Page 4) – Audio I/O
- **Components**: 24 actual + 12 power nodes = ~36 references
  - **U2**: WM8731SEDS (Wolfson stereo audio codec with headphone driver, SSOP-28)
  - **Y2**: Clock for codec
  - **C13–C21, R6–R7**: Filtering & bias (~9 caps, 2 resistors)
  - Role: Handles ADC input and DAC output audio processing

## Cross-Sheet Signals (Sheet Pins)

The STM32 is the **communication hub**. All signals pass through it:

### STM32 → Signal Connectors (Inputs)
From `signal_connector` sheet pins (right side of sheet):
- **Control CV inputs**: BLEND_CV, DENSITY_CV, POSITION_CV, SIZE_CV, TEXTURE_CV
- **Switch inputs**: SW_FREEZE, SW_MODE, SW_LOAD
- **Trigger inputs**: PLAY_TRIGGER, FREEZE_TRIGGER
- **Knob inputs**: BLEND, TEXTURE, PITCH, SIZE, V_OCT

### STM32 ↔ Audio Codec (I2S/I2C)
I2S (digital audio) from `audio-codec` sheet pins:
- **I2S_SIN**: Audio in (serial data from codec ADC)
- **I2S_SOUT**: Audio out (serial data to codec DAC)
- **I2S_LRCK**: Left/Right clock
- **I2S_SCK**: Serial clock

Control bus:
- **I2C_SCL, I2C_SDA**: I2C config lines for codec

Clock distribution:
- **CLK, LRCLK**: Codec timing signals (from STM32 or crystal)
- **ADC_DATA, DAC_DATA**: Audio sample clocks
- **SCLK, SDIN**: Serial control interface

### STM32 → LEDs
- **LED_DATA, LED_CLK, LED_ENABLE**: Shift-register protocol for RGB LED control

## Component Distribution

| Sheet | IC count | Passive count | Purpose |
|-------|----------|--------------|---------|
| Root  | 0        | 1 (R1)       | Power/connectors |
| STM32 | 1 (MCU)  | 12 C + 5 R + FB + diode | Processor & decoupling |
| Signal Connector | 0 | 4 connectors | Analog I/O |
| Audio Codec | 1 (WM8731) | 9 C + 2 R | ADC/DAC |

## High-Level Data Flow

```
Analog Inputs (J8–J11)
        ↓
Signal Connector Sheet (conditioning)
        ↓
STM32 F4 (main processor)
        ↓
Audio Codec (WM8731) ←→ L/R audio I/O
        ↓
LED control output
```

The STM32F405 is the **sole processor** handling:
- Real-time audio DSP (Clouds algorithm clone)
- CV input processing (14 analog parameters)
- Switch/button scanning
- I2S audio streaming
- LED feedback
