# Midas: MIDI-to-CV Converter

Midas is a versatile MIDI-to-CV (Control Voltage) converter designed for the euro rack. 
It bridges the gap between digital MIDI controllers and analog synthesizers, 
allowing seamless integration of modern MIDI equipment with classic or modular synthesizers.

## Features

- 4 CV outputs for pitch control
- 4 Gate outputs for note triggering
- Velocity CV output for expressive playing
- Modulation CV output for additional control
- 3-position mode switch
- High-resolution 12-bit DAC for precise voltage control
- Low latency for responsive playing
- DIN MIDI inputs

## Modes of Operation

Midas offers three distinct modes of operation, selectable via a 3-position switch:

1. **Channel Mode**: Each CV/Gate pair responds to a different MIDI channel, allowing multi-timbral control of up to four monophonic synthesizers.

2. **Poly Mode**: All four CV/Gate pairs work together to provide four-voice polyphony for a single synthesizer or multiple synthesizer voices.

3. **Sequence Mode**: CV/Gate pairs are assigned to incoming MIDI notes in a round-robin fashion, ideal for creating complex sequences or polyrhythmic patterns.

## Technical Specifications

- MIDI Input: 5-pin DIN
- CV Outputs: 0-8V range, 12-bit resolution
- Gate Outputs: 0-8V
- Power: Eurorack +-12V
- Consumption +0.10A/-0.05A 
- Dimensions: 10hp

## Programming
You can use the STM32Programmer to upload the file [midi-2-cv-v2.elf](dist/midi-2-cv-v2.elf). 

## Hacks and special considerations

### system_stm32f4xx.c

`SCB->VTOR = FLASH_BASE;  // <---------------- WORKAROUND!`
is needed to enable the systick handler, for some reason this does not work without.

[Forum link](https://community.st.com/t5/stm32cubemx-mcus/systick-handler-not-called-stm32g0b1/td-p/204749/page/2)


### REV 2 learnings:
- I need external pull up resistors for each I2C connection
- Ensure that the transistors for LED's are set up correct before printing.