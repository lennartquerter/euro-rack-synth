# Ricochet (Stereo Delay)

This module is a stereo delay built around the PT2399 chip. It has two delay lines that can be mixed together.
The delay time, feedback and dry/wet mix can be set for both lines independently. The panning allows the user to 
either output the delay signal to the A or B channel or mix it in the center, resulting in a stereo effect.

Input A is always routing to Input B if there is no cable plugged into Input B. 

## Information

Current Version: 2
Status: PCBs ordered
Panel: Available as Gerber file
Layers: 4
HP: 10
PCB Size: 60mm x 110mm

See [Bill Of Material](./ibom.html) for a full list of material and where to solder the components.

### Inputs

- Delay Input A
- Delay Input B
- Delay Time [A+B]
- Delay Feedback [A+B]
- Delay Dry/Wet (Mix) [A+B]
- Panning A/B [A+B]

### Output

- Delay Output A
- Delay Output B


## Previous versions & Issues

### Version: 1
PCB Tested: Yes

Built: Yes

Issues:
- L7805 was too small to handle the current needed for 2 x PT2399 chips
- PT2399 chips were too big
- Schematic was based on datasheet, but didn't deliver a great sounding delay