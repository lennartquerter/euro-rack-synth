# 9-Channel Stereo Mixer
(9 mono in → pan → stereo out, with a true stereo FX send/return bus)

!! This design has not been built or tested

## Current Version: 1

| Field  | status  |
|--------|---------|
| Tested | No      |
| Built  | No      |
| Width  | ~32–36 HP |

## Concept

A performance mixer: 9 mono channels, each with Level, Pan, and a post-fader
FX Send. The pan pot is **dual-gang**, so every channel lands at the same L/R
position on the main bus *and* the send bus — the FX hears the same stereo
image you do. The stereo return comes back through a **wet VCA**: one Wet
knob plus a CV input, so an LFO or envelope can duck or swell the effect over
a track. DC-coupled, so it mixes CV as happily as audio.

## Features

- 9 mono channels: Level, dual-gang Pan (~−6 dB centre law, full separation
  at the ends), post-fader Send, mute toggle
- True stereo FX bus: send follows pan; FX SEND L/R and FX RETURN L/R jacks,
  RETURN R normalled to L so mono effects land on both sides
- Wet VCA (LM13700) on the return: Wet knob + WET CV jack (attenuated) —
  voltage-controlled FX amount; patch SEND→RETURN for controlled feedback
- Dual-gang master Volume and Send Level, both truly off fully CCW; unity
  per channel at full Volume, hard-panned
- Clip LED: window comparators on both outputs, ±8.2 V (~2 dB before the
  rails), peak-stretched so single spikes are visible
- Bus expansion header: MAIN_L/R + SEND_L/R + GND, for a future
  channel-expander board (no bus amps needed on the expander)
- Headroom-scaled summing (×0.47 summers, recovered by the masters) per the
  drums handoff §4.5

## Inputs

- 9× channel in, mono (3.5 mm)
- FX RETURN L / R (3.5 mm, R normalled to L)
- WET CV (3.5 mm, with attenuator)
- Level ×9, Send ×9, Pan ×9 (knobs) · Mute ×9 (toggles)
- Volume, Send Level, Wet, WET CV amount (knobs)

## Outputs

- OUT L / R (3.5 mm)
- FX SEND L / R (3.5 mm)
- Clip LED

## Electronics

3× TL074 + 1× LM13700, ±12 V. Channels are fully passive (pot → 100 kΩ →
pan gang → virtual-ground bus); all gain lives in four summer+master pairs.
Dry and wet paths each see two inversions, so they always sum in phase.

## Build Guide

Step-by-step schematic guide: [SCHEMATIC.md](./SCHEMATIC.md)

- Potentiometers: Alpha (D-shaft) 9 mm from THONK (https://www.thonk.co.uk/shop/alpha-9mm-pots-dshaft/)
- Jacks: PJ398SM from THONK (https://www.thonk.co.uk/shop/thonkiconn/) — FX RETURN R needs the switched contact
- Switches: mini toggles from THONK (https://www.thonk.co.uk/shop/mini-toggle-switches/)
- Trimmers: RM-065 (wet-VCA bleed nulls)
