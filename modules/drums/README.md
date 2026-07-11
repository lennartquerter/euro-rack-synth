# Drums — analog percussion family

A coherent set of **fully analog** drum voices for techno, based on the
classic Roland circuits (808 for metals/percussion, 909 for snare/clap).
No sampling, no DSP — every voice is an oscillator/noise source shaped by
analog filters, envelopes and VCAs.

This README is the index: it explains **what goes together**, the
**shared building blocks** every voice reuses, and the **parts/BOM
philosophy** for the rebuild. Each module then has its own README with a
concept + circuit-block brief.

---

## Why these are being rebuilt

The existing attempts (Encore clap/snare, the hi-hat and cymbal sheets)
share the same root problems, so the rebuild fixes them once, centrally:

- **Noise source too quiet / not white enough.** A single reverse-biased
  BJT junction gives a tiny (~mV) signal that drifts with temperature and
  needs a lot of clean gain. Every metal/snare/clap voice depends on it, so
  if it is weak the whole module sounds dead. → Solved once with the shared
  [Noise Board](../../helpers/noise-board/README.md) piggyback (a panel-less
  daughterboard that plugs onto the back of any host that needs noise).
- **Trigger conditioning inconsistent.** Voices retrigger unreliably or
  double-trigger from the same gate. → One shared trigger conditioner.
- **VCA control feedthrough / "thump".** Envelope bleeding into the audio
  path through the VCA. → Standardise on a clean OTA or current-steering VCA
  with trim.

These three are exactly your `building-blocks` sheets. The rebuild promotes
them to a properly characterised shared library and builds every voice on
top.

---

## Recommended grouping — 4 physical modules

You listed five voice groups. They collapse naturally into **four boards**,
because several voices share a sound source and it would be wasteful (and
worse-sounding, due to part-to-part variation) to build that source twice.

| # | Module (suggested name)      | Voices on the board                         | Shared core                                  | Folder |
|---|------------------------------|---------------------------------------------|----------------------------------------------|--------|
| 1 | **Cymbals & Hats** ("Coliseum") | Closed HH, Open HH, Crash/Ride cymbal    | One 6-oscillator square "metal" core         | `3-hi-hat` + `4-cymbals` |
| 2 | **Snare / Clap (909)** ("Encore mkII") | 909 snare, Clap                    | Noise via the piggyback Noise Board          | `2-clap` |
| 3 | **Toms / Subkick**           | 2–3 tunable toms + subkick mode             | One pinged resonator topology, repeated      | `5-toms` |
| 4 | **Percussion** ("Bazaar")    | Cowbell, Shaker/Maracas, Claves/Rimshot     | Noise via the piggyback Noise Board          | `8-percussion` |

**Why Hats + Cymbals combine:** In an 808 the hats *and* the cymbal are the
same six square-wave oscillators. They only differ in the post-filter and
the envelope decay. Building the metal core once and fanning it out to three
envelope/VCA/filter chains is the canonical, cheapest, best-sounding option.
Note: 909 hats/cymbals are **samples**, so for an all-analog build the metal
voices are necessarily **808-style**.

**Why Snare + Clap combine:** Both are mostly filtered noise. Sharing one
noise generator guarantees they sit together tonally and saves a board.

**Subkick** lives with the toms because it is the same pinged-resonator
circuit tuned very low with a long decay — it is essentially "tom 0".

---

## Shared building blocks (your `building-blocks` library)

Every voice below is assembled from these. Characterise them once, then
reuse. References map to the sheets already in `building-blocks/`.

| Block | What it does | Core part(s) | Notes |
|-------|--------------|--------------|-------|
| **Trigger conditioner** | Gate/trig → clean fixed-width 5 V pulse + status LED | 1× comparator or Schmitt gate (40106), MMBT3904 | One per voice; debounces and squares ragged triggers |
| **Noise source** | Wide, loud white noise | Reverse-biased BJT (MMBT3904 B-E) + 2-stage TL07x gain | **Lives on the [Noise Board](../../helpers/noise-board/README.md) piggyback** — one good source, fanned out via a header. Not on the main boards |
| **Percussive envelope** | Fast attack + voltage/pot-controlled exponential decay | TL07x + diode, or 40106 one-shot | The workhorse; CH vs OH = same circuit, different decay cap |
| **Trigger repeater (multi-pulse)** | 3–7 retriggers with decaying spacing | 40106 / CD4017 | This is the heart of the **clap** |
| **VCA** | Voltage-controlled amp, low control feedthrough | LM13700 (OTA) | Trim for bleed; two channels per LM13700. Gain already CV-controllable |
| **State-variable / band-pass filter** | BP & HP outputs for metals | LM13700 or TL07x SVF | Tunes the "color" of hats/cymbals; **cutoff is CV-controllable** |
| **Low-pass VCF** | Body shaping | LM13700 | From your snare LPF sheet |
| **Square→triangle shaper** | Cleans osc cores into usable waveforms | TL07x integrator | For tom/snare bodies |
| **VCO (lin. CV)** | Tunable tone oscillator | CD4046 PLL VCO, or relaxation osc | Used by snare body & toms; **pitch is CV-controllable** |
| **VC percussive envelope** | Percussive env with **CV-settable decay** | LM13700 + env cap | The key to LFO-modulated open/close/decay (below) |
| **CV input conditioner** | Jack → attenuator/attenuverter → control node | TL07x + pot | One per CV input; scales the LFO/CV before summing |

---

## CV control (modulation) standard

This is what gives the kit movement over a track — an LFO slowly opening the
hats, drifting the snare tone, breathing the decay. Every module exposes CV
inputs, built three consistent ways in this SMD palette:

- **Tone / filter cutoff CV** — the metal/clap/cowbell filters are already
  OTA-based (LM13700). Cutoff is set by the OTA **control current**, so a CV
  input is just a resistor summing the (attenuated) CV into that control-
  current node. Free modulation, no extra IC.
- **Level / VCA CV (accent, dynamics)** — same trick on the VCA OTA: sum CV
  into its gain-control current. Accent is just a fixed version of this.
- **Decay / "open-close" CV** — the expressive one. Make the percussive
  envelope's decay **voltage-controlled** by discharging the envelope cap
  through an **LM13700 OTA** (or a JFET/transistor as a voltage-controlled
  resistor) instead of a fixed resistor: more CV → faster/slower discharge →
  shorter/longer decay. An LFO on this input continuously morphs closed↔open
  hats, tightens/loosens snares, lengthens tom tails. (A **vactrol** does
  the same job with extra "give" and is a nice optional flavour, but it's
  THT and slower.)
- **Pitch CV** — the CD4046/relaxation-osc cores take a CV straight into
  their tuning node (snare body, toms, cowbell, claves).

**Panel convention for CV:** each CV jack gets an **attenuator** (or
**attenuverter** on the hero inputs, so an LFO can add *or* subtract). Normal
the knob to a sensible base value so the voice still works with nothing
patched. Hero inputs (open/close, decay, tone) get a knob; secondary CVs can
be plain jacks at unity to save HP.

---

## Parts / BOM philosophy (rebuild standard)

**SMD-first.** Everything is SMD except where through-hole is mechanically
or sonically required:

- **Op-amps:** TL072 / TL074 in **SOIC-8 / SOIC-14**. Where you want lower
  noise/offset on the noise-gain and filter stages, **MCP6002/MCP6004** or
  **TL07xH** are pin-compatible upgrades.
- **OTAs:** **LM13700** in **SOIC-16** (still in full production, multi-
  sourced — the standard DIY OTA).
- **CMOS logic:** **CD40106B** (hex Schmitt inverter) and **CD4046B** in
  **SOIC**. These are the noise/clock/one-shot and VCO cores.
- **Transistors:** **MMBT3904 / MMBT3906** (SOT-23) as the generic NPN/PNP;
  **BC847/BC857** equivalents are fine. Use a **matched pair** (e.g.
  MMDT3904 dual, or hand-matched) for OTA-heavy stages.
- **Diodes:** **1N4148W** (SOD-123) for signal/envelope; **BAT54/1N5819**
  Schottky (SOD-123) where low forward drop matters (rectified envelopes).
- **Resistors/ceramics:** **0805** (easy hand-soldering, good values).
- **THT exceptions:** **Panel pots and 3.5 mm jacks** (Thonkiconn);
  **film caps** in audio/timing paths (MKS/MKP) where ceramic microphonics
  or tempco would hurt; **electrolytics** for PSU decoupling and large
  envelope timing caps.

All ICs above are jellybean, multi-sourced, and stocked at LCSC / Mouser /
Digi-Key today — nothing exotic or NOS.

---

## Format conventions

- **Power:** ±12 V via standard 10-pin Eurorack bus; reverse-protect + 78L/
  79L or LDO local regulation if a stage needs clean rails.
- **Levels:** ~10 Vpp audio internally, attenuated to ~8–10 Vpp at output.
- **I/O:** Trigger in (and per-voice trig where it makes sense), accent in,
  **CV inputs with attenuators** (see CV standard above), individual voice
  outputs **and** a mixed output where the board hosts several voices.
- **Width:** target **10–12 HP** per module — the extra room over the
  original 8 HP plan is spent on CV jacks and their attenuators. The
  Cymbals & Hats board is the busiest and sits at the 12 HP end.

See each subfolder's README for the per-module brief.
