# Midas v3 — Pattern Generator + MIDI-2-CV Firmware

THIS IS AN ALTERNATE VERSION SOFTWARE BRIEF --> NOT BUILT!!!

## Overview

Repurpose the existing MIDI-2-CV module (Midas v2) into a dual-function module:
a pattern/sequence generator for live performance AND a 2-channel polyphonic
MIDI-to-CV converter. The hardware stays unchanged — this is firmware only.

Module name: **Midas v3** (or "Midas Pattern" if we want to distinguish)


## Hardware Recap

- MCU: STM32F411 (100 MHz, 512KB flash, 128KB RAM)
- DACs: 3x MCP4728 (quad 12-bit, I2C) = 12 CV outputs
- Gates: 4x GPIO outputs
- Input: MIDI via UART1 @ 31250 baud
- Control: 3-position mode switch (EXTI interrupt)
- Timer: TIM1 available for clock generation
- No knobs, no buttons, no display on the module itself

### Output Mapping (per channel)

| Channel | DAC Bus | CV-A (ch0) | CV-B (ch1) | CV-C (ch2) | Gate |
|---------|---------|------------|------------|------------|------|
| 1       | I2C1    | Output 1   | Output 2   | Output 3   | Gate 1 |
| 2       | I2C2    | Output 4   | Output 5   | Output 6   | Gate 2 |
| 3       | I2C3    | Output 7   | Output 8   | Output 9   | Gate 3 |
| 4       | I2C3*   | Output 10  | Output 11  | Output 12  | Gate 4 |

*Note: verify actual I2C bus mapping — may need adjustment based on PCB routing.*


## Mode Switch (3 positions)

### Position 1: Full Pattern Generator (4 channels)

All 4 channels run pattern sequences. Gate 1 is the master clock output.

| Output   | Channel 1       | Channel 2       | Channel 3       | Channel 4       |
|----------|-----------------|-----------------|-----------------|-----------------|
| CV-A     | Melody pitch    | Melody pitch    | Melody pitch    | Melody pitch    |
| CV-B     | Bass pitch      | Bass pitch      | Bass pitch      | Bass pitch      |
| CV-C     | Accent (0V/5V)  | Accent (0V/5V)  | Accent (0V/5V)  | Accent (0V/5V)  |
| Gate     | **MASTER CLOCK**| Rhythm pattern  | Rhythm pattern  | Rhythm pattern  |

- Gate 1 pulses on every step (16th notes at the set BPM)
- Gates 2-4 follow their channel's rhythm pattern (can be Euclidean or custom)
- Patch Gate 1 to your clock divider or other modules that need sync

### Position 2: Pattern Gen (ch 1-2) + MIDI Poly (ch 3-4)

| Output   | Channel 1       | Channel 2       | Channel 3       | Channel 4       |
|----------|-----------------|-----------------|-----------------|-----------------|
| CV-A     | Melody pitch    | Melody pitch    | MIDI pitch      | MIDI pitch      |
| CV-B     | Bass pitch      | Bass pitch      | MIDI velocity   | MIDI velocity   |
| CV-C     | Accent (0V/5V)  | Accent (0V/5V)  | MIDI mod wheel  | MIDI mod wheel  |
| Gate     | **MASTER CLOCK**| Rhythm pattern  | MIDI gate       | MIDI gate       |

- Pattern channels 1-2 run independently
- MIDI channels 3-4 operate as 2-voice polyphonic (from MIDI channel 1)
- Master clock still on Gate 1 (patterns run on internal clock)

### Position 3: Full MIDI Mode (original Midas v2 behavior)

| Output   | Channel 1       | Channel 2       | Channel 3       | Channel 4       |
|----------|-----------------|-----------------|-----------------|-----------------|
| CV-A     | MIDI pitch      | MIDI pitch      | MIDI pitch      | MIDI pitch      |
| CV-B     | MIDI velocity   | MIDI velocity   | MIDI velocity   | MIDI velocity   |
| CV-C     | MIDI mod wheel  | MIDI mod wheel  | MIDI mod wheel  | MIDI mod wheel  |
| Gate     | MIDI gate       | MIDI gate       | MIDI gate       | MIDI gate       |

- 4-voice polyphonic from MIDI channel 1 (existing POLY mode)
- Essentially the original v2 firmware behavior
- No internal clock, no pattern generation


## Internal Clock System

### Implementation

Use TIM1 as the master clock source. Configure as a periodic interrupt that
fires at the step rate. No MIDI clock sync — keep it simple.

```c
// Clock is driven by BPM setting
// 16th note interval = 60000 / (BPM * 4) milliseconds
// At 120 BPM: 60000 / 480 = 125ms per step
// At 140 BPM: 60000 / 560 = 107ms per step

#define DEFAULT_BPM 120
#define STEPS_PER_BEAT 4  // 16th notes
```

### Gate Timing

- Gate HIGH duration: 50% of step length (adjustable in config)
- Gate 1 (master clock): always pulses on every step
- Gates 2-4: pulse only on active steps in their rhythm pattern

### BPM Control

Since there are no knobs on the module, BPM is set in firmware config
(see Pattern Configuration section below). For live use, consider building
a small MIDI controller that sends CC messages to change BPM in real-time.

Alternatively, a future hardware mod could add a pot to an unused ADC pin
on the STM32 — but that requires PCB changes.


## Pattern System

### Philosophy

All patterns are **pre-defined in firmware** as const arrays. No randomness,
no generative algorithms. This gives:

- 100% reproducible behavior (critical for live sets)
- Instant recall — same pattern every time you power on
- Easy to tweak — edit arrays in code, recompile, reflash
- No UI needed — patterns are "baked in"

### Pattern Structure

Each channel has a **Pattern Set** containing:

```c
typedef struct {
    uint8_t  melody[16];      // MIDI note numbers (0-127), quantized to scale
    uint8_t  bass[16];        // MIDI note numbers for bass line
    uint8_t  accent[16];      // 0 = no accent, 1 = accent
    uint8_t  rhythm[16];      // 0 = rest, 1 = gate HIGH on this step
    uint8_t  length;          // pattern length (1-16, default 16)
} Pattern;

typedef struct {
    Pattern patterns[NUM_PATTERNS];
    uint8_t current_pattern;
    uint8_t current_step;
} PatternChannel;
```

### Pattern Banks

The firmware ships with a configurable number of pattern banks. Each bank
contains a complete set of patterns for all active channels.

```c
#define NUM_PATTERN_BANKS  8   // 8 banks, selectable via MIDI or cycling
#define NUM_PATTERNS       4   // 4 patterns per bank (one per channel max)
```

**Bank switching**: In pattern mode, incoming MIDI Program Change messages
(or a specific CC) select the active bank. This is the "MIDI control surface"
hook — send a Program Change from any MIDI controller (even a cheap one)
to switch between your 8 pre-programmed banks during a live set.

Alternatively, banks auto-advance: after N repetitions of the full 16-step
pattern, automatically move to the next bank. Set N in config.


## Pattern Configuration File

All tweakable parameters live in a single header file for easy editing:

```c
// ============================================================
// patterns_config.h — EDIT THIS FILE TO CHANGE YOUR PATTERNS
// ============================================================

#ifndef PATTERNS_CONFIG_H
#define PATTERNS_CONFIG_H

// --- Global Settings ---
#define DEFAULT_BPM          120
#define GATE_DUTY_PERCENT    50      // gate HIGH as % of step length
#define STEPS_PER_BEAT       4       // 4 = 16th notes, 2 = 8th notes
#define AUTO_ADVANCE_BARS    0       // 0 = manual bank change only
                                     // N = advance bank every N bars

// --- Scale & Root ---
// Scale is used to quantize melody[] values if QUANTIZE_MELODY is enabled
#define QUANTIZE_MELODY      1       // 0 = raw MIDI notes, 1 = snap to scale
#define ROOT_NOTE            0       // 0=C, 1=C#, 2=D, ... 11=B
#define SCALE_TYPE           SCALE_MINOR_PENTATONIC

// Available scales:
// SCALE_CHROMATIC, SCALE_MAJOR, SCALE_MINOR, SCALE_DORIAN,
// SCALE_MIXOLYDIAN, SCALE_MINOR_PENTATONIC, SCALE_MAJOR_PENTATONIC,
// SCALE_BLUES, SCALE_HARMONIC_MINOR

// --- Voltage Mapping ---
#define NOTE_VOLTAGE_MIN     0       // DAC value for lowest note
#define NOTE_VOLTAGE_MAX     2000    // DAC value for highest note (matches v2)
#define ACCENT_VOLTAGE       1600    // DAC value when accent is ON (~4V)
#define ACCENT_OFF_VOLTAGE   0       // DAC value when accent is OFF

// --- Pattern Banks ---
// Each bank has patterns for all active channels.
// In Position 1 (4-channel): banks contain 4 patterns
// In Position 2 (2-channel): banks contain 2 patterns (ch 3-4 are MIDI)

// BANK 0 — "Minimal Techno"
#define BANK0_CH1_MELODY   { 48, 48, 48, 51, 48, 48, 53, 48, 48, 48, 51, 55, 48, 48, 53, 48 }
#define BANK0_CH1_BASS     { 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36 }
#define BANK0_CH1_ACCENT   {  1,  0,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  0,  1,  0 }
#define BANK0_CH1_RHYTHM   {  1,  0,  1,  0,  1,  0,  1,  0,  1,  0,  1,  0,  1,  0,  1,  0 }

#define BANK0_CH2_MELODY   { 60, 63, 60, 67, 60, 63, 60, 65, 60, 63, 60, 67, 60, 63, 65, 60 }
#define BANK0_CH2_BASS     { 36, 36, 36, 36, 39, 39, 39, 39, 41, 41, 41, 41, 36, 36, 36, 36 }
#define BANK0_CH2_ACCENT   {  1,  0,  0,  1,  0,  0,  1,  0,  1,  0,  0,  1,  0,  0,  0,  1 }
#define BANK0_CH2_RHYTHM   {  1,  1,  0,  1,  0,  1,  1,  0,  1,  0,  1,  0,  1,  1,  0,  1 }

#define BANK0_CH3_MELODY   { 55, 55, 58, 55, 60, 55, 58, 55, 55, 55, 58, 55, 62, 60, 58, 55 }
#define BANK0_CH3_BASS     { 43, 43, 43, 43, 43, 43, 43, 43, 41, 41, 41, 41, 41, 41, 41, 41 }
#define BANK0_CH3_ACCENT   {  0,  0,  1,  0,  0,  0,  1,  0,  0,  0,  1,  0,  1,  0,  0,  0 }
#define BANK0_CH3_RHYTHM   {  1,  0,  0,  1,  0,  0,  1,  0,  1,  0,  0,  1,  0,  1,  0,  0 }

#define BANK0_CH4_MELODY   { 48, 48, 48, 48, 51, 51, 51, 51, 53, 53, 53, 53, 55, 55, 55, 55 }
#define BANK0_CH4_BASS     { 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36 }
#define BANK0_CH4_ACCENT   {  1,  0,  0,  0,  1,  0,  0,  0,  1,  0,  0,  0,  1,  0,  0,  0 }
#define BANK0_CH4_RHYTHM   {  1,  0,  0,  0,  1,  0,  0,  0,  1,  0,  0,  0,  1,  0,  0,  0 }

// BANK 1 — "Acid Line"
#define BANK1_CH1_MELODY   { 48, 51, 53, 48, 55, 48, 51, 56, 48, 53, 48, 55, 51, 48, 56, 53 }
#define BANK1_CH1_BASS     { 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36 }
#define BANK1_CH1_ACCENT   {  1,  0,  1,  0,  0,  1,  0,  1,  1,  0,  0,  1,  0,  1,  0,  0 }
#define BANK1_CH1_RHYTHM   {  1,  1,  1,  0,  1,  1,  0,  1,  1,  1,  0,  1,  1,  0,  1,  1 }

#define BANK1_CH2_MELODY   { 60, 60, 63, 65, 60, 60, 67, 65, 63, 60, 60, 65, 63, 60, 67, 60 }
#define BANK1_CH2_BASS     { 36, 36, 36, 36, 39, 39, 39, 39, 41, 41, 36, 36, 39, 39, 41, 36 }
#define BANK1_CH2_ACCENT   {  1,  0,  0,  0,  1,  0,  0,  0,  1,  0,  0,  0,  1,  0,  0,  0 }
#define BANK1_CH2_RHYTHM   {  1,  0,  1,  1,  1,  0,  1,  0,  1,  1,  0,  1,  1,  0,  1,  0 }

#define BANK1_CH3_MELODY   { 55, 58, 55, 60, 55, 58, 62, 60, 58, 55, 58, 60, 62, 60, 58, 55 }
#define BANK1_CH3_BASS     { 43, 43, 43, 43, 43, 43, 43, 43, 41, 41, 41, 41, 41, 41, 41, 41 }
#define BANK1_CH3_ACCENT   {  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0 }
#define BANK1_CH3_RHYTHM   {  1,  0,  1,  0,  0,  1,  0,  1,  0,  1,  0,  1,  1,  0,  0,  1 }

#define BANK1_CH4_MELODY   { 48, 48, 51, 48, 53, 48, 48, 51, 48, 48, 53, 51, 48, 48, 51, 48 }
#define BANK1_CH4_BASS     { 36, 36, 36, 36, 38, 38, 38, 38, 41, 41, 41, 41, 36, 36, 36, 36 }
#define BANK1_CH4_ACCENT   {  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  1,  0,  0,  0 }
#define BANK1_CH4_RHYTHM   {  1,  0,  0,  1,  0,  1,  0,  0,  1,  0,  1,  0,  0,  1,  0,  1 }

// ... BANK 2 through BANK 7: define similarly ...
// (copy a bank above and modify the arrays)

#endif // PATTERNS_CONFIG_H
```


## MIDI Control (optional, for live use)

Even in pattern mode, the UART still receives MIDI. Use it for real-time control:

| MIDI Message         | Function                              |
|----------------------|---------------------------------------|
| Program Change 0-7   | Select pattern bank 0-7               |
| CC 1 (Mod Wheel)     | Transpose all melodies (semitones)    |
| CC 7 (Volume)        | Adjust accent voltage level           |
| CC 14                | Change BPM (map 0-127 to 60-240 BPM) |
| CC 15                | Change gate duty cycle (0-100%)       |
| Note On (any)        | Reset pattern to step 1 (re-sync)     |

This means any cheap MIDI controller (even a phone app) becomes a live
control surface. Send Program Change to switch banks, twist a knob for
BPM, hit a pad to re-sync the pattern.


## Software Architecture

### New Files to Create

```
Core/Src/app/
├── pattern_engine.c      // Pattern playback engine
├── pattern_engine.h      // Pattern types and API
├── patterns_config.h     // THE file to edit for custom patterns
├── clock.c               // TIM1-based internal clock
├── clock.h               // Clock API
├── scale.c               // Scale quantization tables
├── scale.h               // Scale definitions
├── midi_handler.c        // (keep existing)
├── midi_processor.c      // (modify: add 2-voice poly mode)
├── mcp4728.c             // (keep existing)
└── buffer.c              // (keep existing)
```

### Modified Files

**main.c** — Add pattern engine initialization, clock setup, mode switching
logic for 3 positions. Main loop becomes:

```c
while (1) {
    switch (current_mode) {
        case MODE_FULL_PATTERN:
            // Clock tick drives pattern_engine for all 4 channels
            // MIDI input used for bank select / transpose / BPM only
            break;

        case MODE_PATTERN_AND_MIDI:
            // Clock tick drives pattern_engine for channels 1-2
            // MIDI events routed to midi_processor for channels 3-4
            break;

        case MODE_FULL_MIDI:
            // Original v2 behavior — all MIDI, no patterns
            break;
    }
}
```

**midi_processor.c** — Add `MIDI_PROCESSOR_mode_2voice_poly` that only
uses channels 3-4 for note allocation. Minor change to existing poly logic.

### Clock Implementation

```c
// clock.c — TIM1 interrupt drives the step sequencer

static volatile uint8_t clock_tick = 0;
static uint16_t step_interval_ms;

void CLOCK_init(uint16_t bpm) {
    step_interval_ms = 60000 / (bpm * STEPS_PER_BEAT);
    // Configure TIM1 period = step_interval_ms * (SystemCoreClock/1000/prescaler)
    HAL_TIM_Base_Start_IT(&htim1);
}

void CLOCK_set_bpm(uint16_t bpm) {
    step_interval_ms = 60000 / (bpm * STEPS_PER_BEAT);
    // Update TIM1 ARR register
}

// Called from TIM1 interrupt handler
void CLOCK_tick_handler(void) {
    clock_tick = 1;  // Flag for main loop
}

// Called from main loop
uint8_t CLOCK_check_tick(void) {
    if (clock_tick) {
        clock_tick = 0;
        return 1;
    }
    return 0;
}
```

### Pattern Engine

```c
// pattern_engine.c — Reads patterns from config, outputs to DACs

void PATTERN_ENGINE_init(void);
void PATTERN_ENGINE_step(void);          // Advance one step, write outputs
void PATTERN_ENGINE_set_bank(uint8_t bank);
void PATTERN_ENGINE_reset(void);         // Reset to step 0
void PATTERN_ENGINE_set_transpose(int8_t semitones);

// Called each clock tick:
// 1. Read current step from active pattern
// 2. Convert melody/bass MIDI notes to DAC voltages (via note_to_voltage)
// 3. Set accent CV (on/off)
// 4. Set gate outputs (rhythm pattern + gate timing)
// 5. Advance step counter (wrap at pattern length)
```


## Implementation Priority

1. **clock.c** — Get TIM1 ticking at configurable BPM, Gate 1 toggling
2. **patterns_config.h** — Define the data structures and 2 test banks
3. **pattern_engine.c** — Read patterns, output to DACs, advance steps
4. **scale.c** — Note-to-voltage with scale quantization
5. **main.c** — Mode switch logic, integrate pattern + MIDI modes
6. **midi_processor.c** — Add 2-voice poly mode for channels 3-4
7. **MIDI CC handling** — Bank select, BPM, transpose via MIDI
8. **Remaining banks** — Fill in banks 2-7 with patterns


## Future: MIDI Control Surface

A small external MIDI controller (DIY or commercial) could provide:

- 8 buttons for bank selection (or 2 buttons for bank up/down)
- 1 knob for BPM
- 1 button for pattern reset / re-sync
- Optional: small OLED showing current bank + BPM

This controller just sends MIDI CC/Program Change messages over a standard
MIDI cable into the module's existing MIDI input. No hardware changes to the
module needed. Could even be a second small eurorack module (Arduino Nano +
a few buttons + MIDI out jack).


## Workflow for Editing Patterns

1. Open `patterns_config.h` in any text editor
2. Edit the note arrays (use MIDI note numbers: C3=48, C4=60, etc.)
3. Edit the rhythm arrays (1 = play, 0 = rest)
4. Edit the accent arrays (1 = accent, 0 = normal)
5. Compile with STM32CubeIDE (or `make` if using Makefile)
6. Flash via ST-Link / SWD header on the PCB
7. Module immediately plays updated patterns on power-on

Total turnaround: edit → compile → flash ≈ 30 seconds.


## Quick Reference: MIDI Note Numbers

```
C2=36  C#2=37  D2=38  D#2=39  E2=40  F2=41  F#2=42  G2=43  G#2=44  A2=45  A#2=46  B2=47
C3=48  C#3=49  D3=50  D#3=51  E3=52  F3=53  F#3=54  G3=55  G#3=56  A3=57  A#3=58  B3=59
C4=60  C#4=61  D4=62  D#4=63  E4=64  F4=65  F#4=66  G4=67  G#4=68  A4=69  A#4=70  B4=71
C5=72  C#5=73  D5=74  D#5=75  E5=76  F5=77  F#5=78  G5=79  G#5=80  A5=81  A#5=82  B5=83
```
