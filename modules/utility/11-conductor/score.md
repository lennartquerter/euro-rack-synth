(AI generated content)

# Lenimal Score Format — v0.1 (DRAFT)

## Goals

- Human-readable, text-editable on any computer or via the module's screen.
- Parses fast on STM32 at boot; full set lives in RAM during performance.
- Diffs cleanly in git.
- Expresses both discrete events (pattern changes) and continuous automation
  (CV ramps, LFOs, density curves).
- Forgiving — comments, blank lines, loose whitespace.

## File structure

```
set_name.lscore
├── @meta       — set-level metadata
├── @aliases    — short names for modules and parameters
├── @section    — one or more sections defining the timeline
└── # comments  — anywhere, ignored by parser
```

## Top-level directives

### `@meta`

```
@meta
  name        = "Subterrain"
  bpm         = 132
  swing       = 0.54
  bars_per_phrase = 4
  beats_per_bar   = 4
```

### `@aliases`

Maps friendly names to I2C addresses. The conductor still uses addresses
internally; this is purely for score readability.

```
@aliases
  seq1     = sequencer.0       # I2C 0x04
  seq2     = sequencer.1       # I2C 0x05
  qz       = quantizer.0       # I2C 0x08
  drums    = drums.0           # I2C 0x0C
  sampler  = sampler.0         # I2C 0x10
  cv1      = conductor.cv.0
  cv2      = conductor.cv.1
  cv3      = conductor.cv.2
  gate1    = conductor.gate.0
```

### `@section`

```
@section <name> <start_time> length=<duration>
  <event>
  <event>
  ...
```

- `start_time`: absolute time `M:SS` or bar count `bar=N`. Both supported,
  bar count preferred for tight musical sync.
- `length`: `Nbars`, `Nbeats`, or `M:SS`.

## Event syntax

```
<target> <command> [args...] [@quant=now|beat|bar|phrase] [at=<offset>]
```

- `target`: module alias from `@aliases`.
- `command`: see protocol spec per module type.
- `@quant`: when the event should apply. Defaults to `bar`.
- `at`: offset within the section (e.g., `at=8bars`, `at=12beats`).
  Default is section start.

### Continuous automation

```
cv1 ramp 0..5 over=16bars curve=exp
cv2 lfo  rate=1/4 depth=2.5 shape=sine
```

The conductor expands these into a stream of `SET_CV` messages at an
appropriate rate (typically 50-100 Hz for smooth ramps).

### Probabilistic / generative

```
drums set_density track=hat value=0.85
seq1  mutate amount=0.15 seed=42
```

`seed` keeps mutation reproducible across performances.

## Comments and notation

```
# Full-line comment
seq1 set_pattern 3   # inline comment
```

## Example: 3-minute underground techno set

```
# ==============================================================
# "Subterrain" — 3-min techno demo set
# 132 BPM, 4/4, 4-bar phrases
# Structure: intro → groove → break → buildup → drop → outro
# ==============================================================

@meta
  name        = "Subterrain"
  bpm         = 132
  swing       = 0.54
  bars_per_phrase = 4
  beats_per_bar   = 4

@aliases
  drums    = drums.0
  seq1     = sequencer.0      # bassline
  seq2     = sequencer.1      # stab / lead
  qz       = quantizer.0
  cv1      = conductor.cv.0   # main filter cutoff
  cv2      = conductor.cv.1   # reverb send / FX
  cv3      = conductor.cv.2   # sub osc level
  gate1    = conductor.gate.0 # noise sweep gate

# --------------------------------------------------------------
# INTRO  0:00 - 0:22  (8 bars)
# Just kick + hat, sparse. Sets the pulse.
# --------------------------------------------------------------
@section intro bar=0 length=8bars
  qz    set_scale  phrygian @quant=now
  qz    set_root   A        @quant=now
  drums set_bank   0        @quant=now    # 909-ish kit
  drums set_pattern 1       @quant=bar    # kick + closed hat only
  drums set_density track=kick value=1.0
  drums set_density track=hat  value=0.5
  drums mute_track  track=snare on=1
  drums mute_track  track=clap  on=1
  seq1  mute        on=1
  seq2  mute        on=1
  cv1   set         value=0.3              # filter mostly closed
  cv2   set         value=0.0              # no reverb
  cv3   set         value=0.0

# --------------------------------------------------------------
# GROOVE  0:22 - 1:27  (24 bars)
# Bassline enters bar 8. Snare/clap bar 12. Stab on bar 16.
# Slow filter open across the section.
# --------------------------------------------------------------
@section groove bar=8 length=24bars
  seq1  set_pattern 2          @quant=bar           # rolling 16th bass
  seq1  mute        on=0
  seq1  set_octave  -1
  drums set_pattern 3          @quant=bar           # full kick+hat groove
  drums set_density track=hat  value=0.85

  # bring in snare/clap after 4 bars
  drums mute_track  track=snare on=0  at=4bars
  drums mute_track  track=clap  on=0  at=4bars

  # stab enters after 8 bars (=bar 16 absolute)
  seq2  set_pattern 5          at=8bars
  seq2  mute        on=0       at=8bars
  seq2  set_octave  1

  # slow filter open across whole section
  cv1   ramp 0.3..0.75 over=24bars curve=lin
  # touch of reverb creeping in
  cv2   ramp 0.0..0.25 over=24bars curve=exp

# --------------------------------------------------------------
# BREAK  1:27 - 1:49  (8 bars)
# Drop the kick. Bassline carries. Open filter. Reverb up.
# Mutate the stab pattern subtly.
# --------------------------------------------------------------
@section break bar=32 length=8bars
  drums mute_track  track=kick  on=1 @quant=bar
  drums mute_track  track=snare on=1 @quant=bar
  drums set_density track=hat   value=1.0
  seq2  mutate      amount=0.2 seed=7
  cv1   ramp 0.75..0.95 over=8bars curve=exp     # filter wide open
  cv2   ramp 0.25..0.7  over=8bars curve=exp     # reverb drench
  cv3   ramp 0.0..0.6   over=8bars curve=lin     # sub creeping in

# --------------------------------------------------------------
# BUILDUP  1:49 - 2:11  (8 bars)
# Snare roll, rising noise sweep, filter pinched then released.
# Everything held back until the drop.
# --------------------------------------------------------------
@section buildup bar=40 length=8bars
  drums mute_track  track=kick  on=0 @quant=bar    # kick returns
  drums set_pattern 7              # snare roll pattern
  drums set_density track=snare value=1.0

  # noise sweep gate opens, CV ramps up
  gate1 set         on=1
  cv2   ramp 0.7..1.0 over=8bars curve=exp        # massive reverb
  cv1   ramp 0.95..0.4 over=4bars curve=lin       # pinch the filter
  cv1   ramp 0.4..1.0  over=4bars curve=exp  at=4bars  # then slam open

  # accelerate snare last 2 bars
  drums set_pattern 8 at=6bars                     # 32nd-note roll
  seq1  set_density value=1.0

# --------------------------------------------------------------
# DROP  2:11 - 2:44  (12 bars)
# Everything in. Hard pattern jump. NOW quant = no mercy.
# --------------------------------------------------------------
@section drop bar=48 length=12bars
  gate1 set         on=0                @quant=now
  drums set_pattern 4                   @quant=now    # full peak-time
  drums set_density track=hat   value=1.0
  drums set_density track=snare value=0.7
  drums set_density track=clap  value=0.9
  seq1  set_pattern 6                   @quant=now    # heavier bassline
  seq1  set_octave  -1
  seq2  set_pattern 9                   @quant=now    # acid stab
  seq2  mutate      amount=0.1 seed=13
  cv1   set         value=0.7                         # filter back to sweet spot
  cv2   set         value=0.35                        # reverb tame
  cv3   set         value=0.8                         # sub locked in

  # subtle filter wobble across the drop
  cv1   lfo rate=1/8 depth=0.15 shape=sine

# --------------------------------------------------------------
# OUTRO  2:44 - 3:00  (6 bars + tail)
# Strip elements one phrase at a time. End on kick + sub.
# --------------------------------------------------------------
@section outro bar=60 length=6bars
  seq2  mute        on=1                @quant=bar
  cv1   stop_lfo
  cv1   ramp 0.7..0.4 over=4bars curve=lin
  seq1  mute        on=1                at=2bars
  drums mute_track  track=clap  on=1    at=2bars
  drums mute_track  track=snare on=1    at=4bars
  drums set_density track=hat   value=0.0  at=4bars
  cv2   ramp 0.35..0.0  over=6bars curve=lin
  cv3   ramp 0.8..0.0   over=6bars curve=exp

  # kick alone for last 2 bars then stop
  drums mute on=1   at=6bars  @quant=now
```

## Parser notes

- Two-pass: first pass resolves `@aliases` and `@meta`, second pass walks
  sections into a flat event list sorted by absolute bar.
- Continuous automations (`ramp`, `lfo`) compile into a small struct with
  start, end, duration, curve — expanded to I2C messages at runtime.
- Total memory for this 3-min set: ~80 events × 16 bytes = ~1.3 KB. Easily
  fits in any STM32.

## Open questions

- Do we want loop markers (`@loop section_name times=4`) for live extension?
- Crossfade syntax between sections — explicit, or auto if both sections
  define the same target?
- A "manual override" channel — events that fire only when the performer
  presses a button, not by timeline. Probably worth a `@cue` directive.
- Scale/chord progression as first-class concept, or always per-section?