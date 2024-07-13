# Scribble
(Envelope Generator)

!! The current version has not been built or tested


## Current Version: 2

| Field  | status  |
|--------|---------|
| Tested | No      |
| Built  | No      |
| Length | 111 mm  |
| Width  | 40.3 mm |

## Inputs

- Trigger (3.5mm)
- Attack (knob)
- Sustain (knob)
- Release (knob)
- Loop Switch (Switch)


## Output

- Envelope (3.5mm)
- Inverted Envelope (3.5mm)


## Build Guide

Schematics can be [found here](./img/scribble_schematic_a3.pdf)

- Potentiometers: I use the Alpha (D-shaft) 9mm fom THONKICONN (https://www.thonk.co.uk/shop/alpha-9mm-pots-dshaft/)
- Jacks: I use the PJ398SM from THONKICONN (https://www.thonk.co.uk/shop/thonkiconn/)
- Switch: I use the Mini toggle switch from THONKICONN (https://www.thonk.co.uk/shop/mini-toggle-switches/)
- All mounting holes are M3 (3.2mm), and pick an offset that you feel good with

### Bill Of Material

See [Bill Of Material](./BOM.csv) for a full list of material and where to solder the components

### Soldering

In general: use the Bill Of Material and solder each component. Start from the lower components and go to the higher ones.


## Images (3D)

![Front Panel](./img/scribble_3D_front.png)

![Back Panel](./img/scribble_3D_back.png)


## Previous versions & Issues

### Version: 1
PCB Tested: Yes

Built: Yes

Issues:
- Schematic was wrong and had inverted inputs on the TL074's
- 2mm sockets are a bit too small and required layering to make it high enough
- Capacitors were on the front, making the height of the bottom board too high