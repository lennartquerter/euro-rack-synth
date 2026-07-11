# Drum modules — build handoff / reference

Consolidated reference for building the **step-by-step schematic guides**
(`SCHEMATIC.md`) and KiCad schematics for the remaining drum voices:
**Cymbals & Hats**, **Toms / Subkick**, and **Percussion**. It captures
everything established while designing the **Noise Board** (done, in
production) and the **Snare / Clap** (built — see its `SCHEMATIC.md` §12 for
the v2.1 rework), so the three new modules reuse the same proven blocks,
conventions, and lessons.

> Per-module *concept* briefs already exist as `README.md` in each folder
> (`3-hats`, `5-toms`, `8-percussion`). This doc adds the
> shared building blocks, real values, the document format to follow, and the
> gotchas — the stuff learned by actually drawing the first two boards.

---

## 0. House style — how to write each module guide

`2-clap/SCHEMATIC.md` is the reference (it went through the most review).
`3-hats/SCHEMATIC.md` is a second worked example. The rules below are what
"clear" means for this project — apply **all** of them.

**Document skeleton:**

1. **Title + status** line (design / ready for production / built).
2. **Conventions** — net labels, NC flags, "place the symbol then wire it".
3. **IC pinouts** — TL074, LM13700 (verified, §2), CD40106; give the **actual
   pin numbers**.
4. **IC map** table, by **section letter** (A/B/C/D per chip). Add the note:
   *unit numbers are a planning aid — KiCad auto-numbers refdes by placement,
   so match parts by function/section letter, not by Ux.*
5. **Net-name list** + an **ASCII architecture diagram** (signal flow).
6. **Numbered build sections** (see per-section rules).
7. **Bring-up order** — build & test one section at a time; shared inputs
   (noise, triggers) first.
8. **BOM** (SMD unless noted) + a **verified / starting-values** appendix.

**Per-section rules — this is the important part:**

- Open with **"What it does"** in one plain sentence, then **"How it works"**
  in plain terms (a few bullets; no jargon).
- **Split a section into labelled sub-parts (A, B, C…)** whenever it does more
  than one thing (filter → envelope → VCA). One step does one thing —
  never cram a whole stage into a single step.
- **Values inline in every step** — "**100 kΩ** from `X` → `Y`", never
  "place R5" with the value hidden in a parts list. **Do not keep a separate
  `Parts:` line.**
- **Readable NET_NAMES** (uppercase, backticked) for signals that matter or
  cross sub-parts (`SHELL_ENV`, `CLAP_BP`, `METAL_BUS`). **Don't invent a name
  for every little local node** — describe local junctions by their parts
  ("the HP node", "the timing node"). No `va1`/`env1`/`CLK0` clutter.
- **Number the IC pins** in every step, especially LM13700 VCA steps
  (amp A = 1/3/4/5/7/8, amp B = 16/15/14/13/12/10/9 — see §2).
- End each section with a one-line ***Test*** checkpoint (what to scope/hear).
- For a complex section, add a tiny **ASCII flow** at the top (see the clap
  generator, §8 of the Snare/Clap guide).

**Design philosophy the guides bake in:**

- **Working voice first, controls last.** Build every stage with *fixed
  resistors* so each is easy to scope; then a final **Controls & CV** section
  breaks parameters out into knobs and CV jacks. *Explain* this so the reader
  knows why the knobs come last. Only the pots a stage literally can't be set
  without (tuning, decay) live in their own stage.
- **Two control recipes** (state them in the Controls section): (A) *fixed R →
  pot* (wiper to one outer leg + a small series min-resistor stop); (B) *CV
  jack → attenuator → 100 kΩ into the control node* — Iabc for level, a
  voltage-controlled resistor (vactrol or OTA) across the cap for decay, an
  OTA control-current for filter cutoff.
- **Decay pots get a ~4.7 kΩ series min-resistor** so fully-CCW isn't a dead
  short (which would stop the envelope charging entirely).
- **Film caps** in every tuning/timing path (oscillators, resonators, metal
  core). **Verify** oscillator/filter values numerically and clearly mark the
  ones that are **bench-tuned**.
- **Verify pinouts against the datasheet** — don't trust memory (the LM13700
  pinout in §2 is datasheet-confirmed).

---

## 1. Global conventions & constraints

- **Supply:** ±12 V from the Eurorack 10-pin bus; GND is the 0 V mid-rail, so
  audio swings both ways around ground (no bias network needed). Reuse the
  user's standard reverse-protected power inlet block.
- **Build style — SMD-first, with THT exceptions:**
    - SMD: op-amps (SOIC), OTAs (SOIC-16), logic (SOIC), transistors (SOT-23),
      diodes (SOD-123), 0805 R/C.
    - **THT:** panel pots, 3.5 mm jacks (Thonkiconn), **film caps in every
      tuning/timing path** (oscillator/resonator/metal-core caps — ceramic
      drifts and is microphonic), electrolytics, trimmers (**RM-065**,
      5.0×5.0 mm triangular footprint, single-turn).
- **Parts palette (jellybean, all current/multi-sourced):**
    - **TL072 / TL074** op-amps (SOIC-8 / SOIC-14).
    - **LM13700** dual OTA (SOIC-16) — the VCA / VC-filter / VC-decay workhorse.
    - **CD40106B** hex Schmitt inverter (SOIC-14) — oscillators, one-shots.
      **Runs 0–12 V (VDD=+12, VSS=GND), not ±12.**
    - **CD4046B** PLL VCO (SOIC-16) — optional wide-range/CV oscillator.
    - **MMBT3904 / MMBT3906** (SOT-23) NPN/PNP; **2N3904** (TO-92) where hand-
      select matters.
    - **1N4148W** (SOD-123) signal/envelope; **BAT54 / 1N5819** Schottky where
      low drop matters.
- **Panel target:** 10–12 HP per module.
- **KiCad:** use net labels for routing. Refdes are auto-assigned by placement
  — **match parts by function**, not by U-number.

---

## 2. IC pinouts (copy into each guide)

**TL074** (quad op-amp): A = pins 1(out)/2(−)/3(+) · B = 7(out)/6(−)/5(+) ·
C = 8(out)/9(−)/10(+) · D = 14(out)/13(−)/12(+). Power: **4 = +12 V,
11 = −12 V**.

**LM13700** (dual OTA) — **verified against the TI datasheet** (symmetric).
Amp A: **1** Iabc · **2** diode-bias · **3** +in · **4** −in · **5** output ·
**7** buffer-in · **8** buffer-out. Amp B: **16** Iabc · **15** diode-bias ·
**14** +in · **13** −in · **12** output · **10** buffer-in · **9** buffer-out.
Supplies: **V+ = pin 11, V− = pin 6.**

**CD40106** (hex Schmitt inverter): inverter pairs in/out = (1→2), (3→4),
(5→6), (9→8), (11→10), (13→12). **VDD pin 14 = +12 V, VSS pin 7 = GND.**

> Confirmed from the TI LM13700 datasheet (SNOSBW2F) Pin Functions table:
> Iabc = **1 & 16** · diode-bias = **2 & 15** · +in = **3 & 14** ·
> −in = **4 & 13** · output = **5 & 12** · buffer-in = **7 & 10** ·
> buffer-out = **8 & 9** · **V+ = 11 · V− = 6**. It's the clean symmetric
> pinout. (An earlier draft had amp B and the supply pins wrong — this is it.)

---

## 3. The Noise Board (shared source — DONE, ready for production)

A small SMD daughterboard (**25 × 35 mm**) that piggybacks on the back of any
host via headers. One known-good white-noise generator, reused everywhere.
Full build: `helpers/noise-board/SCHEMATIC.md`.

**Circuit chain:** clean +12 V (1 kΩ + 10 µF RC filter → `+12V_F`) → **noise
source** (reverse-biased transistor B-E junction in avalanche: 1 MΩ from
+12V_F into the emitter, base to GND, collector NC; **dual footprint** SOT-23
MMBT3904 *or* TO-92 2N3904 — fit one, socket the TO-92 and test-select for the
loudest) → AC-couple (1 µF) → **gain A** (inverting ×100, TL072) → **gain B**
(inverting, **1 MΩ RM-065 trimmer** to set ~6 Vpp white) → output series 1 kΩ

+ AC-couple → **white out**. Optional **pink**: 12 kΩ feed → 4-leg RC ladder
  (7k5/220n · 2k7/82n · 1k8/33n · 330R/22n) → ×4.9 buffer (2nd TL072) → pink
  out. Pink slope simulation-verified at **−9.97 dB/dec, 1 dB ripple**
  (20 Hz–20 kHz).

**"Lenimal Noise Header v1" (use this on every consumer):**

- **J1 — power, 1×3:** pin 1 +12 V, pin 2 GND, pin 3 −12 V (host feeds the board).
- **J2 — output, 1×2:** pin 1 = white noise (buffered, low-Z, AC-coupled,
  ~6 Vpp), pin 2 = pink (only if the pink section is fitted). Return is the
  GND on J1.

**Host-side connection (the consuming module):**

- Place a **1×3 socket** (sends power up) + a **1×2 socket** (receives noise).
- `NOISE_IN` = J2 pin 1, with a **100 kΩ to GND** (references the node to 0 V,
  silent if no board fitted). **No coupling cap** — the board output is
  already AC-coupled and every consumer's input has its own series cap.

**Who needs noise:** Snare/Clap **yes**, Percussion **yes** (shaker, rimshot),
Cymbals & Hats **optional** (a little white blended in for "air" — unpopulated
by default), Toms **no**.

---

## 4. Reusable building blocks (proven this build — use these verbatim)

### 4.1 Trigger conditioner (comparator block, from the kick)

Turns any gate/trigger into a clean fixed pulse. **Use this for every voice's
trigger.**

- Input jack tip → `*_TRIG`; **100 kΩ to GND** (input reference).
- **10 nF series** → `*_SPIKE`; **diff resistor to GND** — **use 100 kΩ
  (~1 ms pulse) whenever the fire pulse charges 1 µF envelope caps** (the
  39 kΩ short pulse is only enough for small caps like the kick's);
  **clamp 1N4148 with cathode→`*_SPIKE`, anode→GND** (kills
  the negative spike — *this direction matters*; reversed it eats the
  positive spike and nothing fires).
- **Comparator** (one TL074 section): spike → +in; threshold on −in via
  **100 kΩ from +12 V + 33 kΩ to GND ≈ +3 V**. Output = clean fire pulse.
- Threshold ~3 V means it wants a **≥ ~3.5 V trigger** (fine for 5–10 V).
- **Feeding a CD40106 from this:** the comparator output swings ±~10.5 V, and
  a 40106 at +12 V only flips at **~7 V**. Convert to a clean 0→+12 V edge:
  fire pulse → **series 1N4148 (anode = pulse) → node + 100 kΩ to GND**, then
  into the 40106 input.

### 4.2 Percussive envelope (attack-decay)

- Fire pulse → **1N4148 (anode = fire) → cap (1 µF)** to GND = instant attack.
- **Decay pot (~500 kΩ, wiper to one outer leg = variable resistor) + 4.7 kΩ
  series min-resistor** from the cap to GND = the decay; bigger = longer (the
  stop keeps fully-CCW from shorting the cap *and* the fire pulse).
- **Op-amp follower** buffers the cap so reading it doesn't drain it → the
  control voltage.
- Attack caveat: if the fire pulse is too short to fully charge a 1 µF, use
  0.1 µF + a larger decay pot, or widen the pulse (bigger diff resistor).
- **Decay times:** τ = R·C. 1 µF + 150 k ≈ 150 ms; + 500 k ≈ 500 ms.

### 4.3 OTA VCA (LM13700) — the standard voice gate

- **Attenuate** the audio to OTA level: **22 kΩ in series + 100 Ω to GND**
  (~25–30 mV from a ±7 V source — the OTA's linear range; a touch of grit is
  fine for drums).
- Attenuated audio → **pin 3 (+in)**; **pin 4 (−in) → GND**.
- Envelope/control → **47 kΩ → pin 1 (Iabc)** (sets the gain).
- **Bleed null (mandatory, every VCA):** **4.7 kΩ + 5 kΩ RM-065 trimmer in
  series** from pin 1 → **−12 V**; trim silent at rest. *(The Iabc pin sits
  ≈1.2 V above −12 V, so a 0 V envelope through a bare resistor still pushes
  ~100 µA in — with a ~10 V envelope peak that's only ~6 dB below the hit:
  the voice drones, and env-discharge tricks like the hats' choke can't
  fully mute it. The null pulls the idle current back out; peak Iabc stays
  ~210 µA. The advice that used to live here — "one NPN: base ← env,
  emitter → GND, collector → pin 1" — does **not** work: the collector sits
  ~11 V below the emitter (reverse-biased), and the forward-biased B–C
  junction leaks the same ~100 µA at env = 0.)*
- **pin 5 (current out) → 100 kΩ → GND** (I-to-V) and → **pin 7 (buffer in)**;
  **10 kΩ from pin 8 (buffer out) → −12 V** — the Darlington buffer only
  *sources*, so without this pull-down the negative half-cycle distorts —
  then a **10 µF** coupling cap off pin 8 = the gated audio (strips the
  buffer's ~−1.2 V offset).
- **CV on the VCA gain** (accent, "snappy", level): **not into the Iabc
  node** — any 0 V-at-rest source through a resistor there re-injects the
  idle bleed the null removes. Sum it into the **envelope cap** instead
  (jack → 1N4148, anode = jack → ~33 kΩ → cap node): a coincident pulse
  charges the envelope higher = louder hit (this is also how the classic
  machines accent).

### 4.4 Triangle/square oscillator (the LFO core — for tonal bodies)

Integrator (op-amp A) + non-inverting Schmitt (op-amp B):

- **Integrator:** square (from the Schmitt's divided output node) → **R_int**
  → −in; **cap C** −in→out; +in→GND. Output = triangle.
- **Schmitt:** triangle → **R_in (100 kΩ)** → +in; positive feedback from the
  square-divider node → **R_fb (100 kΩ)** → +in; −in→GND. Output = square →
  **4k7 + 10 kΩ divider to GND** (sets the internal ±~7 V square; also feeds
  R_fb and R_int).
- **Frequency:** with R_in = R_fb, **f = 1 / (4 · R_int · C)**. Triangle
  amplitude = (R_in/R_fb)·(divider level) ≈ ±7 V.
- **Verified body values:** C = 10 nF; **R_int = 135 kΩ → 185 Hz**,
  **R_int = 75 kΩ → 333 Hz**.
- **Tune:** a dual-gang pot in series with R_int (one gang per oscillator),
  target at pot minimum so it tunes *down*. **B250 K** → ~185→65 Hz /
  333→77 Hz; B500 K → wider but twitchier. 470 Ω series stop on the wiper.
- Put **test points** on triangle and square nodes for bench tuning.

### 4.5 Inverting summing mixer

- Each input **100 kΩ → −in**; **100 kΩ feedback**; +in → GND. Unity sum.
- **Headroom:** summing several full-level signals can clip the rails — drop
  the feedback (e.g. 47 kΩ) to scale down when stacking.

### 4.6 Filters

- **1st-order RC:** corner **fc = 1/(2π·R·C)**. HPF = series C + shunt R;
  LPF = series R + shunt C. Buffer with an op-amp follower.
- **Band-pass:** cascade HPF + LPF (gentle) or a multiple-feedback band-pass
  (peakier). Example BP ~1 kHz = HPF 22 k/10 nF (≈720 Hz) + LPF 10 k/10 nF
  (≈1.6 kHz).

### 4.7 Pinged resonator (bridged-T / twin-T) — for toms, claves, rimshot

- A twin-T/bridged-T network around a transistor or op-amp set just below
  self-oscillation; a trigger **pulse "pings"** it → it **rings out as a
  decaying sine** at its resonant frequency. Same family as the user's
  working Muay Thai kick / Basari clone — reuse the resonator + ping stage.
- **Tune** = the bridged-T resistance (pitch); **Decay** = feedback/Q (ring
  length). **Film caps in the network** (tuning-critical). The natural ring
  can serve as the envelope; add a VCA only to gate the tail cleanly.

### 4.8 CV input standard

Jack → **attenuator pot** (attenuverter on hero inputs so an LFO can add or
subtract) → sum into the control node:

- **Tone/filter cutoff** → into the OTA filter's control current (free,
  OTA-based).
- **Level / accent / snappy** → into the VCA Iabc.
- **Decay / open-close** → a **VC-decay**: discharge the envelope cap through
  an **LM13700 OTA** instead of a fixed resistor (CV sets discharge current =
  decay time). This is what makes an LFO morph open/close/decay over a track.
- **Normal every knob to a sensible base value** so the voice works unpatched.

---

## 5. Module: Cymbals & Hats — `3-hats`, **808-style**

(909 hats/cymbals are samples, so analog metals must be 808.) Concept brief:
`3-hats/README.md`. ~**12 HP**.

**Metal core (shared by all three voices):** **six square-wave oscillators**
at inharmonic frequencies — **205.3, 304.4, 369.6, 522.7, 540, 800 Hz** —
each = one **CD40106B** inverter + RC (two CD40106B give all six + spares).
**Film timing caps** (drift detunes the metal "chord"). Sum the six through
equal ~100 kΩ resistors into a TL074 inverting mixer = the **metal bus**.
40106 oscillator freq ≈ 1/(1.2·R·C) (bench-tune).

**Per-voice chains off the metal bus:**

- **Closed Hat / Open Hat** (share one LM13700 = two VCA channels): sharp
  **high-pass** (~6–9 kHz, 2-pole — make it aggressive or it sounds like buzz,
  not a tick) → VCA → percussive envelope. CH decay short (~40–120 ms), OH
  long (~300 ms–1 s). **Choke:** a CH trigger mutes the OH VCA (signature
  techno). Output HPF after the VCA tightens it.
- **Crash/Ride cymbal:** broad **band-pass** (~3–6 kHz, more low-mid than the
  hats) → VCA → **two-stage envelope** (fast "chick" + long wash tail) for
  crash vs ride.

**CV inputs (hero = Open/Close):** Open/Close (lengthens OH decay + eases
choke), OH Decay, CH Decay, HH Tone (post-HPF cutoff), Cymbal Decay, Cymbal
Tone. **Optional white-noise "air"** via the Noise Board header (unpopulated
by default).

**Core BOM (as per the 3-hats guide):** 1× CD40106B (all six inverters),
3× TL074, 2× LM13700 (3 VCAs + terminated spare), MMBT3904, 1N4148W,
**THT film metal-core caps**, 3× RM-065 bleed trimmers, pots/jacks.

---

## 6. Module: Toms / Subkick — `5-toms`

Concept brief: `5-toms/README.md`. ~**10–12 HP**. **Two flavours, offer both:**

- **Acoustic-ish tom (808):** the **pinged bridged-T resonator** (§4.7) — Tune
  = pitch, Decay = ring length. Three of them at three tunings.
- **Electro tom (Syndrum):** a **CD4046 VCO + fast downward pitch-sweep
  envelope** (the "pew/dewww" drop) — great for techno and for the subkick.

**Voices/tunings:** Hi ~300–400 Hz, Mid ~180–250 Hz, Lo ~100–150 Hz,
**Subkick ~45–60 Hz** (= the resonator tuned very low, long decay, pitch-sweep
on, optional gentle diode soft-clip for sub weight — "tom 0").

**Per voice:** trigger conditioner (§4.1) → **ping pulse** (transistor) into
the resonator → VCA + percussive envelope (or use the natural ring). Optional
pitch-sweep CV on the resonator/VCO tuning.

**CV inputs (hero = Tune):** Tune (per-voice, or one CV normalled to all three
to save HP), Decay, Pitch-Sweep amount, Accent.

**Core BOM:** 2× TL074, 1–2× LM13700, CD4046B (optional electro/subkick),
CD40106B (ping one-shots / sweep env), MMBT3904/3906, 1N4148W, **THT film
twin-T caps**, pots/jacks. **No Noise Board needed.**

---

## 7. Module: Percussion (Cowbell / Shaker / Claves) — `8-percussion`, **808**

Concept brief: `8-percussion/README.md`. ~**10 HP**.

- **Cowbell:** two **CD40106B square oscillators ~540 Hz + 800 Hz** (ratio
  ≈1.48, the inharmonic clank), **film timing caps** → sum → **multiple-
  feedback band-pass ~2–2.6 kHz** (TL074) → VCA, fast attack / medium decay
  (~150–400 ms). *(These two frequencies are oscillators 5 & 6 of the 808
  metal core — mirror them here, or tap the Hats board.)*
- **Shaker / Maracas:** **noise from the Noise Board header** → **high-pass
  ~5 kHz** → VCA, very short decay (~30–80 ms). Feed the VCA from a trigger
  repeater for rolls.
- **Claves / Rimshot:** a **twin-T pinged resonator** (§4.7) ~**2.5 kHz**
  (claves) / ~**1.7 kHz** (rimshot, + a touch of noise) → VCA, short envelope.

**CV inputs:** Cowbell Tone (BP center) ★, Cowbell Decay, Shaker Decay, Claves
Pitch, Accent (one jack into all VCAs).

**Core BOM:** CD40106B (cowbell osc + ping one-shots + shaker env), 2× TL074
(BP, HPF, resonator, mixing), 1–2× LM13700 (3 VCA channels), MMBT3904/3906,
1N4148W, **THT film** cowbell + twin-T caps, pots/jacks, **Noise-In header**.

---

## 8. Verified values / formulas (quick reference)

| Thing                                | Formula / value                                                  |
|--------------------------------------|------------------------------------------------------------------|
| Triangle osc frequency (R_in = R_fb) | **f = 1/(4·R_int·C)**; 135 k/10 nF = 185 Hz, 75 k/10 nF = 333 Hz |
| Envelope decay (to 37%)              | **τ = R·C**; 1 µF + 150 k ≈ 150 ms                               |
| 1st-order RC corner                  | **fc = 1/(2π·R·C)**                                              |
| OTA input attenuator                 | 22 k : 100 Ω → ~25–30 mV from ±7 V                               |
| Noise-board pink ladder (verified)   | 12 k feed + 7k5/220n · 2k7/82n · 1k8/33n · 330R/22n → −3 dB/oct  |
| 808 metal/cowbell freqs              | 205.3, 304.4, 369.6, 522.7, **540**, **800** Hz                  |
| Clap stutter (bench-tune)            | ~28 ms gate + ~110 Hz burst osc → ~3 pulses                      |

---

## 9. Gotchas / lessons (apply to all three modules)

1. **Bring up the shared Noise Board first** and scope it before the VCAs —
   weak noise is the usual reason a voice is silent.
2. **Film caps in every tuning/timing path** (metal core, twin-T resonators,
   oscillator integrators). Ceramic drifts and is microphonic.
3. **Trigger comparator threshold ~3 V** → needs ≥~3.5 V triggers. **CD40106
   flips at ~7 V at +12 V** → feed it a comparator pulse via series diode +
   100 k pulldown, not a raw 5 V edge.
4. **Trigger clamp diode: cathode to the signal node** (clamps the negative
   spike).
5. **OTA Iabc bleed:** a bare resistor from a ground-ref envelope leaves
   ~100 µA rest bias → the voice drones at only ~6 dB below the hit (not
   "faint"). Fit the §4.3 null network (4.7 k + 5 k trimmer → −12 V) on
   **every** VCA, standard. The old NPN-driver advice doesn't work
   (reverse-biased collector; B–C junction leaks the same bleed).
6. **Mixer headroom:** scale feedback down when summing several full-level
   sources so simultaneous peaks don't clip.
7. **Coupling:** op-amp outputs DC-couple fine (downstream filters block DC),
   but anything taken from an LM13700 **buffer** (pins 8/9) sits ~−1.2 V and
   needs its 10 µF (§4.3). Don't stack redundant caps elsewhere. A small
   **X7R MLCC** is fine as a coupling cap on noise (distortion irrelevant for
   hiss).
8. **KiCad refdes auto-number by placement** → match parts by function/section
   letter, not by Ux.
9. **JLC assembly:** for any dual-footprint part (like the noise-board Q1),
   assign one and **DNP** the other or the pick-and-place flags the overlap.
10. **Test points** on oscillator/resonator/envelope nodes — these voices are
    bench-tuned, not purely closed-form.

---

## 10. Repo layout

- `helpers/noise-board/` — shared noise source (README + SCHEMATIC.md). **Done.**
- `modules/drums/README.md` — drums index (grouping, CV standard, BOM
  philosophy). Contains the **CV control standard** (anchor:
  `#cv-control-modulation-standard`).
- `modules/drums/2-clap/` — Snare/Clap (Encore mkII): `README.md` concept +
  `SCHEMATIC.md` full build guide (the **format template** to copy).
- `modules/drums/3-hats/` — Cymbals & Hats: `README.md` concept +
  `SCHEMATIC.md` full build guide (v1.1).
- `modules/drums/5-toms/` — Toms/Subkick brief.
- `modules/drums/8-percussion/` — Percussion brief.
- Cross-link new guides to `../README.md` (CV standard) and
  `../../../helpers/noise-board/SCHEMATIC.md`.

---

## 11. Copy-paste subagent prompts

**Cymbals & Hats:**
> *(Done — kept for reference.)* Build `modules/drums/3-hats/SCHEMATIC.md`, a step-by-step schematic build
> guide for the 808-style Cymbals & Hats module (closed hat, open hat,
> crash/ride). Follow the exact format of `modules/drums/2-clap/SCHEMATIC.md`
> (sections with inline-value place-and-wire steps, numbered IC pins, readable
> net names, test checkpoints, BOM, verified-values appendix). Read
> `modules/drums/build-handoff.md` (§2, §4, §5, §9) and the existing
> `3-hats/README.md` first. Build it from the shared
> blocks in the handoff (40106 metal core, summing mixer, sharp HPF, LM13700
> VCA, percussive envelope, choke). Verify the six metal-core RC values and
> the HPF corner numerically before finalizing.

**Toms / Subkick:**
> Build `modules/drums/5-toms/SCHEMATIC.md` for 3 tunable toms + subkick,
> same format as `2-clap/SCHEMATIC.md`. Read `build-handoff.md` (§2, §4, §6,
> §9) and `5-toms/README.md`. Use the pinged bridged-T resonator block (§4.7)
> for the acoustic toms and offer the CD4046 + pitch-sweep (Syndrum) path for
> electro toms / the subkick drop. Compute resonator R/C for the target
> tunings and the pitch-sweep timing; mark bench-tune points.

**Percussion:**
> Build `modules/drums/8-percussion/SCHEMATIC.md` for cowbell + shaker +
> claves/rimshot, same format. Read `build-handoff.md` (§2, §3, §4, §7, §9)
> and `8-percussion/README.md`. Cowbell = two 40106 squares (540/800 Hz) →
> MFB band-pass → VCA; shaker = Noise-Board noise → HPF → short VCA; claves/
> rimshot = twin-T pinged resonator. Wire noise in via the Noise Header
> (§3). Compute the band-pass and resonator values numerically.
