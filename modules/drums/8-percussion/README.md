# Percussion — Module 4 ("Bazaar")

**Status: redesign brief (fresh design — new folder).**

Voices on this board: **Cowbell · Shaker/Maracas · Claves/Rimshot.**
Style: **808**. These are the "extra cool" percussion that make techno
patterns breathe. SMD except pots, jacks, film/electrolytic caps.

---

## Concept

Three small, classic 808 circuits, each a different sound-generation trick,
on one board:

- **Cowbell** = two square oscillators (≈**540 Hz** + **800 Hz**) mixed →
  band-pass → fast-attack/medium-decay VCA. The most recognisable 808 sound.
- **Shaker / Maracas** = white noise → high-pass → very short VCA decay. The
  "tsst". Retriggerable for shaker rolls.
- **Claves / Rimshot** = a **pinged resonator** (twin-T) ringing briefly at a
  high pitch (~**2.5 kHz** claves, ~**1.7 kHz** rimshot).

> The cowbell's two oscillators are literally oscillators 5 & 6 of the 808
> metal core in [`../3-hi-hat/README.md`](../3-hi-hat/README.md). If you want
> them to match the hats exactly you can tap that board; otherwise give this
> module its own two 40106 oscillators (simpler, self-contained — that's the
> default below).

### Signal flow

```
540Hz ┐
800Hz ┴► sum ─► BAND-PASS ─► VCA ─[fast atk / med decay]─► Cowbell out

NOISE ─► HIGH-PASS ─► VCA ─[very short decay]──────────────► Shaker out
                                ▲ retrigger for rolls

ping ─► TWIN-T RESONATOR (~2.5/1.7 kHz) ─► VCA ─[short]─────► Claves/Rim out
```

---

## Circuit blocks

**Cowbell**
- Two **CD40106B** Schmitt oscillators at ~540 Hz and ~800 Hz (ratio ≈1.48 —
  the inharmonic "clank"). **THT film** timing caps so it stays in tune.
- Sum → multiple-feedback **band-pass** (TL074) centred ~2–2.6 kHz.
- **VCA** (LM13700) with fast attack, medium decay (~150–400 ms). Decay pot.

**Shaker / Maracas**
- Noise from the **[Noise Board](../../../helpers/noise-board/README.md)
  piggyback** via a `Noise-In` header (no generator on this board) →
  **high-pass** (~5 kHz) → VCA with **very short** decay (~30–80 ms).
- Feed the VCA from the **trigger repeater** for shaker rolls / closed
  patterns.

**Claves / Rimshot**
- A **twin-T resonator** pinged by the trigger (same family as the toms,
  just high and short). Switch/CV the resonant frequency for claves (~2.5
  kHz) vs rimshot (~1.7 kHz). Rimshot adds a touch of noise to the ping.
- VCA + short envelope.

---

## Controls & I/O

| Voice | Controls |
|-------|----------|
| Cowbell | Tone (band-pass center) · Decay · Level |
| Shaker | Tone (HPF) · Decay · Level |
| Claves/Rim | Pitch · Claves/Rim switch · Level |

**Jacks:** Cowbell trig · Shaker trig · Claves trig · Accent in · 3 voice
outs · Mix out.

### CV inputs

Per the [CV control standard](../README.md#cv-control-modulation-standard).

| CV input | Atten. | What it does | How |
|----------|--------|--------------|-----|
| **Cowbell Tone** ★ | knob | Sweeps the band-pass — evolving clank | CV → cowbell BP OTA control current |
| **Cowbell Decay** | plain jack | Ring length | CV → cowbell **VC-envelope** decay |
| **Shaker Decay** | knob | Tight tick ↔ longer shh | CV → shaker VC-envelope decay |
| **Claves Pitch** | plain jack | Resonator pitch (claves↔rim region) | CV → twin-T tuning node |
| **Accent** | (existing) | Velocity / dynamics | CV → VCA gains |

A slow LFO on **Cowbell Tone** plus shaker rolls off the trigger repeater is
a lot of evolving top-end percussion from one small board.

**Panel / size:** target **10 HP**.

---

## Core BOM (SMD unless noted)

| Part | Pkg | Use |
|------|-----|-----|
| CD40106B | SOIC-14 | cowbell oscillators, ping one-shots, shaker env |
| 2× TL074 | SOIC-14 | band-pass, high-pass, resonator, mixing |
| 1–2× LM13700 | SOIC-16 | 3 VCA channels |
| MMBT3904/3906 | SOT-23 | ping injectors, envelopes |
| 1×3 + 1×2 sockets | THT | **Noise Board headers** (power up / noise in) |
| 1N4148W / BAT54 | SOD-123 | envelope rectifiers |
| Cowbell + twin-T caps | **THT film** | tuning-critical |
| Envelope caps | THT film/elec | decays |
| Pots, 3.5 mm jacks | THT | panel |

---

## Build order

Cowbell first (most iconic, two oscillators are easy to verify on a scope),
then shaker (trivial once noise is good), then the pinged claves/rimshot
(reuses the tom resonator know-how).
