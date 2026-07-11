(AI generated content)

# Lenimal Conductor I2C Protocol — v0.1 (DRAFT)

## Scope

Unidirectional broadcast protocol from the **Conductor** (single master) to
**slave modules** (sequencer, quantizer, drums, sampler, future).

Clock and reset are **not** transmitted over I2C — they are dedicated hardware
lines from the external master clock module. I2C carries only musical events:
pattern changes, transpositions, bank selects, parameter updates, CV target
values.

## Physical layer

- I2C bus, 400 kHz (Fast-mode), 3V3 logic.
- 3-pin header on every module: `SDA`, `SCL`, `GND`.
- Pull-ups (4k7) live on the Conductor only. Slaves do not pull up.
- Bus topology is daisy-chain or star; total bus capacitance budget ~400 pF.
- Use shielded ribbon or twisted pair if running across the rack.

## Addressing

8-bit address: `[TTTT TTII]` — 6 bits module type, 2 bits instance.

| Bits 7-2 | Type      | Notes                       |
|----------|-----------|-----------------------------|
| `000001` | Sequencer | `0x04`-`0x07` instances 0-3 |
| `000010` | Quantizer | `0x08`-`0x0B`               |
| `000011` | Drums     | `0x0C`-`0x0F`               |
| `000100` | Sampler   | `0x10`-`0x13`               |
| `111111` | Broadcast | `0xFC` — all modules listen |

Instance is set on the module via two 3-pin headers with a jumper each:
`NC = 0`, `left = 1`, `right = 2`. Combined gives instances `0`, `1`, `2`, `3`.

Silkscreen labels: `ADDR HI: • L R` / `ADDR LO: • L R`.

## Frame format

```
[START][ADDR][CMD][LEN][PAYLOAD...][SEQ][CHK][STOP]
```

| Field   | Bytes | Description                                   |
|---------|-------|-----------------------------------------------|
| ADDR    | 1     | Target address (type + instance) or broadcast |
| CMD     | 1     | Command code (see per-module tables below)    |
| LEN     | 1     | Payload length in bytes (0-32)                |
| PAYLOAD | 0-32  | Command-specific data                         |
| SEQ     | 1     | Monotonic sequence number, wraps at 256       |
| CHK     | 1     | XOR of CMD, LEN, PAYLOAD, SEQ                 |

`START` and `STOP` are I2C bus conditions, not bytes in the frame.

### Sequence number / idempotency

Conductor sends every event **twice**, ~3 ms apart. Slaves track the last
`SEQ` per command and ignore duplicates. This is cheap insurance against bus
noise without requiring acks.

### Error handling

- Bad checksum → silent drop. Conductor may retransmit on next slot.
- Unknown CMD → silent drop. Forward compatibility: new commands ignored by
  old firmware.
- Bus error / NAK → slave is offline; conductor logs but does not block.

## Timing

All event-applying commands accept a quantization mode in the **upper 2 bits
of CMD**:

| Bits 7-6 | Mode           | Slave action                                  |
|----------|----------------|-----------------------------------------------|
| `00`     | `QUANT_NOW`    | Apply immediately on receive                  |
| `01`     | `QUANT_BEAT`   | Apply on next beat boundary                   |
| `10`     | `QUANT_BAR`    | Apply on next bar boundary                    |
| `11`     | `QUANT_PHRASE` | Apply on next phrase boundary (slave-defined) |

The Conductor sends events **a few ms before** the intended boundary so the
slave has time to receive, decode, and arm. The slave then snaps to its own
next matching boundary using its hardware clock input.

Lower 6 bits of CMD are the actual command code (so up to 64 commands per
module type).

## Command tables

### Common (all modules)

| Code   | Name       | Payload    | Notes                      |
|--------|------------|------------|----------------------------|
| `0x00` | `NOOP`     | -          | Heartbeat / ping           |
| `0x01` | `RESET`    | -          | Reset module state         |
| `0x02` | `MUTE`     | `[on/off]` | Mute output                |
| `0x3F` | `IDENTIFY` | -          | Flash LED for ID/debugging |

### Sequencer (`0x04`–`0x07`)

| Code   | Name          | Payload                     | Notes                |
|--------|---------------|-----------------------------|----------------------|
| `0x10` | `SET_PATTERN` | `[pattern_id]`              | 0-127                |
| `0x11` | `SET_LENGTH`  | `[steps]`                   | 1-64                 |
| `0x12` | `TRANSPOSE`   | `[semitones (signed)]`      | -24 to +24           |
| `0x13` | `SET_SCALE`   | `[scale_id]`                | If quantized output  |
| `0x14` | `SET_DIR`     | `[0=fwd 1=rev 2=pp 3=rand]` |                      |
| `0x15` | `MUTATE`      | `[amount 0-127]`            | Probabilistic change |
| `0x16` | `SET_DENSITY` | `[0-127]`                   | Step probability     |
| `0x17` | `SET_OCTAVE`  | `[octave (signed)]`         | -3 to +3             |

### Quantizer (`0x08`–`0x0B`)

| Code   | Name         | Payload                | Notes          |
|--------|--------------|------------------------|----------------|
| `0x10` | `SET_SCALE`  | `[scale_id]`           | 0-127          |
| `0x11` | `SET_ROOT`   | `[note 0-11]`          | C=0            |
| `0x12` | `SET_CUSTOM` | `[12 bits note mask]`  | 2-byte payload |
| `0x13` | `TRANSPOSE`  | `[semitones (signed)]` |                |

### Drums (`0x0C`–`0x0F`)

| Code   | Name          | Payload           | Notes                |
|--------|---------------|-------------------|----------------------|
| `0x10` | `SET_PATTERN` | `[pattern_id]`    | 0-127                |
| `0x11` | `SET_BANK`    | `[bank_id]`       | Drum kit / sound set |
| `0x12` | `SET_DENSITY` | `[track][0-127]`  | Per-track density    |
| `0x13` | `SET_FILL`    | `[fill_id]`       | One-shot fill        |
| `0x14` | `MUTE_TRACK`  | `[track][on/off]` |                      |
| `0x15` | `SET_SWING`   | `[0-127]`         | 50%-75% swing        |
| `0x16` | `ACCENT`      | `[track][level]`  |                      |

### Sampler (`0x10`–`0x13`)

| Code   | Name            | Payload             | Notes               |
|--------|-----------------|---------------------|---------------------|
| `0x10` | `SET_BANK`      | `[bank_id]`         | Sample bank         |
| `0x11` | `SELECT_SAMPLE` | `[slot][sample_id]` | Per-slot assignment |
| `0x12` | `SET_PITCH`     | `[slot][semitones]` |                     |
| `0x13` | `SET_START`     | `[slot][0-127]`     | Start point         |
| `0x14` | `SET_LENGTH`    | `[slot][0-127]`     | Playback length     |
| `0x15` | `SET_LOOP`      | `[slot][on/off]`    |                     |
| `0x16` | `TRIGGER`       | `[slot]`            | One-shot trigger    |

### CV outputs (on Conductor itself, addressed for symmetry)

Address `0x00` is the Conductor's own CV/gate outputs. The score addresses
these like any other module.

| Code   | Name       | Payload                     | Notes                    |
|--------|------------|-----------------------------|--------------------------|
| `0x10` | `SET_CV`   | `[ch][value MSB][LSB]`      | 12-bit value             |
| `0x11` | `RAMP_CV`  | `[ch][target][bars][curve]` | Curve: 0=lin 1=exp 2=log |
| `0x12` | `LFO_CV`   | `[ch][rate][depth][shape]`  |                          |
| `0x13` | `SET_GATE` | `[ch][on/off]`              |                          |

## Open questions

- Do we need a `SYNC_QUERY` broadcast for boot-time discovery? Probably not v1.
- Should `MUTATE` be deterministic from a seed for reproducible sets? Lean yes.
- 32-byte payload max — enough for v1, may need extension for big sample lists.

## Module implementation notes

Decisions made by slave implementations that the Conductor must match.

### Sequencer: quantizer v2 "flip side" (firmware ≥ 0.8.0-s5)

Implemented in
[`../2-quantizer_v2/firmware/Core/Src/app/i2c_slave.c`](../2-quantizer_v2/firmware/Core/Src/app/i2c_slave.c).
The module is a two-lane phrase player: lane A = lead bank, lane B =
bass bank, both lanes locked to the same clock/position.

- **Lane addressing:** the frame has no lane field, so **bit 6 of
  `SET_PATTERN`'s `pattern_id` selects the lane** — `0–63` = lane A
  (lead bank), `64–127` = lane B (bass bank). Ids beyond the bank size
  are silently dropped (banks currently hold 32 sequences each, but the
  Conductor should not hard-code that).
- **Module-wide commands:** `TRANSPOSE`, `SET_SCALE`, `SET_OCTAVE` and
  `MUTE` apply to **both lanes**. Per-lane muting is panel-only for now.
- **`SET_LENGTH`:** the payload is interpreted as **beats** (quarter of
  the 4 PPQN grid), clamped to 1–32. The loop shortens; the phrase grid
  does not (see next point).
- **`QUANT_PHRASE`:** the slave-defined phrase is **4 bars × 8 beats =
  128 clock pulses at 4 PPQN**, a fixed grid counted from the last
  reset — a `SET_LENGTH` override does not move the phrase boundary.
- **`RESET`:** restarts the phrase on the **next clock pulse**
  regardless of the quant bits; phrase-armed changes apply on that pulse
  too (send `SET_PATTERN|QUANT_PHRASE` + `RESET` for an immediate clean
  switch).
- **`SET_SCALE` ids:** 0 chromatic, 1 major, 2 minor, 3 harmonic minor,
  4 pentatonic major, 5 pentatonic minor, 6 blues, 7 dorian,
  8 mixolydian, 9 whole tone, 10 octaves, 11 fifths. Other ids are
  dropped. Root stays at C (no root command yet).
- **`MUTATE` / `SET_DENSITY`:** accepted (checksum/SEQ processed) but
  ignored in v1.
- **Instance:** the board has no address jumpers; the instance (0–3) is
  a stored setting changed over the service port (`i2c instance <n>`),
  default 0.
- **Broadcast:** interpreted as 7-bit address `0x7E` (= `0xFC` on the
  wire with the W bit). Worth pinning down in this spec.
- **Master reads:** the module answers `0x00` bytes (protocol is
  write-only today).
- **Spec observation:** 7-bit addresses `0x00–0x07` fall in the I2C
  reserved range (`0x04–0x07` = HS-mode master codes). Fine on this
  private bus, but consider shifting the type table if third-party
  hardware ever joins.

### Quantizer personality (not yet implemented)

The same hardware flipped to quantizer mode will listen as type
`000010` (`0x08–0x0B`) for `SET_SCALE` / `SET_ROOT` / `SET_CUSTOM` /
`TRANSPOSE` — planned, not built.