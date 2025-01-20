# Weathering (The Storm) / Wolkje

This module is a Clouds clone, originally developed by Mutable Instruments. The board is built in 2 layers, 
one for the control panel and one for the MCU. The assembly is SMD, with the exception of the jacks, pots, LED's and some capacitors.

The module is a granular audio processor, which can be used as a delay, pitch shifter, reverb, looper, etc.

The goal of having the MCU board as a independent board is to allow the user to create new firmwares and experiment with the module. 
They can replace the Audio codec & MCU with more modern versions to generate different effects. The front panel design
is different on the second side, providing a alternative look for the module.

License: CC BY-SA 3.0

Schematics used:
- Original Clouds
- TOIL's THT version of Clouds

## Information

Current Version: 2
Status: PCBs ordered
Panel: Available as Gerber file

### Control Panel
Layers: 2
HP: 18
PCB Size: 90mm x 110mm

### MCU Panel (Brain)

Layers: 4
PCB Size: 50mm x 60mm

### Inputs

### Output

## Previous versions & Issues

### Version: 1
PCB Tested: Yes

Built: Yes

Issues:
- Voltage pins for TL072 were swapped
- LM4040 had the wrong footprint (pin-out was 1->2 instead of 3>2). swapped with cheaper LM4040 SOT-23-3 component
- LED's were in the wrong configuration, swapped position