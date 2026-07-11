# Cymbals & Hats — Module 1 ("Coliseum")

**Status: redesign brief (not built).** This README covers the *whole*
metal-voice module — hats *and* the cymbal voicing (the former `4-cymbals`
brief was folded in here); all voices live on **one board** and share the
metal core below.

Voices on this board: **Closed Hat · Open Hat · Crash/Ride Cymbal.**
Style: **808** (the only way to get analog metals — 909 hats/cymbals are
samples). All SMD except pots, jacks, and film/electrolytic caps.

---

## Concept

An 808 hi-hat and cymbal are not noise — they are **six square-wave
oscillators at inharmonic frequencies**, summed into a clangy, metallic
"chord", then high-pass filtered and shaped by an envelope/VCA. Closed hat,
open hat and cymbal are the **same source** with different filters and
different decay times.

Building the six oscillators **once** and fanning them out to three voice
chains is why this is one module, not three.

### Signal flow

```
                        ┌─► HPF (sharp) ─► VCA ─[CH env]─► Closed Hat out
6-osc METAL CORE ─► sum ─┼─► HPF (sharp) ─► VCA ─[OH env]─► Open Hat out
(square waves)           └─► BPF/HPF ─────► VCA ─[Cym env]─► Cymbal out
                                                              │
   CH trig ─► choke ─► mutes OH                          all ─┴─► Mix out
```

---

## The metal core (shared)

Six Schmitt-trigger relaxation oscillators, each = one inverter of a
**CD40106B** with an RC. Two CD40106B (12 gates) give you the 6 oscillators
plus spare gates for buffering/clocking. Classic 808 frequencies:

| Osc | Freq (Hz) | timing |
|-----|-----------|--------|
| 1   | 205.3     | film cap + resistor on a 40106 gate |
| 2   | 304.4     | "" |
| 3   | 369.6     | "" |
| 4   | 522.7     | "" |
| 5   | 540.0     | "" |
| 6   | 800.0     | "" |

- Use **film caps** (THT) for the timing caps — ceramic tempco/microphonics
  will detune the metal "chord" and is a classic reason a clone sounds
  wrong.
- Sum the six squares through equal resistors (~100 k) into a TL074
  inverting mixer. That summed node is the metal bus feeding all three
  voices.
- Optional **Tune** pot: pull all six RC references off one trimmed voltage
  so you can shift the whole metal pitch a little (techno detune trick).

> These same six oscillators are what an 808 taps for the **cowbell**
> (oscillators 5 & 6, ~540/800 Hz). If you want the percussion module to
> match perfectly, mirror this core there — see `8-percussion`.

---

## Per-voice chains

**Closed Hat / Open Hat** (share one LM13700 = two VCA channels):

1. **HPF** on the metal bus — sharp high-pass (~6–9 kHz corner, 2-pole) so
   you hear *tick/sizzle*, not the raw square buzz. This is the #1 thing to
   get right: too gentle and it sounds like a fuzzy oscillator, not a hat.
2. **VCA** (LM13700 OTA) controlled by a **percussive envelope** (fast
   attack, exponential decay).
   - CH decay short (~40–120 ms), set by small envelope cap.
   - OH decay long (~300 ms–1 s), set by larger cap + decay pot.
3. **Output HPF** after the VCA tightens the transient further.

**Choke:** a CH trigger should **mute the OH VCA** (real hi-hats can't ring
open and closed at once). Implement as a short mute pulse from the CH
trigger pulling the OH envelope/VCA down — this "choke" is signature techno.

**Cymbal:** same metal bus, but band-pass-ish post-filter with more low-mid
left in, and a **two-stage decay** envelope (fast initial + long tail) for
crash vs ride character. Full detail in `../4-cymbals/README.md`.

**Optional "air":** the classic 808 metal is pure oscillators, but a little
white noise blended into the bus adds modern sizzle. If you want it, fit an
unpopulated-by-default `Noise-In` header to the shared
[Noise Board](../../../helpers/noise-board/README.md). Not needed for the
authentic sound.

---

## Controls & I/O

**Front panel (THT pots/jacks):**

| Control | Function |
|---------|----------|
| HH Tone | post-filter HPF cutoff (shared CH/OH) |
| CH Decay | short envelope decay |
| OH Decay | long envelope decay |
| Cymbal Decay | two-stage tail |
| Cymbal Tone | cymbal band-pass center |
| Tune (opt.) | global metal-core pitch |
| CH / OH / Cym Level | per-voice |

**Jacks:** CH trig in · OH trig in · Cymbal trig in · Accent in (raises VCA
gain) · CH out · OH out · Cymbal out · Mix out.

### CV inputs — the character (LFO movement)

This is the busiest board, so it gets the richest modulation. The hero input
is **Open/Close**: patch an LFO and the hats breathe from tight closed ticks
to long open washes across the track. Built per the
[CV control standard](../README.md#cv-control-modulation-standard).

| CV input | Atten. | What it does | How |
|----------|--------|--------------|-----|
| **Open/Close** ★ | attenuverter | Morphs CH↔OH: lengthens OH decay and eases the choke | CV → OH **VC-envelope** decay current (+ scales choke depth) |
| **OH Decay** | knob | Open-hat length on its own | CV → OH envelope decay current |
| **CH Decay** | knob | Closed-hat tightness | CV → CH envelope decay current |
| **HH Tone** | knob | Sweeps hat brightness | CV → HPF OTA control current |
| **Cymbal Decay** | knob | Wash length | CV → cymbal tail env current |
| **Cymbal Tone** | plain jack | Ride↔crash color | CV → cymbal BP control current |
| **Accent** | (existing) | Velocity/dynamics | CV → all VCA gain currents |

Normal each knob to a usable base value so the voice still sounds with
nothing patched. The **Open/Close** and the two **Decay** inputs use the
**VC percussive envelope** (LM13700 discharging the env cap) — that's the
one circuit change versus a fixed-decay design, and it's where the life
comes from.

> Note on the old sample-and-hold sheet: with proper CV decay inputs you may
> not need the S&H "random hat" trick — an external LFO/S&H patched into
> Open/Close does the same job and more, freeing panel space.

**Panel / size:** target **12 HP** — three trig + accent + ~6 CV jacks + the
attenuators push it to the top of your range; this is the widest board in
the set.

---

## Why the earlier sheets didn't land (things to fix)

Your `3-hi-hat` sheets already had the right ingredients (40106 oscillator
bank, LM13700 filter, sample-and-hold, retrigger). Likely culprits to
verify on the rebuild:

- **HPF not aggressive enough** → sounds like buzzy noise, not a hat. Go
  steeper and higher.
- **Oscillator timing caps ceramic** → detuned/temperature-drifting metal
  chord. Use film.
- **VCA control feedthrough** → audible "thump" on each trig. Trim the
  LM13700 and AC-couple the output.
- **No OH choke** → open and closed hats overlap unnaturally.

---

## Core BOM (SMD unless noted)

| Part | Pkg | Use |
|------|-----|-----|
| 2× CD40106B | SOIC-14 | 6 metal oscillators + buffers |
| 2–3× TL074 | SOIC-14 | summing, HPF/BPF, output buffers, **CV attenuators** |
| 3× LM13700 | SOIC-16 | 3 VCA + filter OTAs + **VC-envelope decay OTAs** |
| MMBT3904/3906 | SOT-23 | trigger conditioning, choke, envelope |
| 1N4148W / BAT54 | SOD-123 | envelope rectifiers, choke gating |
| 0805 R/C | 0805 | general |
| Metal-core timing caps | **THT film** | the six oscillator caps |
| Pots, 3.5 mm jacks | THT | panel |
