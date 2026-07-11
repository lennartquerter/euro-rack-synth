# Quantizer / Sequencer v2 — User Manual

One Eurorack module, two instruments. With the faceplate mounted one way
it is a **dual-channel quantizer**; flip the faceplate and it becomes a
**two-lane phrase sequencer**. Both personalities live in the same
firmware — a jumper on the board selects which one boots.

The panel is the same in both cases: 12 illuminated buttons in a circle
around a push encoder, 4 function buttons (A, B, C, D), 4 status LEDs,
2 CV inputs, 2 CV outputs, and 2 trigger inputs/outputs.

## Specifications

|                 |                                         |
|-----------------|-----------------------------------------|
| Power           | Eurorack ±12 V (consumption: TBD)       |
| CV outputs      | 0 – 10 V, 1 V/oct, 12-bit               |
| CV inputs       | −4 V … +9 V usable range                |
| Trigger outputs | ~8 V, pulse or gate                     |
| MCU             | STM32F411, USB-less; USART service port |

## Choosing the personality

- **MODE jumper** (on the brain board): open/low = quantizer,
  high (3.3 V) = sequencer. Set it when you flip the faceplate.
- **Panel override**: hold the **encoder button while powering on** to
  flip the personality (left half of the note LEDs lights = quantizer,
  right half = sequencer). The choice is remembered.

---

# Quantizer

Two independent channels (A and B) snap incoming CV to the notes you
allow. Input A is normalled to input B's jack.

**Jacks**: CV IN A/B → quantized CV OUT A/B. TRIG IN A/B: with a cable
inserted, that channel switches to **sample-and-hold** — it only
re-quantizes on a trigger rising edge (clocked quantization). Without a
cable it tracks continuously. TRIG OUT A/B fires an ~8 ms pulse whenever
the channel's output note changes.

## Controls

| Control         | Action                                                                                                                                 |
|-----------------|----------------------------------------------------------------------------------------------------------------------------------------|
| Note buttons    | Toggle that semitone in the active channel's scale; LEDs show the allowed notes                                                        |
| Button A / B    | Select channel A / B                                                                                                                   |
| Button C        | Scale browse: turn the encoder to step through the presets (applied live); C, encoder click, or 8 s exits. LED C blinks while browsing |
| Button D (hold) | The root note's LED blinks; **D + note button** sets the root                                                                          |
| Encoder turn    | Transpose the active channel ±24 semitones                                                                                             |
| Encoder click   | Switch channel                                                                                                                         |
| Status LEDs 1/2 | Active channel                                                                                                                         | 
| Status LEDs 3/4 | Blink when a channel changes note                                                                                                      |

**Scale presets** (browse with C): chromatic, major, minor, harmonic
minor, pentatonic major/minor, blues, dorian, mixolydian, whole tone,
octaves, fifths. Editing notes after loading a preset makes it a custom
scale. Everything is saved automatically a couple of seconds after the
last change.

---

# Sequencer

A phrase player: it performs pre-composed sequences from an internal
bank — 32 **lead** sequences for lane A and 32 **bassline** sequences
for lane B. A sequence is one phrase of **4 bars × 8 beats**, with up to
4 notes per beat.

**Jacks**: TRIG IN A = **clock in, 4 pulses per beat** — inserting a
cable switches from the internal clock to the external one
automatically. TRIG IN B = **reset** (back to bar 1 on the next pulse).
CV OUT A/B = lane pitch, TRIG OUT A/B = lane gates (long notes play
legato).

## Controls

| Control              | Play mode                                                                    | With shift (hold D)                                       |
|----------------------|------------------------------------------------------------------------------|-----------------------------------------------------------|
| Note LEDs 1–8 / 9–12 | Show the current beat / bar                                                  | —                                                         |
| Button A / B         | Open **load mode** for lane A / B                                            | Mute/unmute lane A / B                                    |
| Button C             | Run/stop the internal clock (LED: on = running, slow blink = external clock) | Cycle play direction: forward, reverse, ping-pong, random |
| Encoder turn         | Tempo, 30–300 BPM (internal clock)                                           | Transpose the focus lane ±24                              |
| Encoder click        | Switch focus lane (lit A/B LED)                                              | —                                                         |
| Status LEDs 1/2      | Lane A/B: on = playing, blink = new sequence armed, off = muted              |                                                           |
| Status LEDs 3/4      | Note activity per lane                                                       |                                                           |

## Loading a sequence

1. Press **A** or **B** — its LED blinks, you are in load mode.
2. Enter the sequence number **in binary** on the note buttons: button 1
   = 1, button 2 = 2, button 3 = 4, button 4 = 8, button 5 = 16. The
   LEDs show your entry (example: buttons 3+2+1 lit = sequence #7). If
   the LEDs blink, the number is beyond the bank.
3. Press the **same lane button** (or click the encoder) to confirm.

The new sequence is **armed**, not started: the running phrase always
finishes all 4 bars, then the new one takes over cleanly — the status
LED blinks until then. Press the other lane button to load that lane
instead; C or 8 seconds of inactivity cancels.

## Conductor (I2C)

The module listens on the Lenimal Conductor bus and accepts remote
pattern changes, transpose, scale, direction, length, mute, and reset —
all quantized to the beat, bar, or phrase. Sequence numbers 0–63 address
lane A, 64–127 lane B. The bus instance (0–3) is set from the service
port (`i2c instance <n>`).

---

# Service port

A 115200-baud serial console on the UART header offers every function
plus diagnostics and calibration (`help` lists the commands). The
two-point CV calibration is done once per board: trim the outputs, then
loop each output back into an input and follow the `cal` commands in the
firmware README.

# Firmware

Both personalities and the sequence bank are one firmware image. New
sequence banks are compiled in from a text file — see
`firmware/sequences/sequences.md`; the bank sizes are not fixed. Flash
updates go over SWD; settings and calibration survive updates.
