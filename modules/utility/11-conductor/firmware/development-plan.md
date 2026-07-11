# Ordo — Firmware Development Plan

Clock-slaved, SD-driven conductor dispatching musical events over I²C.
C on **STM32H7B0VBT6** (280MHz, 128KB flash, 1.4MB SRAM), STM32CubeMX /
CubeIDE, application code in `Core/Src/app/` (midi-2-cv convention).

Companion docs — this plan does not duplicate them:
- [`cubemx-setup.md`](cubemx-setup.md) — complete CubeMX walkthrough (pins,
  clocks, peripheral params). Apply the **EXTI8 fix** (SD_CD → PE0/PE1) there
  and in the schematic.
- [`../protocol.md`](../protocol.md) — I²C frame format, addressing, QUANT
  semantics, double-send/SEQ rule.
- [`../score.md`](../score.md) — `.lscore` text format.
- [`../TO_FIX.md`](../TO_FIX.md) — schematic gaps; firmware assumes the
  completed board (gates possibly inverted by driver stage — confirm at
  bring-up and encode as `GATE_ACTIVE_LEVEL`).

---

## 1. Architecture at a glance

```
TIM2 IC (CLK edge) ──► clock engine (musical time: tick/beat/bar/phrase)
                            │ lookahead
score (RAM) ──► scheduler ──┼──► i2c dispatcher (I2C2, queue, send ×2, SEQ)
                            ├──► cv engine (MCP4728 on I2C1, ramps/LFOs)
                            └──► gate engine (PE2–PE5, pulse/hold)
SD (SDMMC1+FatFS) ──► parser ──► score in RAM        UI: OLED(SPI1)+enc+4 btns
```

Two planes, strictly separated:
- **Real-time plane (ISRs):** TIM2 capture, scheduler tick, gate edges,
  time-critical I²C queue pump. No malloc, no FatFS, no OLED.
- **UI plane (main loop):** SD, parser, OLED rendering, encoder/buttons,
  snapshot writes.

## 2. Module layout (`Core/Src/app/`)

| File | Responsibility |
|---|---|
| `app.c` | init, main loop, mode state machine (BOOT→IDLE→PLAYING⇄HOLD, JUMP/CUE/PANIC overlays) |
| `clock_engine.c` | TIM2 capture ISR: inter-edge period estimate (median-of-5 + smoothing), 32-bit rollover handling (~15s @280MHz), clock-stopped timeout, RST→bar 0; publishes `musical_now` (tick within bar/phrase) and predicts boundary times |
| `scheduler.c` | walks the active section's sorted event list; converts event times (bar/beat/offset) to predicted MCU time; fires events **lead-time ms early** (protocol: slaves need time to arm); owns HOLD/JUMP/CUE semantics |
| `i2c_bus.c` | I2C2 master TX with DMA + queue; frame builder (ADDR/CMD/LEN/PAYLOAD/SEQ/CHK per protocol.md); double-send ~3ms apart; per-command SEQ counters; bus-error recovery (re-init, log, never block) |
| `cv_engine.c` | MCP4728 driver (I2C1, fast-write); automation primitives: set, ramp, LFO; 1kHz update tick; volts↔code calibration (±5V stage) |
| `gates.c` | gate set/pulse with ms durations, polarity constant |
| `score_parser.c` | `.lscore` → in-RAM structures (@meta/@aliases/@section/events, quant tags, offsets); line-based, tolerant; host-testable (no HAL includes) |
| `library.c` | SD mount (FatFS), enumerate `*.lscore`, load selected set, snapshot file read/write (resume-after-crash: current section + params, per README) |
| `ui.c` | set picker, playing view (section, bar:beat, next event), JUMP navigator, CUE picker; button/LED logic |
| `oled.c` | SH1106/SSD1309 SPI driver (2.42″), framebuffer + dirty-rect flush from main loop only |
| `encoder.c` | TIM1 encoder delta + push |
| `cli.c` | USART3 shell: dump score, fire event, I²C sniff/log, clock stats (BPM, jitter), calibration |
| `panic.c` | broadcast mute-all (0xFC) repeatedly, zero CVs, gates off, return to idle — reachable from any state (long-press combo) |

## 3. Timing design (the core of this module)

- **Musical time base:** count CLK edges (e.g. 4 PPQN — decide and document;
  score BPM is informative, real tempo comes from measured clock). Beat =
  N edges, bar = beats_per_bar, phrase = bars_per_phrase from `@meta`.
- **Prediction:** next-boundary MCU time = last edge time + k×smoothed period.
  Scheduler wakes (TIM/SysTick compare) at `boundary − lead_time` (default
  ~5ms) to emit quantized I²C so slaves arm in time; `QUANT_NOW` events skip
  the queue.
- **Jitter budget:** capture in hardware (TIM2 IC, filter=4) → edge timestamp
  jitter ≪100µs. All musical decisions run off timestamps, not ISR latency.
- **Clock stop:** no edge for >2× expected period → freeze musical time,
  UI shows STALLED; resume cleanly on next edge. RST edge: reset counters to
  bar 0 of current section (define: does RST also un-hold? — decide in Phase 4).
- **HOLD:** section pointer frozen, clock keeps counting, loops current bar
  (re-fire looped events); **JUMP:** target section applied at next bar;
  **CUE:** selected `@cue` event fires at its quant boundary.

## 4. Memory & flash budget (H7B0VB = 128KB flash!)

- Flash is the scarce resource: HAL + FatFS + app must fit 128KB. Use `-Os`,
  LTO, trim unused HAL modules, ONE small font + one medium font (no font
  zoo), no printf-float in release. Track headroom in CI from Phase 1.
  Fallback if it gets tight: move fonts/strings to SD, or tiny-FatFS config.
- RAM is abundant (1.4MB): whole parsed set lives in AXI SRAM; OLED
  framebuffer (2.42″ 128×64 = 1KB) trivial; generous I²C/CLI queues fine.

## 5. Development phases

### Phase 0 — Skeleton (needs only MCU core + SWD, board incomplete OK)
CubeMX per `cubemx-setup.md` (with EXTI8 fix), CLI on USART3, blink, flash
budget baseline. Deliverable: shell prompt + size report.

### Phase 1 — Clock engine
CLK jack → TIM2 capture; CLI prints BPM/jitter/rollover correctness against a
signal generator and the 9-master-clock module. RST handling. Deliverable:
stable musical time readout, measured jitter number.

### Phase 2 — I²C dispatcher
Frame builder + queue + double-send + SEQ; CLI `send` command; logic-analyzer
validation of timing lead. Bench slave: any spare Nano/STM32 acking address
0x08. Deliverable: protocol.md v0.1 on the wire, verified.

### Phase 3 — Score pipeline
`score_parser.c` host-side first (unit tests with score.md's worked example),
then on target: SD mount, set list, load to RAM. Deliverable: parse the
3-minute techno example bit-perfectly on host and target.

### Phase 4 — Scheduler + transport
Events fire at boundaries from a live clock; RUN/HOLD/JUMP semantics; CLI
event trace with timestamps to prove <1ms boundary accuracy. Deliverable:
full set plays end-to-end against bench slave.

### Phase 5 — CV + gates
MCP4728 driver + ±5V calibration (CLI-guided, stored on SD), ramps/LFOs at
1kHz; gate pulses (scope-verified polarity + 1ms rise per README).
Deliverable: `conductor.cv.*`/`conductor.gate.*` score targets work.

### Phase 6 — UI + resilience
OLED views, encoder navigation, CUE picker, panic combo, snapshot/resume,
SD hot-remove behavior, IWDG watchdog. Deliverable: performable front panel,
no CLI needed.

### Phase 7 — Integration
Retrofit first slave (quantizer per README build-status), end-to-end
5-minute test set, then the 30-minute set. Measure: boundary accuracy at
slave, bus errors/hour, flash/RAM headroom. Update README build-status boxes.

## 6. Risks / open points

- **128KB flash** — the #1 constraint; watch from day one (§4).
- **PPQN decision** (clock edges per beat) shapes everything in
  `clock_engine.c` — fix it in Phase 1 and write it into protocol.md.
- **I2C1 vs MCP4728 LDAC:** per-channel synchronous update needs LDAC
  (PB9 per cubemx doc) — wire it in the schematic (TO_FIX.md block 5).
- **Gate polarity** unknown until output stage is designed — keep
  `GATE_ACTIVE_LEVEL` per channel.
- **Tempo drift vs prediction:** heavy swing or tempo ramps from the master
  clock stress the boundary predictor — median+EMA smoothing first, revisit
  with real measurements (Phase 4 trace data).
- **USB feature** is unscoped (mass-storage for score upload would be ideal
  but costs flash) — decide before Phase 6; dropping it frees PA11/PA12 and
  code space.
- Snapshot write frequency vs SD wear/latency during performance — write on
  section change only, never on the RT plane.

## 7. Tooling

STM32CubeMX ≥6.x per `cubemx-setup.md`, STM32CubeIDE or CLion+CMake,
ST-Link/OpenOCD; host `gcc` + unit tests for `score_parser.c`, frame builder,
and clock math (all HAL-free by design); logic analyzer for Phase 2/4 timing
proof. No RTOS — two-plane superloop keeps the RT path auditable.
