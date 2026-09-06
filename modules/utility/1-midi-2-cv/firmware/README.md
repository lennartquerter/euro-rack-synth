# Midas: MIDI-to-CV Converter

Midas is a versatile MIDI-to-CV (Control Voltage) converter designed for the euro rack. 
It bridges the gap between digital MIDI controllers and analog synthesizers, 
allowing seamless integration of modern MIDI equipment with classic or modular synthesizers.

## Features

- 4 CV outputs for pitch control
- 4 Gate outputs for note triggering
- Velocity CV output for expressive playing
- Modulation CV output driven by the mod wheel (CC1)
- Pitch bend applied to the pitch CV, +/-2 semitones
- 3-position mode switch
- High-resolution 12-bit DAC for precise voltage control
- Low latency for responsive playing
- DIN MIDI inputs

## Modes of Operation

Midas offers three distinct modes of operation, selectable via a 3-position switch:

1. **Channel Mode**: Each CV/Gate pair responds to a different MIDI channel, allowing multi-timbral control of up to four monophonic synthesizers.

2. **Poly Mode**: All four CV/Gate pairs work together to provide four-voice polyphony for a single synthesizer or multiple synthesizer voices.

3. **Sequence Mode**: CV/Gate pairs are assigned to incoming MIDI notes in a round-robin fashion, ideal for creating complex sequences or polyrhythmic patterns.

Channel mode listens on MIDI channels 1-4, one per output. Poly and Sequence mode both listen on
MIDI channel 1 only (`MIDI_INPUT_CHANNEL`) and drive all four outputs from it.

### Jack detection

Each gate jack has a normalling contact wired to a detect input (PA1, PA2, PA4, PA5 -- the
`GATE_n_CALLBACK` nets). The contact shorts to the tip while the jack is empty, so the detect line
follows the gate output when nothing is plugged in and is left floating high by the board's 100K
pull-up once a plug lifts it. **High therefore means a cable is present, and the reading is only
valid with the gates driven low** -- `detect_available_channels()` drives them low, settles, then
samples.

Poly and Sequence mode allocate voices only to patched outputs, so a two-cable patch gives
two-voice polyphony and a two-step sequence instead of sending notes to empty jacks. Channel mode
is unaffected: its output mapping is fixed by MIDI channel. If nothing is detected the firmware
falls back to using all four, which is also what happens on a board without the detect hardware.

The scan runs at startup and on every mode change, so a cable patched while running is picked up by
flicking the mode switch.

Poly mode steals the oldest sounding voice when all patched voices are busy, rather than dropping
the note.

## Technical Specifications

- MIDI Input: 5-pin DIN
- Pitch CV Outputs: 1V/oct, 0-8.166V (MIDI notes C0-D8), 12-bit resolution
- Velocity/Mod CV Outputs: 0-8V range, 12-bit resolution
- Gate Outputs: 0-8V
- Power: Eurorack +-12V
- Consumption +0.10A/-0.05A 
- Dimensions: 10hp

## Programming
You can use the STM32Programmer to upload the file [midi-2-cv-v2.elf](../compiled/midi-2-cv-v2.elf). 

## Output scaling and calibration

The MCP4728 is written with Fast Write, which carries no Vref/gain bits, so the DACs run on their
EEPROM-stored configuration -- the factory default of an internal 2.048V reference at gain 1. That
is 0.5mV per code, and the 4x output stage makes it **500 DAC codes per volt at the jack**.

The pitch CV is therefore *not* a full-scale mapping of the MIDI range. Volts per octave is a fixed
physical constant, so one semitone is a fixed `DAC_CODES_PER_OUTPUT_VOLT / 12` = 41.667 codes
(83.33mV at the jack). `CV_LOWEST_NOTE` (C0, MIDI 12) sits at 0V, which puts C4 (MIDI 60) at exactly
4.000V. Notes below C0 clamp to 0V; the highest note that still tracks is D8 (MIDI 110, 8.166V),
above which the DAC clamps at 8.19V. Every octave lands exactly on a volt boundary, and the integer
rounding error is at most 0.8 cents.

Pitch bend is added to the pitch CV as a signed code offset, so a bend on a held note glides that
note rather than waiting for the next note-on. `DEFAULT_PITCH_BEND_SEMITONES` sets the range as a
musical interval (2 => +/-83 codes => +/-0.167V), and the mod wheel (CC1) drives the MOD output
across its full 0-8V span. Both follow the routing of the current mode: per MIDI channel in Channel
mode, applied to all four outputs in Poly and Sequence mode.

The DACs are configured at boot by `MCP4728_Init`. It writes Vref and gain to the volatile input
registers on every start, then reads back the EEPROM and only rewrites it when the stored
configuration differs -- so the chips also come up correct before firmware runs, without wearing the
EEPROM on every boot. A failure there is non-fatal by design; the module keeps running.

To calibrate against real hardware, play a note an octave above another, measure the difference, and
trim `cv_dac_codes_per_volt` -- it is the only constant in the pitch path. `DEFAULT_DAC_MAX_VALUE`
(4000 codes = 8.00V) is unrelated to pitch; it is the full-scale span used by the velocity and
modulation outputs, which *are* proportional 0-127 controller values.

## Hacks and special considerations

### system_stm32f4xx.c

`SCB->VTOR = FLASH_BASE;  // <---------------- WORKAROUND!`
is needed to enable the systick handler, for some reason this does not work without.

[Forum link](https://community.st.com/t5/stm32cubemx-mcus/systick-handler-not-called-stm32g0b1/td-p/204749/page/2)