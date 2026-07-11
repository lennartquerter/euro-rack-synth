# Quantizer v2 — Firmware

Dual-channel CV quantizer firmware for the STM32F411RET6 brain board.
Structure mirrors `modules/utility/1-midi-2-cv/firmware`: CubeMX `.ioc`,
generated code in `Core/`, all application logic in `Core/Src/app` +
`Core/Inc/app`, hooked in through the `USER CODE` sections of `main.c`.

See [development-plan.md](development-plan.md) for the full plan and
[../TO_FIX.md](../TO_FIX.md) for the **required hardware fixes** — this
firmware assumes the corrected board (PB4 = DAC ~CS, ~LDAC to GND, base
resistors on the trigger transistors).

The same hardware will also host a step-sequencer personality (flipped
faceplate) in this firmware image — see
[sequencer-development-plan.md](sequencer-development-plan.md).

## Status

| Phase | | State |
|---|---|---|
| 0 | Bring-up: clocks, UART CLI | ✅ implemented |
| 1 | Output path: MCP4922 driver, `dac` CLI command | ✅ implemented |
| 2 | Input path: ADC DMA + oversampling, two-point calibration, flash settings | ✅ implemented |
| 3 | Quantizer core: note mask + hysteresis, host unit tests | ✅ implemented |
| 4 | Control surface: 74HC165 switches, 74HC595 LEDs, encoder | ✅ implemented |
| 5 | Triggers & modes (sample-and-hold, jack detect) | ✅ implemented |
| 6 | UI & presets (`ui_quant.c`) | ✅ implemented |
| 7 | Polish & release (guided calibration, IWDG, `compiled/`) | ⬜ |

Sequencer personality (see
[sequencer-development-plan.md](sequencer-development-plan.md)):

| Phase | | State |
|---|---|---|
| S0 | Personality plumbing: PB2 strap, boot combo, `mode` CLI | ✅ implemented |
| S1 | Sequence toolchain: `sequences.md` → `gen_sequences.py` → bank (32+32) | ✅ implemented |
| S2 | Engine core (`seq.c`, host-tested: transport, arming, directions) | ✅ implemented |
| S3 | Clocking & outputs (internal BPM clock, external 4 PPQN, DAC/gates) | ✅ implemented |
| S4 | Panel UI (`ui_seq.c`: binary load flow, shift layer) | ✅ implemented |
| S5 | Conductor I2C slave (`i2c_slave.c`) | ✅ implemented |
| S6 | Polish | ⬜ (bench work) |

The firmware is **one image with two personalities**: PB2 strap low =
quantizer, high = sequencer; `mode quantizer|sequencer|auto` overrides it
from the CLI, and holding the encoder switch during power-up flips the
override from the panel (note LEDs: left half = quantizer, right half =
sequencer). The sequencer currently boots to a stub that reports its
compiled-in sequence bank. Regenerate the bank after editing
`sequences/sequences.md` with `python3 tools/gen_sequences.py`.

Everything is written but **untested on hardware** — Phase deliverables
still need to be verified board-by-board (see bring-up notes below).

## Building

1. Open `quantizer-v2.ioc` in **STM32CubeMX ≥ 6.x** and *Generate Code*
   (toolchain STM32CubeIDE). This drops in `Drivers/` (HAL + CMSIS) and the
   CubeIDE project files; `main.c` user sections and `Core/*/app` are
   preserved (`KeepUserCode` is set).
2. Build with **STM32CubeIDE**, or with **CLion/CMake** via the committed
   `CMakeLists.txt` (regenerated from `CMakeLists_template.txt`, same trick
   as midi-2-cv).
3. Flash over ST-Link: `stm32-custom.cfg` (OpenOCD) is copied from
   midi-2-cv — same MCU, works unchanged.

Host-side unit tests for the quantizer core (no HAL needed):

```sh
cd test && make        # 33 assertions, plain C
```

## Hardware map (from the .ioc)

- **SYSCLK 96 MHz** from the 20 MHz HSE (PLLM 10, PLLN 96, PLLP /2), APB1 48 MHz.
- **ADC1** scan PC2/PC3 (CV in A/B), 480-cycle sampling, circular DMA,
  ×16 oversampling in `cv_in.c`.
- **SPI1** (PB3/PB5, 12 MHz) + PB4 software ~CS → MCP4922 DAC (`dac.c`).
- **SPI2** RX-only (PB10/PB14, 3 MHz) + PB13 ~PL → 2× 74HC165 note switches.
- **SPI3** TX-only (PC10/PC12, 3 MHz) + PC11 RCLK → 3× 74HC595, 20 LEDs.
- **TIM3** encoder mode on PA6/PA7, encoder push on PA5; buttons A–D PC6–PC9.
- **TIM2** = 1 kHz scheduler tick; the quantize path free-runs in the main
  loop (≪1 ms latency), UI work happens on the tick.
- **USART1** PA9/PA10, 115200 8N1 debug CLI.
- **PB2 = MODE strap** (personality select, read once at boot; PB2 is also
  BOOT1 — strap through ~10K, see ../TO_FIX.md).
- **PB0/PB1** trigger outs (idle **high** = jack 0 V — output stage inverts),
  PC4/PC5 trigger ins (inverted), PC0/PC1 jack detect — handled in
  `triggers.c`. Both inversions are encoded as defines in `triggers.h`;
  **confirm the trig-out polarity on a scope** (plan §7) — `trig a` fires a
  manual pulse for exactly that.

## CLI (115200 8N1)

```
status                overview of both channels
adc                   averaged ADC codes + input volts
dac <a|b> <code|off>  force a raw DAC code / hand back to quantizer
mask <a|b> <hex>      12-bit note mask, bit0 = C
scale <a|b> <name>    chrom major minor harm pentmaj pentmin blues
                      dorian mixo whole oct fifths
root <a|b> <0-11>     rotate the mask
trans <a|b> <n>       transpose output ±24 semitones
trig <a|b>            fire a manual trigger-out pulse (scope check)
ctl                   dump switches/buttons/encoder
led test on|off       take LED control, then: led <0-19> on|off|slow|fast
cal show|reset        calibration constants / drop pending points
cal adc <a|b> <mv>    capture a point at a known input (run twice)
cal dac <a|b> <code> <mv>  capture a measured output point (run twice)
mode [quantizer|sequencer|auto]  personality override (then 'reset')
reset                 reboot
save / load / defaults / ver / help

sequencer personality:
bpm [30-300]          internal clock tempo (also: encoder turn)
run / stop            internal clock transport (external clock overrides)
seq status            engine state
seq load <a|b> <n> [now]  arm a sequence (default: next phrase boundary)
seq reset             restart the phrase on the next pulse (also: trig-in B)
i2c [instance <0-3>]  conductor bus address + frame stats
```

### Calibration procedure (Phase 1 + 2)

1. **DAC first**: `dac a 500`, measure the CV OUT A jack, then
   `cal dac a 500 <measured mV>`. Repeat with `dac a 3500`. Trim RV1/RV2 to
   10.000 V at `dac a 4095` *before* calibrating.
2. **ADC via loopback**: patch CV OUT A → CV IN A, use the now-trusted DAC
   to apply two known voltages (e.g. 1 V and 5 V), `cal adc a <mv>` after
   each. Repeat for channel B.
3. `save` (also autosaves ~2 s after any edit). Settings live in flash
   sector 7 as an append-only CRC'd record chain (`settings.c`).

## Panel controls — quantizer personality (`ui_quant.c`)

| Control | Action |
|---|---|
| Note buttons | Toggle that semitone in the active channel's mask; LEDs show the allowed notes |
| Button **A** / **B** | Select channel A / B (button LED + status LED mirror it) |
| Button **C** | Scale browse: encoder steps through the 12 presets + your original custom mask (slot before `chrom`), applied live; C / encoder click / 8 s timeout exits, LED C blinks while browsing |
| Button **D** (hold) | Shift: the root note's LED blinks; **D + note button** sets the root to that note |
| Encoder turn | Transpose the active channel ±24 semitones (with acceleration) |
| Encoder click | Toggle the active channel (exits scale browse first) |
| Status LEDs 1/2 | Channel A/B indicator |
| Status LEDs 3/4 | Pulse when a channel's output note changes |

All edits autosave to flash ~2 s after the last change.

**Modes** (per channel, automatic): continuous tracking normally; a cable
in the trig-in jack (jack detect, 5 ms debounced) switches to
sample-and-hold — the input is quantized only on a trigger rising edge.
Trigger out fires a ~8 ms pulse whenever the output note changes, in
both modes.

## Panel controls — sequencer personality (`ui_seq.c`)

| Control | Play mode | Shift (hold D) |
|---|---|---|
| Note LEDs 1–8 / 9–12 | beat chase / bar within the phrase | — |
| Button **A** / **B** | enter **load mode** for lane A / B | mute/unmute lane A / B |
| Button **C** | run/stop (internal clock) or mute the focus lane (external clock); LED: on = running, slow blink = external clock | cycle direction (fwd/rev/ping-pong/random) |
| Button **D** | (hold) shift | — |
| Encoder turn | BPM 30–300 (internal clock) | transpose the focus lane ±24 |
| Encoder click | toggle focus lane (A/B LED marks it) | — |
| Status LEDs 1/2 | lane A/B: on = playing, fast blink = load armed, off = muted | |
| Status LEDs 3/4 | lane A/B note onsets | |

**Load mode** (press A or B — its LED blinks): enter the sequence number
in binary on the note buttons — button *n* is worth 2^(n−1), so pressing
buttons 3, 2, 1 selects #7. The LEDs show the value live and blink when
it exceeds the bank. Confirm with the same lane button or an encoder
click — the sequence is **armed** and starts at the next phrase boundary
(the current 4 bars always finish). The other lane button switches
target; C or 8 s of inactivity cancels. The number of value buttons
follows the bank size (5 buttons for the stock 32-per-bank).

**Jacks** (flipped panel): trig-in A = clock (4 PPQN, auto-detected),
trig-in B = reset, CV outs = lane pitch, trig outs = lane gates (ties
play legato, retriggers drop the gate for 3 ms).

## Bring-up notes (adjust here, not in the UI code)

- `switches.c` `chain_map[]` — 74HC165 bit → note button order, plus
  `SWITCHES_*_ACTIVE_LOW` defines.
- `leds.c` `led_map[]` — 74HC595 bit → LED order, `LEDS_ACTIVE_HIGH`.
- `encoder.h` `ENCODER_COUNTS_PER_DETENT` (4 assumed).
- Defaults for the analog transfer live in `settings.c`
  (`CAL_DEFAULT_*`) — two-point calibration overrides them.
