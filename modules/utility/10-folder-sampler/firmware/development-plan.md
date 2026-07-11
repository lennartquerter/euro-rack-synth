# Folder Sampler — Firmware Development Plan

SD-card sample player: 6 gate inputs trigger 6 WAV voices from a CV/button
selected folder. Firmware in C on **STM32H743VIT6**, generated with
**STM32CubeMX / CubeIDE** (a starter `firmware.ioc` already exists in this
folder), following the structure proven in `1-midi-2-cv` and planned for
`2-quantizer_v2`: untouched CubeMX code + all application logic in
`Core/Src/app/`.

> **Prerequisites:** the hardware fixes in [`../TO_FIX.md`](../TO_FIX.md) —
> especially A1 (MCU VDD), A2 (+5VA), A4 (output DC) — and the missing
> buttons/switch/folder-CV. The plan below assumes the corrected board.

---

## 1. System overview

```
 6× GATE IN ─► NPN inverters ─► PE7–PE12 (EXTI, active LOW)
 2× PITCH CV + pots ─► inverting scalers ─► ADC1 (PB1=INP5, PC4=INP4)
 FOLDER CV (to add) ─► ADC1 (suggest PB0=INP9)   2× BUTTONS ─► PD14/PD15
                          │
                    STM32H743VIT6  (HSE 12.288 MHz)
                          │
 SDMMC1 4-bit (PC8–PC12, PD2) ◄─► microSD (FatFS), card detect PA12
 SAI1 A master: MCLK PE2, FS(LRCK) PE4, SCK PE5, SD PE6 ──► CS4385 SDIN1 (ch1-2)
 SAI2 A sync-slave: SD PD11 ──────────────────────────────► CS4385 SDIN2 (ch3-4)
 SAI3 A sync-slave: SD PD1 ───────────────────────────────► CS4385 SDIN3 (ch5-6)
 SPI1 (PA5 SCK, PA7 MOSI) + PA4 ~CS ──► CS4385 control port · PA3 ──► DAC ~RST
 PB12–PB15 ──► 4 binary folder LEDs · UART4 (PA0/PA1) debug CLI · SWD PA13/14
```

CS4385: 6 of 8 channels used, one shared MCLK/SCLK/LRCK from SAI1, three
stereo I2S data lines. Audio 48kHz/16-bit WAV (accept 44.1k via resampling).

## 2. CubeMX updates needed to the existing `firmware.ioc`

The .ioc already has SAI1/2/3, SDMMC1 (4-bit), SPI1, UART4, ADC1(INP5),
gates PE7–PE12, LEDs PB12–15, buttons PD14/PD15, PA12 input. Still missing:

| Item | Setting |
|---|---|
| PA3 | GPIO output — CS4385 ~RST (currently absent) |
| PC4 | ADC1_INP4, second pitch CV (currently absent) |
| PB0 (or chosen pin) | ADC1_INP9 — folder CV, once hardware adds it |
| Retrigger/one-shot switch | GPIO input on a free pin (PD13/PE13…) |
| PE7–PE12 | switch from plain input to **EXTI falling edge** (gate = LOW) |
| ADC1 | scan 3 channels, continuous + DMA circular, oversampling ×16 |
| SAI1 | Master, I2S, 16-bit in 32-bit slots, MCLK enabled (256×fs), DMA circular |
| SAI2/SAI3 | Sync slave to SAI1 (GCR), same frame config, DMA circular |
| SDMMC1 | IDMA, 4-bit, ~25MHz to start (50MHz once stable) |
| HRTIM, LPTIM1, RTC | remove — leftovers, unused |
| Clocks | see §3 |
| MPU/Cache | I+D cache on; MPU region for DMA buffers (see §5) |

## 3. Clock tree

HSE = 12.288 MHz (audio-rational: 256 × 48 kHz).

- **PLL3 → SAI1/2/3 kernel:** 12.288 /2 ×32 = 196.608 MHz, /16 = 12.288 MHz
  MCLK — bit-exact 48 kHz. (Divider values indicative; let CubeMX solve for
  MCLK = 12.288 MHz exactly — it can, since HSE is already audio-rational.)
- **PLL1 → CPU:** target **200–240 MHz**, not 480 — halves LDO heat
  (TO_FIX.md §C) and is far more than 6 voices of 16-bit mixing needs.
- **PLL2 → SDMMC kernel** (e.g. 200 MHz /4).
- ADC kernel from PER_CK or PLL2P.

## 4. Module layout (`Core/Src/app/`)

| File | Responsibility |
|---|---|
| `app.c` | init sequence, main-loop scheduler (audio is ISR/DMA-driven; main loop does SD streaming, UI, folder logic) |
| `audio_out.c` | SAI1/2/3 + DMA setup, 3× stereo ping-pong buffers, half/complete callbacks calling the mixer |
| `cs4385.c` | SPI control-port driver: release ~RST, set I2S/16-bit, volume, un-mute; register defs |
| `voice.c` | 6 voice state machines: idle/attack-cache/streaming/tail; retrigger vs one-shot; per-voice ring buffer refill requests |
| `mixer.c` | per-voice fetch → (optional resample for ch1-2) → write into the right SAI slot; soft clip/saturate |
| `pitch.c` | 1V/oct CV + pot → playback ratio; linear interpolation first, cubic later |
| `sdcard.c` | SDMMC + FatFS glue, mount/remount on card detect (PA12), error recovery |
| `library.c` | folder scan (0–127), WAV header parse (16-bit PCM, 44.1/48k), per-folder file table, attack-cache preload |
| `gates.c` | EXTI handlers PE7–PE12 (falling edge = gate on), debounce/retrigger window |
| `cv_in.c` | ADC DMA, ×16 oversampling, calibration, folder-CV hysteresis (±½ step so folders don't flutter) |
| `ui.c` | buttons (next/prev folder), binary LEDs PB12–15, mode switch |
| `settings.c` | last folder + calibration in flash/RTC backup |
| `cli.c` | UART4 shell: mount status, folder dump, trigger voice, ADC/pitch dump, timing stats |

## 5. Memory & cache strategy (the H7-specific part)

- **DTCM (128K, no cache, no DMA):** voice state, mixer scratch, hot code data.
- **AXI SRAM (512K):** per-folder **attack cache** — first ~32KB of each of the
  6 samples (192KB) so triggers start with zero SD latency; FatFS work buffers.
- **SRAM1/2 (D2, 288K):** SAI DMA ping-pong buffers and SDMMC IDMA buffers —
  D2 is where SDMMC/SAI DMA masters live; configure an **MPU non-cacheable
  region** over these buffers (simplest correct choice; avoids
  clean/invalidate bugs). Alternative: cacheable + explicit
  `SCB_CleanDCache_ByAddr` before TX / `Invalidate` after RX — only if profiling
  demands it.
- Audio granularity: 48 samples per half-buffer (1ms) per SAI → gate-to-sound
  latency ≈ 1–2ms worst case from the attack cache.

**Streaming model:** trigger → play from attack cache immediately → `voice.c`
posts a stream request → main loop f_reads the remainder in 4–8KB chunks into
that voice's ring buffer ahead of the read pointer. 6 voices × 48kHz × 2B =
576 KB/s worst case — well within 4-bit SDMMC (~10MB/s), but **budget for
SD read-latency spikes** (hundreds of ms on cheap cards): attack cache must be
large enough to cover them, and the streamer must prioritize the emptiest ring.

## 6. Core behavior spec

- **Trigger:** falling edge on PEx (hardware inverts) starts voice x.
  Retrigger mode: restart from 0 on every edge. One-shot: ignore edges while
  playing. Mode switch read once per buffer.
- **Folder select:** buttons step ±1 (0–127, wrap); folder CV (0–8V → 0–127)
  **overrides** buttons when a cable is present if jack detection is added —
  otherwise sum/last-touched, decide during UI phase. On folder change:
  keep playing voices from the old folder to their end; preload the new
  folder's attack caches in the background; LEDs show new folder immediately
  (binary, 4 LEDs = low nibble… **note:** 4 LEDs can only show 0–15 — either
  accept 7 folders-pages, add 3 LEDs, or display bank/page; flag for UI
  decision — README says 0–127 with 4 LEDs, which doesn't fit).
- **Pitch (ch 1-2):** ratio = 2^(CV_volts + pot_offset); resample with linear
  interpolation from the ring buffer; clamp 0.25×–4×.
- **Missing/short files:** a folder may have fewer than 6 WAVs — unmapped
  voices stay silent; CLI reports.
- **Card events:** remove → mute all, LEDs blink; insert → remount, rescan,
  restore folder.

## 7. Development phases

### Phase 0 — Bring-up (after TO_FIX.md fixes)
CubeMX regen with §2 config; SWD attach, UART CLI echo; verify 2.5/3.3/3.3A/5/5A
rails and MCU clocks (MCO out on a test point optional). Deliverable: CLI alive.

### Phase 1 — CS4385 + SAI path (riskiest first)
`cs4385.c` + `audio_out.c`: release reset, configure via SPI, output a
generated sine on all 6 channels from flash — no SD involved.
Scope AOUTs and the op-amp outputs; verify MCLK = 12.288MHz exactly, no
clicks at buffer boundaries, all three SAIs sample-locked.
Deliverable: 6-channel test tone.

### Phase 2 — SD + FatFS
`sdcard.c` + `library.c`: mount, scan folders, parse WAV headers, CLI `ls`;
sustained-read benchmark (CLI prints MB/s and max latency) with the actual
target card. Deliverable: folder table + read performance numbers.

### Phase 3 — One streaming voice
`voice.c`/`mixer.c` minimal: gate 1 plays 1.wav to completion, attack cache +
ring streaming, starvation counters on CLI. Torture: retrigger at 50Hz,
long files, folder of max-size files. Deliverable: reliable single voice.

### Phase 4 — Six voices + modes
All gates via EXTI, 6 independent voices, retrigger/one-shot, voice-starved
LED/CLI diagnostics. Deliverable: full polyphonic playback.

### Phase 5 — Pitch + folder CV
ADC pipeline, calibration (CLI-guided, stored), `pitch.c` resampling on ch1-2,
folder CV with hysteresis. Deliverable: 1V/oct tracking over 0–8V (once the
CV range fix from TO_FIX.md §B is in), stable folder selection.

### Phase 6 — UI + persistence + polish
Buttons, LED display decision (0–127 vs 4 LEDs), settings persistence, card
hot-swap handling, IWDG watchdog, startup animation, README update with real
specs, `.elf`/`.bin` in `compiled/`.

## 8. Risks / open points

- **4 LEDs cannot display 0–127** (README conflict) — resolve in Phase 6 at
  the latest; cheapest hardware-free answer is 16 folders, or LEDs show
  folder mod 16.
- **SD latency spikes** dominate the streaming design — measure early
  (Phase 2) with the real card; size attack caches from data, not hope.
- **Three-SAI sync:** SAI2/3 must be configured as sync-slaves via SAI_GCR and
  started before SAI1's master enable so all SDINs align to the same LRCK —
  verify channel alignment with a per-channel test tone (Phase 1).
- **Cache coherency** is the classic H7 footgun — take the MPU non-cacheable
  route first (§5).
- CPU clock vs LDO heat: stay ≤240MHz until the power architecture decision
  (TO_FIX.md §C) is made.
- 44.1k files on a 48k output need resampling even on non-pitch channels
  (ratio 0.919) — decide whether to support or reject at scan time (Phase 2).

## 9. Tooling

Same as midi-2-cv/quantizer: STM32CubeMX ≥ 6.x, STM32CubeIDE or CLion+CMake,
ST-Link/OpenOCD (H7 target config), `arm-none-eabi-gcc`; host `gcc` unit tests
for `library.c` WAV parsing and `mixer.c`/`pitch.c` math. Middleware: FatFS
(CubeMX bundled) with exFAT off, LFN on (folder names are numeric, but be
tolerant); no RTOS — superloop + ISRs is sufficient and easier to reason about
for audio timing.
