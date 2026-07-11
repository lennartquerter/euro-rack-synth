# Clap voice — part of Module 2 (Snare / Clap)

**Status: redesign brief.** Supersedes the built-but-not-working **Encore**
board. The clap is a voice of the combined **Snare / Clap** module — it
shares the noise input and trigger conditioning with the snare; the full
build is in [`./SCHEMATIC.md`](./SCHEMATIC.md). Build on one board.

Style: classic analog clap (808/909 family). SMD except pots, jacks,
film/electrolytic caps.

---

## Concept — it's all in the envelope

A hand-clap is white noise through a fixed band-pass, shaped by a very
specific **multi-pulse envelope**: three (or four) fast bursts a few
milliseconds apart — the "cl-cl-clap" of several hands — riding on top of
**one longer decaying tail** that gives the room/reverb wash.

Get the envelope right and cheap noise sounds like a clap. Get it wrong and
it's a single tick or a buzz — the most common clap-clone failure.

### Signal flow

```
shared NOISE ─► BAND-PASS (~1 kHz, broad) ─► VCA ─► output ─► Clap out
                                              ▲
                                              │  CV =
   Clap trig ─► TRIGGER REPEATER ─► [ 3 fast pulses ] + [ long tail ]
               (your 7-step repeater)     summed into one envelope
```

---

## Circuit blocks

- **Multi-pulse generator:** use your **trigger repeater (7-step)** block to
  fire **3 closely spaced pulses** (~8–12 ms apart), each fast attack /
  fast decay. This is the clap "stutter". Spacing and count are the whole
  character — make at least the spacing tweakable.
- **Tail:** sum a single **longer decaying envelope** (~150–300 ms) onto the
  same VCA CV so the last burst washes out. 3 pulses **+** tail → one
  composite envelope.
- **Band-pass:** noise → multiple-feedback band-pass (TL074) or LM13700 BP,
  centred ~**1 kHz**, moderate Q. The **Tone** pot moves the center; the 808
  vs 909 character difference is mostly this filter + tail length.
- **VCA:** one LM13700 channel (the third on the Snare/Clap board).
- **Accent:** lifts VCA gain.

---

## Controls & I/O

| Control      | Function                                 |
|--------------|------------------------------------------|
| Tone         | band-pass center                         |
| Spread       | pulse spacing (tightness of the stutter) |
| Tail / Decay | length of the reverb-y wash              |
| Level        | output                                   |

**Jacks:** Clap trig in · Accent in · Clap out (→ module Mix out). Shares
noise, trigger conditioning and ±12 V with the snare.

### CV inputs

Per the [CV control standard](../README.md#cv-control-modulation-standard).
On the shared Snare/Clap panel:

| CV input   | Atten.     | What it does                      | How                                     |
|------------|------------|-----------------------------------|-----------------------------------------|
| **Tail** ★ | knob       | LFO opens the reverb-y wash       | CV → tail **VC-envelope** decay current |
| **Tone**   | plain jack | Band-pass center                  | CV → band-pass OTA control current      |
| **Spread** | plain jack | Pulse spacing (stutter tightness) | CV → repeater timing reference          |
| **Accent** | (shared)   | Louder clap                       | CV → clap VCA gain                      |

**Spread** under slow LFO is a great evolving-clap trick — the stutter
tightens and loosens over the loop.

---

## Why the Encore didn't work (fix list)

The Encore schematic has the right shape (TL07x band-pass, BC8xx noise,
Schottky-rectified envelopes) but is flagged not-working. Check, in order:

1. **Noise amplitude** — scope the shared noise node first; if it's weak,
   the clap is dead regardless of the rest. (Shared rebuild fix.)
2. **Multi-pulse actually pulsing** — verify the repeater produces 3 spaced
   pulses, not one. Tune spacing caps (THT film/elec).
3. **Composite envelope reaches the VCA** — confirm pulses **and** tail are
   summed into the CV, not fighting each other.
4. **Band-pass center** — if it's too high/low it sounds like hiss, not a
   clap; aim ~1 kHz.
5. **VCA feedthrough** — trim, AC-couple the output.

Full net-by-net design, values and BOM for the whole Snare/Clap module:
[`./SCHEMATIC.md`](./SCHEMATIC.md).
