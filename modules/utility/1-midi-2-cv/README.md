# Midas

*!! Please read the Issues at the end of the file before starting the build !!*

This module is one of my first designs that include a MCU. I first designed the software around a STM32 Blackpill board,
which I hooked up to some proto-type boards. The software was written in C and used the STM32 HAL library. I used the
MCP4728 DAC to output the CV signals. Each DAC has 4 output channels and is driven by an I2C bus. The DAC 12-bit ranging
from 0 to 2V. I use an OpAmp to amplify the signal to 0-8V. 

The module has 4 gate outputs, 4 CV outputs, 4 Velocity outputs, and 4 Mod outputs. It contains a switch for the following 3 modes:

- Monophonic (4 midi channels, independently working)
- Polyphonic (1 midi channel, 4 voices)
- Sequence (each note triggers the next channel)

## Information

Current Version: 2
Status: Completed
Panel: Available as Gerber file
Layers: 4
HP: 10
PCB Size: 60mm x 110mm

### Inputs

- Midi input
- Mode switch

### Output

- 4 CV outputs
- 4 Gate outputs
- 4 Velocity outputs
- 4 Mod outputs

## Previous versions & Issues


### Version: 2
PCB Tested: Yes

Built: Yes

Issues:
- LED transistors were reversed. You can alter the board by soldering them upside-down
- MIDI input pin 2 and 4 are inverted. You need to cut a trace & rewire them
- I2C pull-up resistors are missing on the schematic. This is solved by setting the internal pull-up resistors in the software, but ideally they should be set on the board.

### Version: 1
PCB Tested: Yes

Built: Yes

If you have a Version 1 board: You can not make it work.

Issues:
- STM32F411 could not find the crystal on the board, I think this was due to bad ground. 
- The DAC was not working, No connection could be made, this might have been due to the floating SET pin.
- The original optocoupler was a 6N137, which was not working. I replaced it with a 6N137-L because this one allows 3V3 input.

