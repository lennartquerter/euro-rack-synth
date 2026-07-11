# Sequencer ("flip side") — Firmware Development Plan

The quantizer v2 hardware doubles as a step sequencer: same brain + control
boards, faceplate flipped. Two CV/trig outs, two CV/trig ins, 12 buttons in
a circle around an encoder, 4 function buttons, 20 LEDs.

The sequencer is a **phrase player**, not a live step-programmer: it plays
pre-authored sequences from a compiled-in bank, selected from the panel (in
binary) or remotely by the **Conductor** module over I2C
([../../11-conductor/protocol.md](../../11-conductor/protocol.md)).
Sequence changes are queued and take effect at the next phrase boundary —
the running 4-bar phrase always completes.

This plan builds on the quantizer firmware
([development-plan.md](development-plan.md), Phases 0–6 implemented).

---

## 1. Decision: one firmware, two personalities

**A single firmware image containing both apps, selected at boot.**
Not two separate builds.

Why:

- **Flash is a non-issue.** The complete quantizer uses 62 KB of 512 KB
  (12%). The sequencer app plus a 64-sequence bank (~16 KB) fits many
  times over.
- **~80% of the code is shared.** `dac`, `cv_in`, `switches`, `leds`,
  `encoder`, `triggers`, `settings`, `cli`, and `quantize` (the sequencer
  reuses scale masks and CV transpose) are personality-independent. Two
  builds means every driver fix ships twice.
- **Calibration is shared.** DAC/ADC calibration lives in flash and is a
  property of the *board*, not the app. One image means you calibrate once
  and both personalities use it.
- **Switching personality coincides with disassembly anyway.** Flipping
  the faceplate means pulling the module — the strap is set in that same
  moment, and afterwards the module *is* what its panel says it is.
- **One I2C identity per personality.** The conductor protocol addresses
  module *types*: the same board answers as Sequencer (`0x04`–`0x07`)
  when flipped, or (future) as Quantizer (`0x08`–`0x0B`).

Rejected: two firmware images (reflash at every flip, two release
pipelines, calibration preservation risk); button-combo-only selection
(invisible state — kept as fallback, not primary).

## 2. Mode strap: PB2 (brain board)

**PB2 = MODE**, strapped high or low, read once at boot:

- **LOW (GND) = quantizer** (default / safe)
- **HIGH (3.3V) = sequencer**

Notes:

- PB2 was unused; no peripheral conflict.
- **PB2 is also BOOT1** on the STM32F411. Strap through a **10K
  resistor**, not a hard tie: with a hard tie to 3.3V, setting the
  BOOT_RUN jumper (BOOT0 = 1) would select SRAM boot instead of the ROM
  bootloader. Normal flash boots (BOOT0 = 0) ignore BOOT1 entirely. With
  a 10K strap this caveat disappears for SWD users and only reorders the
  BOOT0=1 case.
- Firmware side (Phase S0): add PB2 as `GPIO_Input`, label `MODE`, no
  internal pull (the strap defines the level), read it once before the
  personality dispatch.
- The four floating 74HC165 spare inputs still get tied to GND — that is
  board hygiene, tracked in [../TO_FIX.md](../TO_FIX.md) #4, and no
  longer carries the strap role.

Fallbacks, in priority order (highest wins):

1. **Flash override**: CLI `mode quantizer|sequencer|auto` — `auto`
   (default) follows the strap; the explicit modes pin the personality
   for boards without the strap.
2. **Boot combo**: holding the **encoder switch during power-up** toggles
   the flash override (LED feedback: left half lit = quantizer, right
   half = sequencer).
3. **PB2 strap** (when override is `auto`).

## 3. The flip: control-surface remapping

A flipped faceplate rotates the panel 180°. Buttons and LEDs keep their
electrical positions but their *panel* positions rotate. This is already
solved by design: `switches.c` and `leds.c` route everything through
`chain_map[]` / `led_map[]`; the sequencer personality gets **its own map
tables** (first guess: quantizer maps rotated by 6), finalized during
bring-up like the quantizer's.

Jack roles after the flip (electrical name → sequencer function):

| Electrical | Quantizer role | Sequencer role |
|---|---|---|
| CV IN A/B | pitch input A/B | transpose CV, lane A/B (quantized) |
| TRIG IN A | S&H trigger A | **clock in, 4 PPQN** (jack detect ⇒ external clock) |
| TRIG IN B | S&H trigger B | **reset in** (back to bar 1, beat 1) |
| CV OUT A/B | quantized pitch A/B | lane A/B pitch |
| TRIG OUT A/B | note-change pulse | lane A/B gate |
| I2C header | (future: quantizer slave) | **Conductor slave** |

## 4. Musical model

Timing hierarchy (terms used throughout):

```
1 phrase   = 4 bars                     ← a sequence is exactly one phrase
1 bar      = 8 beats
1 beat     = 4 sub-steps (4 PPQN)       ← one note slot per clock pulse
⇒ 1 phrase = 4 × 8 × 4 = 128 sub-steps
```

- **Clock input is 4 PPQN** (both lanes share the master clock): every
  incoming pulse advances one sub-step, so up to 4 notes per beat — this
  is what gives the sequences their character.
- The two lanes run in lockstep position-wise (same bar/beat counter);
  they differ in *content* (their loaded sequence).
- **Reset** (TRIG IN B rising edge) rearms to bar 1, beat 1, sub-step 1.
- **Internal clock** (no cable in the clock jack, per jack detect):
  a timer-generated 4 PPQN equivalent, BPM 30–300 on the encoder, with a
  fractional accumulator so there is no drift.

Panel display (note-button LEDs, sequencer maps):

| LEDs | Shows |
|---|---|
| 1–8 | current **beat** within the bar (chase) |
| 9–12 | current **bar** within the phrase (1–4) |

### Sequences

- A sequence is one phrase of per-sub-step events:
  `{ note (semitone + octave), gate, tie, accent }` — rest = gate off,
  tie holds the gate across sub-step boundaries (for notes longer than
  one sub-step), accent is reserved in v1 (no velocity output on this
  hardware) but carried in the data format from day one.
- Sequences are **pre-authored and compiled in** (see §6): the starter
  bank is 32 sequences for lane A (**LEAD** material) and 32 for lane B
  (**BASSLINE** material). **No count is hard-coded** — the firmware
  reads bank sizes from the generated tables, and the panel/CLI/I2C
  selection range follows automatically.
- Per-lane playback state on top of the sequence: transpose (CV in +
  conductor), octave shift, scale mask (reuses `quantize.c`; chromatic
  default), mute.

### Loading a sequence (queued switching)

From the panel:

1. Press the **LOAD** button for the lane (see §5 UI mapping).
2. Enter the sequence number **in binary** on the note buttons — buttons
   carry weights 1, 2, 4, 8, 16 (bit count = ⌈log₂(bank size)⌉, derived,
   not hard-coded). Example: pressing 4, 2 and 1 selects sequence #7.
   The entered value shows live on the corresponding LEDs.
3. Confirm (press LOAD again / encoder click). The selection is **armed**,
   not applied: the current phrase always plays to completion (all
   4 bars), and the new sequence starts clean at the next phrase boundary.
   The armed lane's LED blinks until the switch happens.

From the Conductor: `SET_PATTERN` with the quantization bits set to
`QUANT_PHRASE` behaves identically (see §7). Panel and I2C use the same
arming path in the engine.

## 5. Panel UI (sequencer personality)

| Control | Play mode | Shift (hold D) |
|---|---|---|
| Note buttons 1–8 | (display: beat chase) / binary entry in load mode | — |
| Note buttons 9–12 | (display: bar) / binary entry high bits if needed | — |
| Button A | LOAD lane A (enter/confirm binary select) | mute lane A |
| Button B | LOAD lane B | mute lane B |
| Button C | run/stop (internal clock) | direction (fwd/rev/pp/rand) |
| Button D | shift (hold) | — |
| Encoder turn | BPM (internal clock) | transpose active lane |
| Encoder click | confirm load / toggle edit focus lane | — |
| Status LEDs 1/2 | lane A/B loaded & unmuted (blink while armed) | |
| Status LEDs 3/4 | lane A/B gate activity | |

Exact mapping may shift during the UI phase (same disclaimer as the
quantizer plan) — the load flow and the beat/bar display are the fixed
requirements.

## 6. Sequence bank toolchain

Sequences are authored in a human-editable **`sequences.md`** and compiled
to C by a Python script — no hand-editing of generated code:

```
firmware/sequences/sequences.md          ← authored (bank A + bank B sections)
firmware/tools/gen_sequences.py          ← generator (python3, no deps)
firmware/Core/Src/app/sequences.c        ← GENERATED — do not edit
firmware/Core/Inc/app/sequences.h        ← GENERATED — do not edit
```

- Format sketch (finalized in Phase S1): one section per sequence with
  bank, id and name; one line per bar; one token per sub-step, e.g.
  `c2` note, `c2~` tie, `c2!` accent, `.` rest. 32 tokens per bar,
  4 bars per sequence — the script validates counts and note ranges and
  fails loudly on errors.
- Generated interface (shape, not final):

  ```c
  struct SEQ_step { int8_t note; uint8_t flags; };
  struct SEQ_sequence { const char* name; uint16_t step_count;
                        const struct SEQ_step* steps; };
  extern const struct SEQ_sequence SEQ_bank_a[];
  extern const uint16_t SEQ_bank_a_count;   // derived from the .md
  extern const struct SEQ_sequence SEQ_bank_b[];
  extern const uint16_t SEQ_bank_b_count;
  ```

- Everything downstream (panel bit count, CLI ranges, I2C `SET_PATTERN`
  validation, flash budget) derives from `*_count` at build/run time.
- Budget check: 64 sequences × 128 sub-steps × 2 bytes ≈ **16 KB** flash —
  comfortable; the bank can grow substantially before it matters.
- The generated files are committed (so the firmware builds without
  running the script) and regenerated whenever `sequences.md` changes;
  the script runs on the host, like the unit tests.

## 7. Conductor integration (I2C slave)

Per [protocol.md](../../11-conductor/protocol.md) (v0.1 draft):

- **Bus**: I2C1 (PB6/PB7), 400 kHz, slave mode. Pull-ups live on the
  Conductor — consistent with this board having none.
- **Address**: sequencer type `000001` → `0x04`–`0x07`. The board has no
  instance jumpers, so the instance (0–3) is a setting: CLI
  `i2c instance <0-3>`, stored in flash, default 0. Also listens to
  broadcast `0xFC`.
- **Frame handling**: `[CMD][LEN][PAYLOAD][SEQ][CHK]` — XOR checksum,
  duplicate suppression via per-command SEQ tracking (conductor sends
  everything twice), unknown commands silently dropped.
- **Quantized apply**: upper 2 bits of CMD (`NOW`/`BEAT`/`BAR`/`PHRASE`)
  map directly onto the engine's arming mechanism — the same one the
  panel load flow uses. `PHRASE` = this module's 4-bar boundary.
- **Command set (v1)**: common `NOOP`, `RESET`, `MUTE`, `IDENTIFY`; and
  sequencer `SET_PATTERN`, `SET_LENGTH`, `TRANSPOSE`, `SET_SCALE`,
  `SET_DIR`, `SET_OCTAVE`. `MUTATE` and `SET_DENSITY` are acknowledged
  -but-ignored in v1 (documented), implemented later.
- **Lane mapping (module-side convention, feed back into protocol.md)**:
  the frame has no lane field, so `SET_PATTERN` uses **bit 6 of
  pattern_id as the lane select** (0–63 = lane A/lead, 64–127 = lane
  B/bass); `TRANSPOSE`/`SET_SCALE`/`SET_OCTAVE`/`MUTE` apply to **both
  lanes**. `SET_LENGTH` "steps" are interpreted as **beats** (1–32).
- ISR discipline: bytes are captured in the I2C interrupt into a frame
  buffer; validation and application happen in the main loop, arming
  changes exactly like panel input does.
- Future (separate mini-phase, after the sequencer ships): the quantizer
  personality listens as type `000010` (`0x08`–`0x0B`) for
  `SET_SCALE`/`SET_ROOT`/`SET_CUSTOM`/`TRANSPOSE`.

## 8. Architecture changes

```
Core/Src/app/
  app.c          boot personality select (PB2/override) + shared scheduler
  personality.h  struct PERSONALITY { init, fast_loop, tick }
  app_quant.c    current quantizer fast path + glue   (moved out of app.c)
  ui_quant.c     current ui.c                          (renamed)
  app_seq.c      sequencer glue: clock sources, outputs, arming (new)
  ui_seq.c       sequencer panel UI                    (new)
  seq.c          engine core: position, sub-steps, arming, ties —
                 pure C, host-testable                 (new)
  sequences.c    GENERATED sequence bank               (new)
  i2c_slave.c    conductor protocol slave              (new)
  ...            all existing drivers unchanged
```

- `app.c` keeps ownership of: settings autosave, CLI tick, TRIGGERS
  poll/tick, LEDS tick — the personality supplies `fast_loop()`/`tick()`.
- `settings.c`: one payload holding `{ mode override, i2c instance,
  quantizer data, sequencer data (loaded seq ids, bpm, per-lane config),
  shared cal }`. Both personalities' state coexists; flipping back loses
  nothing. (No migration framework — no boards are in the field; the
  format can change freely until the first boards ship.)
- `quantize.c` reused for lane scale masks and transpose-CV quantization.
- CLI stays one shell; sequencer adds `seq`, `bpm`, `run`, `i2c` etc.;
  personality-foreign commands report "wrong personality".

## 9. Development phases

S0–S2 need no hardware at all (same situation as quantizer Phase 6).

### Phase S0 — Personality plumbing ✅ (implemented 2026-07-04)
- PB2 in the .ioc (`MODE`, input, no pull) + boot-time read; flash
  override + boot combo + CLI `mode`.
- `personality.h` dispatch; move quantizer code to `app_quant.c`/
  `ui_quant.c` with **zero behavior change** (host tests green, build
  size compared).
- Settings payload extended (override, instance, sequencer fields).
- Deliverable: quantizer identical; sequencer personality boots to a stub
  banner; `mode` switches between them.

### Phase S1 — Sequence toolchain ✅ (implemented 2026-07-04)
- Define the `sequences.md` grammar; write `tools/gen_sequences.py`
  (python3, stdlib only) with validation; generate `sequences.c/h`.
- Author the starter bank: 32 LEAD + 32 BASSLINE sequences (musical
  content iterates forever after; placeholders are fine to start).
- Deliverable: `python3 tools/gen_sequences.py` regenerates the bank;
  generated code compiles host-side and on target; counts flow from the
  file.

### Phase S2 — Engine core, host-side ✅ (implemented 2026-07-04)
- `seq.c` pure C: phrase/bar/beat/sub-step position, 4 PPQN advance,
  reset semantics, queued (armed) sequence switching at phrase/bar/beat
  boundaries, tie/gate shaping, transpose/octave/scale application,
  direction and length overrides (for the conductor commands).
- `test_seq.c` host tests beside `test_quantize.c` (same Makefile):
  boundary arming, reset-mid-phrase, tie across bar edges, bank-size
  independence (runs against a tiny generated test bank).
- Deliverable: `make test` covers the whole engine.

### Phase S3 — Clocking & outputs ✅ (implemented 2026-07-04, scope verification pending hardware)
- Internal 4 PPQN clock (fractional accumulator, BPM on encoder);
  external clock via `triggers.c` edges (already µs-class); reset input;
  lane pitch → DAC through shared calibration; gate/tie → trig outs.
- CLI: `bpm`, `run/stop`, `seq load <lane> <n>`, `seq status`.
- Deliverable: both lanes loop a bank sequence on scope, externally
  clockable at 4 PPQN, reset works.

### Phase S4 — Panel UI ✅ (implemented 2026-07-04, maps/bring-up pending hardware)
- `ui_seq.c`: beat/bar chase display, binary load flow with armed-blink,
  run/stop, mutes, shift layer (direction, transpose); sequencer
  `chain_map`/`led_map` tables.
- Deliverable: load and perform sequences without the CLI.

### Phase S5 — Conductor slave ✅ (implemented 2026-07-04, bus bring-up pending a conductor)
- `i2c_slave.c`: address + broadcast, frame parse, checksum, SEQ dedup,
  quantized apply through the engine's arming path; `i2c instance` CLI;
  `IDENTIFY` blinks the panel.
- Deliverable: conductor switches patterns live, phrase-quantized, while
  the module is externally clocked.

### Phase S6 — Polish
- Bank curation, README for the sequencer faceplate, `compiled/`
  artifacts, consumption note, finalize flip maps on hardware.

## 10. Risks / open points

- **PB2 = BOOT1**: strap through 10K, not a hard tie (see §2). Harmless
  for SWD flashing either way.
- **Interpretation pinned down**: "always playing 32" is read as the
  full phrase = 4 bars × 8 beats = **32 beats** (128 sub-steps at
  4 PPQN) always completing before an armed sequence takes over. If
  32 *bars* was meant, the phrase constant changes — it is one define +
  the .md grammar, but say so before Phase S1.
- **Conductor protocol is v0.1 draft** — frame/command details may move;
  `i2c_slave.c` isolates the protocol from the engine (arming API stays).
- **No instance jumpers** on this board while the protocol expects them —
  solved in firmware (flash setting), revisit if a rev B adds jumpers.
- **Clock loss handling** (external clock stops mid-phrase): v1 simply
  halts; gate-off on timeout is a Phase S3 detail to decide on hardware.
- **Flip maps are guesses until bring-up** — same procedure as the
  quantizer's.
- Internal-clock jitter bounded by the 1 kHz tick (±0.5 ms) — fine at
  musical tempos; external 4 PPQN is fast-loop polled (µs-class).
