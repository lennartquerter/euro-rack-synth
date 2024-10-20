# Random Harmonic Sequence Generator

### Features (To implement)

- internal BPM clock from 60 to 240
  - Clock output (???)
- external input for clock signal
- Random generated within a scale
- Octave settings (+-1,2,...)
- Probability for Gate length
- 2 sequences patterns
- Per channel:
  - Gate out
  - 1v/oct out [0-8V]
  - Accent output [0-4V]
  - 8 to 64 step length
- 2 x 8 LED sequence steps
- 3 x control status LED [RUN/HALT/EDIT]
- 4 x Sequence bank LED


### Technical information

Based on the STM32F401CCU8

#### ICs

- 3x HC595
- 2x MCP4822


- 1x 5V regulator (ICs)
- 1x 3.3V regulator (STM32 power)
- 1x 3.3V regulator (encoder)