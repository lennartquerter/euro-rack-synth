# Quantizer v2 — Firmware Development Plan

Dual-channel CV quantizer for Eurorack. Firmware in C on **STM32F411RET6**,
generated with **STM32CubeMX / CubeIDE**, following the structure proven in
`modules/utility/1-midi-2-cv/firmware` (CubeMX `.ioc` + `Core/Src/app`
application layer, CMake/CubeIDE dual build).

> **Prerequisite:** the three hardware issues in [`../TO_FIX.md`](../TO_FIX.md)
> must be fixed first (rev B or bodges). This plan assumes the corrected board:
> PB4 = DAC ~CS (software CS), ~LDAC tied to GND.

---

## 1. System overview

```
 CV IN A/B ──► TL072 (inv, gain -0.25, bias 1.815V) ──► MCP6002 buffer ──► ADC1 (PC2/PC3)
 TRIG IN A/B ─► NPN inverter ──► PC4/PC5        jack-detect (normalled) ──► PC0/PC1
                                   │
                              STM32F411RET6
                                   │
 SPI1 (PB3/PB5, PB4=~CS) ──► MCP4922 ──► OPA2277 ×~4 ──► CV OUT A/B (0–10V)
 PB0/PB1 ──► NPN ──► TRIG OUT A/B (~8V, inverted)
 SPI2 (PB10 SCK, PB14 MISO, PB13 latch) ──► 2× 74HC165 ──► 12 note switches
 SPI3 (PC10 SCK, PC12 MOSI, PC11 latch) ──► 3× 74HC595 ──► 16 switch LEDs + 4 status LEDs
 TIM3 encoder (PA6/PA7), encoder switch PA5, buttons A–D PC6–PC9
 USART1 (PA9/PA10) debug/CLI · I2C1 (PB6/PB7) expansion header
```

UI: 12 illuminated note buttons in a chromatic circle around the encoder,
4 function buttons (2 per channel, bottom corners), 4 status LEDs.

## 2. Pin map (CubeMX configuration)

| Pin | Signal | CubeMX setting |
|---|---|---|
| PH0/PH1 | HSE 20 MHz crystal | HSE crystal/ceramic; PLL → **SYSCLK 96 MHz** (20/10 ×96 /2) |
| PC2, PC3 | CV_IN A / B | ADC1_IN12 / IN13, scan, 480-cycle sampling |
| PB3, PB5 | DAC SCK / SDI | SPI1, master TX-only, mode 0,0, ≤20 MHz |
| PB4 | DAC ~CS | GPIO output, high idle (after TO_FIX #1) |
| PB10, PB14 | SW SCK / MISO | SPI2, master RX-only, mode 0,0 |
| PB13 | SW ~PL (165 load) | GPIO output, high idle |
| PC10, PC12 | LED SCK / MOSI | SPI3, master TX-only, mode 0,0 |
| PC11 | LED RCLK (595 latch) | GPIO output, low idle, pulse high |
| PA6, PA7 | Encoder A / B | TIM3 CH1/CH2, encoder mode TI1+TI2 |
| PA5 | Encoder switch | GPIO input (external 10K pull-up), active low |
| PC6–PC9 | Buttons A–D | GPIO input (external pull-ups), active low |
| PC4, PC5 | TRIG_A_IN / TRIG_B_IN | GPIO input or EXTI, **inverted** (low = gate high) |
| PC0, PC1 | Jack detect A / B | GPIO input (high = cable inserted) |
| PB0, PB1 | TRIG_A_OUT / TRIG_B_OUT | GPIO output, **high = jack output low** (inverted) |
| PA9/PA10 | USART1 TX/RX | 115200 8N1, debug CLI |
| PB6/PB7 | I2C1 SCL/SDA | reserved, expansion header (no on-board pull-ups) |
| PA13/PA14 | SWDIO/SWCLK | Serial Wire debug |

Note: PB3/PB4 default to JTAG functions — CubeMX must set debug to
*Serial Wire* only so they are free for SPI1/~CS.

## 3. Analog scaling (constants for `quantize.c` / calibration)

- **ADC input transfer:** `Vadc = 1.815V·1.25 − 0.25·Vin` → `Vin = 5·Vbias − 4·Vadc`
  (inverting stage). Usable input ≈ **−4V … +9V**; clamps beyond that. 12-bit ADC,
  ~14.6 mV of input per LSB → oversample ×16 and average for pitch stability
  (1 semitone = 83.3 mV).
- **DAC output transfer:** MCP4922, VREF = 2.5V, gain ×1 → output stage gain ≈ 4.04
  (trimmed via RV1/RV2 to 10.000V full scale) → **0–10V**, ~2.44 mV per code,
  ~34 codes per semitone. 1V/oct, 10 octave range.
- Calibration data (per channel): ADC offset/slope, DAC code per volt. Two-point
  measurement (e.g. 1V and 5V) stored in flash.

## 4. Project setup

1. New CubeMX project, `STM32F411RETx`, name `quantizer-v2`, toolchain
   *STM32CubeIDE* (same as midi-2-cv; keep the CMakeLists template trick for
   CLion builds).
2. Configure clocks + peripherals per §2. Enable TIM (e.g. TIM2) as a 1 kHz
   scheduler tick if not using SysTick callbacks directly.
3. Keep generated code untouched; all application code lives in
   `Core/Src/app/` + `Core/Inc/app/` (midi-2-cv convention), called from the
   `USER CODE` sections of `main.c`.

### Proposed module layout (`Core/Src/app/`)

| File | Responsibility |
|---|---|
| `app.c` | init + main loop scheduler (1 kHz UI tick, fast quantize path) |
| `cv_in.c` | ADC oversampling/averaging, volts conversion, calibration applied |
| `dac.c` | MCP4922 driver: `dac_write(ch, code)` — PB4 ~CS framing, 16-bit word (channel, buffered, gain=1, active bits) |
| `quantize.c` | note-mask quantization core, hysteresis, volts↔note helpers |
| `triggers.c` | trig-in edge detect (inverted), jack detect, trig-out pulse timing (inverted) |
| `switches.c` | 74HC165 read via SPI2 (~PL pulse then 16-bit read), debounce, edge events |
| `leds.c` | 74HC595 frame buffer (24 bits), blink/brightness-by-blink patterns, RCLK latch |
| `encoder.c` | TIM3 encoder delta, encoder switch, acceleration |
| `ui.c` | modes, note-mask editing, scale presets, channel select, status LEDs |
| `settings.c` | flash persistence (last sector), wear-levelled record append |
| `calibrate.c` | guided calibration routine over UART/UI |
| `cli.c` | USART1 debug shell (print ADC/DAC values, force notes, dump settings) |

## 5. Core behavior spec

- **Channels:** 2 independent (A/B). CV in A normals to B's input jack switch
  (hardware) — firmware needs no special handling.
- **Quantization:** per-channel 12-bit note mask (which semitones are allowed).
  Continuous mode: input tracked at ~2 kHz, output updated when the quantized
  note changes, with ±¼ semitone hysteresis to prevent boundary flutter.
  Triggered mode (auto-selected when a cable is in the trig-in jack, PC0/PC1):
  sample-and-hold on trigger rising edge.
- **Trigger outputs:** 5–10 ms pulse whenever the channel's output note changes
  (both modes). **The output stage inverts:** with Q1/Q2 off, the 1K/2K divider
  holds the jack at ~8V; GPIO high turns Q on and pulls the jack to 0V. So idle
  = GPIO **high** (jack 0V), pulse = GPIO **low** for the pulse width (jack 8V).
  Encode as `TRIG_ACTIVE_LEVEL` in `triggers.c` and confirm on scope at Phase 5.
- **Note buttons:** toggle semitone in the active channel's mask; LED shows state.
  Encoder: root/transpose (turn), channel select or menu (push). Buttons A–D:
  channel A/B select + scale preset page / shift functions (final mapping decided
  during UI phase). Status LEDs D5–D8: channel indicators, trig activity.
- **Scale presets:** chromatic, major, natural/harmonic minor, pentatonic maj/min,
  blues, dorian, mixolydian, whole tone, octaves, fifths — selectable via
  encoder; custom masks saved to flash.
- **Persistence:** masks, root, mode, calibration → flash sector 7 (last 128 KB),
  simple append-record scheme with CRC; load on boot, save on change (debounced
  ~2 s after last edit).

## 6. Development phases

Each phase ends with something testable on hardware.

### Phase 0 — Board bring-up (blocked by TO_FIX.md fixes)
- CubeMX project, clocks, SWD attach, blink status LED via 595 chain not yet —
  use UART "hello" + SWD as first sign of life.
- Verify 3.3V / 3.3VA / 2.5V ref / ±12V rails on test points **before** MCU risk.
- Deliverable: UART CLI echo at 115200.

### Phase 1 — Output path (the risky path first)
- `dac.c`: SPI1 + PB4 ~CS driver; CLI command `dac <ch> <code>`.
- Scope DAC_A/B test points, then op-amp outputs; trim RV1/RV2 to 10.000V at
  code 4095. Verify both channels, check for missing-code behavior.
- Deliverable: CLI-driven voltage source, trimmed.

### Phase 2 — Input path
- `cv_in.c`: ADC1 scan + DMA circular, ×16 oversampling, CLI `adc` dump.
- Feed known voltages (use the module's own trimmed outputs looped back),
  implement two-point calibration, store in flash (`settings.c` minimal).
- Deliverable: CLI prints input volts within ±5 mV across −4…+9V.

### Phase 3 — Quantizer core
- `quantize.c` + loopback test: CV out follows CV in, chromatic mask,
  hysteresis verified with slow ramp (no note flutter at boundaries).
- Unit-test the mask/hysteresis logic host-side (plain C, no HAL) — same
  approach as midi-2-cv app-layer separation allows.
- Deliverable: chromatic tracking A and B, ≤1 ms input→output latency.

### Phase 4 — Control surface
- `switches.c` (165 chain), `leds.c` (595 chain), `encoder.c` (TIM3),
  debounce + event queue; CLI dump of all controls.
- Deliverable: pressing any button lights its LED; encoder count on CLI.

### Phase 5 — Triggers & modes
- `triggers.c`: edge detection on PC4/PC5 (mind inversion), jack detect on
  PC0/PC1, trig-out pulses (mind inversion, confirm on scope).
- Triggered sample-and-hold mode; auto mode switch by jack detect.
- Deliverable: clocked quantization from an external sequencer clock.

### Phase 6 — UI & presets
- `ui.c`: note-mask editing, scale presets, root/transpose on encoder,
  channel select, status LEDs; full `settings.c` persistence.
- Deliverable: usable instrument without CLI.

### Phase 7 — Polish & release
- Guided calibration via UI, settings versioning, watchdog (IWDG),
  brown-out level, `.elf`/`.bin` in `compiled/`, README with panel controls
  (mirror midi-2-cv README style), consumption measurement for the spec table.

## 7. Risks / open points

- **Trigger-out polarity** (§5) — confirm on scope at Phase 5 before trusting it.
- ADC bias network sits on the digital 3.3V rail (see TO_FIX.md notes): if noise
  shows up in pitch, increase oversampling and/or sync ADC bursts away from LED
  SPI traffic.
- 20 MHz HSE is non-standard for USB-friendly clocks — irrelevant here (no USB),
  96 MHz SYSCLK works cleanly (20 → PLLM=10 → 2 MHz × 96 → /2).
- I2C1 header has no pull-ups on board — any future expansion board must carry
  its own.

## 8. Tooling (same as midi-2-cv)

- STM32CubeMX ≥ 6.x, STM32CubeIDE (or CLion + CMake via `CMakeLists_template.txt`)
- ST-Link + OpenOCD (`stm32-custom.cfg` from midi-2-cv works — same MCU)
- `arm-none-eabi-gcc`, host `gcc` for unit tests of `quantize.c`
