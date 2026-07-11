# Toms / Subkick — Module 3

**Status: redesign brief (fresh design — folder was empty).**

Voices on this board: **3 tunable toms (Lo / Mid / Hi) + Subkick mode.**
Style: **808** pinged-resonator toms, with an optional **Syndrum-style
pitch sweep** for electro/techno toms. SMD except pots, jacks,
film/electrolytic caps.

---

## Concept — a tom is a pinged resonator

An 808 tom is a **bridged-T (twin-T) resonator "pinged" by a trigger
pulse**. The pulse kicks the network and it rings out as a decaying sine at
its resonant frequency — exactly like the body of your Muay Thai kick, just
tuned higher and retriggered. Three of these at three tunings give Lo/Mid/Hi
toms. The **same circuit tuned very low with a long decay is the subkick.**

Two flavours, both useful for techno — offer both:

- **Acoustic-ish tom (808):** bridged-T rings → natural "donk". Tune = pitch,
  Decay = ring length.
- **Electro tom (Syndrum):** a VCO with a **fast downward pitch envelope** —
  the "pew/dewww" drop. This is your **Impact Tone Generator** block
  (PAIA Syndrum) + **CD4046 VCO**. Great as the subkick "drop" and for
  electro toms.

### Signal flow (per tom)

```
Tom trig ─► conditioner ─► PING pulse ─► BRIDGED-T RESONATOR ─► VCA ─► out
                                            ▲          ▲          ▲
                                          Tune       Decay     [percussive
                                         (pitch)   (ring Q)      envelope]
                              (optional) PITCH-SWEEP CV ─┘  ← Syndrum drop
```

---

## Circuit blocks

- **Resonator (×3):** bridged-T network around a transistor/op-amp set just
  below self-oscillation, so a pulse makes it ring and decay. **Tune** =
  the bridged-T resistance (pitch); **Decay** = feedback/Q (ring length).
  Use **THT film caps** in the twin-T — this is a tuned circuit and ceramic
  drift will detune the tom.
- **Ping/excitation:** trigger conditioner → short pulse through a transistor
  into the resonator. Accent makes a louder, slightly longer ping.
- **Pitch sweep (optional per tom, default on subkick):** apply a fast
  decaying envelope to the resonator/VCO tuning so the pitch starts high and
  drops — the electro-tom and 909-ish "snap". This is your Impact Tone
  Generator / CD4046 block.
- **VCA + envelope:** LM13700 channel + percussive envelope per voice (or let
  the resonator's natural decay do the work and use the VCA just to gate out
  the tail cleanly).
- **Subkick mode:** one channel switchable to subkick = **Tune ~45–60 Hz**,
  **long decay**, **pitch-sweep on**, optional gentle saturation (diode
  clipper) for sub weight. Effectively "tom 0".

Suggested tunings: Hi ~300–400 Hz, Mid ~180–250 Hz, Lo ~100–150 Hz,
Subkick ~45–60 Hz — all shiftable by the Tune pots.

---

## Controls & I/O

| Control (×3 toms) | Function |
|-------------------|----------|
| Tune | resonant pitch |
| Decay | ring length |
| Level | per-voice |
| Pitch-sweep amt (global or per tom) | Syndrum drop depth |
| Subkick switch (on Lo channel) | low tune + long decay + sweep |

**Jacks:** Lo/Mid/Hi trig in (×3) · Subkick trig (or shares Lo) · Accent in ·
Lo/Mid/Hi out · Mix out.

### CV inputs

Per the [CV control standard](../README.md#cv-control-modulation-standard).
**Tune CV** is the star on toms — an LFO/sequencer makes melodic, evolving
tom lines (and a slow LFO on the subkick Tune is a classic rumbling
sub-wobble).

| CV input | Atten. | What it does | How |
|----------|--------|--------------|-----|
| **Tune** ★ | attenuverter | Pitches the resonator(s) | CV → bridged-T / VCO tuning node |
| **Decay** | knob | Ring length | CV → resonator Q / VC-envelope decay |
| **Pitch-Sweep** | knob | Depth of the Syndrum drop | CV → sweep-envelope amount |
| **Accent** | (existing) | Velocity / harder ping | CV → ping level + VCA gain |

**Tune routing:** one Tune CV jack **normalled to all three toms** keeps HP
down; break the normal with the per-voice trig jacks if you want independent
pitch. If you can spare the width, a **Tune CV per tom** is worth it for
melodic patterns — that's the main reason this board can grow toward 12 HP.

**Panel / size:** **10–12 HP** depending on shared vs per-voice Tune CV.

---

## Relationship to your kicks

This is the **same resonant-body idea** as your working **Muay Thai** kick
and the **Basari** clone — reuse what already works there for the resonator
and ping stages, just add Tune/Decay range and the pitch sweep. If the
bridged-T proves fussy across 3 tunings, the CD4046-VCO + pitch-envelope
(Syndrum) path is the more repeatable, build-friendly fallback and still
very techno.

---

## Core BOM (SMD unless noted)

| Part | Pkg | Use |
|------|-----|-----|
| 2× TL074 | SOIC-14 | resonators, mixing, buffers |
| 1–2× LM13700 | SOIC-16 | VCAs (3–4 channels) |
| CD4046B (opt.) | SOIC-16 | electro-tom / subkick VCO |
| CD40106B | SOIC-14 | ping one-shots, pitch-sweep envelope |
| MMBT3904/3906 | SOT-23 | ping injectors, envelopes |
| 1N4148W / BAT54 | SOD-123 | envelope rectifiers, soft clip |
| Twin-T timing caps | **THT film** | the resonators (tuning-critical) |
| Envelope caps | THT film/elec | decays |
| Pots, 3.5 mm jacks | THT | panel |
