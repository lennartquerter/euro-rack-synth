(AI generated content)

# Ordo

*Conductor module for the Lenimal Eurorack system.*

> *Ordinem rerum* — the order of things.

Ordo is a clock-slaved, SD-card-driven master controller that dispatches
musical events to a small constellation of Lenimal slave modules over an
I²C bus. It is the brain of a live-performable modular system: a 30-minute
set is composed in plain text on a computer, copied to an SD card, and
performed from the front panel with a handful of recovery controls.

Ordo does not generate clock and does not produce audio. It receives a
hardware clock and reset, counts musical time, and emits I²C messages,
CV, and gates at exactly the right moments. Everything else in the rack
makes the sound.

## Companion documents

- `protocol.md` — I²C protocol, addressing scheme, command tables, timing semantics
- `score.md` — score file format with a worked 3-minute techno example

## Aesthetic

Ordo · Vitruvian geometry. Cream plate, dark engraved silkscreen,
Cormorant Garamond throughout. The panel is dominated by an inscribed
square and circle with radiating proportion lines and a 12-position
Roman numeral ring (clock face / astrolabe). The wordmark sits at the
top of the panel in spaced serif caps with a Latin motto beneath.
Brand footer reads `— mmxxvi —` to match the other Lenimal
modules.

## Form factor

| Property            | Value                                        |
|---------------------|----------------------------------------------|
| Width               | 16 HP (81.28 mm)                             |
| Height              | 3U (128.5 mm)                                |
| Depth               | TBD — target ≤ 30 mm behind panel            |
| PCB constraint zone | 110 mm tall (vertical), full panel width     |
| Mounting            | Standard Eurorack, 4× M3, slotted top/bottom |
| Power               | Eurorack 16-pin, +12 V / -12 V / +5 V        |

## Front panel I/O

### Inputs (left of divider)

- **CLK** — clock input, 3.5 mm, Schmitt-trigger conditioned, expects ~5 V gate
- **RST** — reset input, 3.5 mm, rising-edge triggered

### Outputs (right of divider)

- **G1–G4** — 4× gate outputs, 3.5 mm, 0/+5 V, ~1 ms rise time
- **CV1–CV4** — 4× control voltage outputs, 3.5 mm, 12-bit DAC, ±5 V

### Controls

- **CURSOR** — main encoder with push (centered in Vitruvian circle)
- **RUN / HOLD / JUMP / CUE** — 4× tactile buttons with LEDs

### Display

- **OLED** — 1.3″ or 2.42″ monochrome, SPI-driven, recessed flush with plate

### Storage

- **SD card** — front-loading push-push slot, FAT32, scores read on boot

## Back panel I/O

- **I²C bus** — 3-pin shrouded header, 2.54 mm pitch, pinout `GND / SCL / SDA`
    - Single header on the conductor
    - 4k7 pull-ups on conductor only
    - **3V3 not exposed** — Lenimal slaves are individually powered

## System architecture

```
        ┌──────────────┐    hardware clock (jack)
        │   MASTER     ├──────────────┐
        │   CLOCK      │              │
        └──────────────┘              ▼
                                ┌────────────┐
                                │    ORDO    │
                                │ (conductor)│
                                └─────┬──────┘
                                      │ I²C bus
                ┌────────────┬────────┼────────┬────────────┐
                ▼            ▼        ▼        ▼            ▼
            ┌───────┐   ┌───────┐ ┌──────┐ ┌────────┐  ┌────────┐
            │ Seq.  │   │ Quant.│ │Drums │ │Sampler │  │(future)│
            └───────┘   └───────┘ └──────┘ └────────┘  └────────┘
                  Audio + CV/gate fan out to the rest of the rack
```

- Clock and reset are dedicated hardware lines, not transmitted over I²C
- All I²C traffic is unidirectional, conductor → slaves
- Slaves do not report back; errors are silent and the next message resyncs
- Every event carries a quantization mode (now / beat / bar / phrase)

See `protocol.md` for the full message format and per-module command tables.

## Electrical design notes

### Microcontroller

- **STM32H7** family preferred — has fast SDMMC peripheral for SD card,
  plenty of timers for clock counting and DAC updates, hardware I²C master
  with DMA.

### Clock-in conditioning

- AC-couple, then Schmitt trigger (74HC14 or comparator) → MCU GPIO with
  hardware timer input capture. Aim for < 100 µs jitter on detected edges.

### CV outputs

- 12-bit DAC (e.g. MCP4728 quad I²C, or a faster SPI DAC like DAC8814 if
  smoother ramps matter). Op-amp output stage for ±5 V, 1× gain, with
  short-circuit protection on the jack.

### Gate outputs

- Open-collector or MCU GPIO via level shifter to +5 V, current-limited,
  series resistor + clamping diode for back-EMF protection.

### Power topology

- Local 3V3 LDO for the digital section
- Separate analog rail for the CV DAC reference if using an external Vref
- Keep digital and analog grounds joined at a single star point near the
  power connector

### I²C bus

- 4k7 pull-ups to local 3V3 on SDA and SCL (conductor only)
- ESD protection diodes on SDA/SCL near the header
- Footprints for optional 100Ω series resistors on SDA/SCL (DNP by default)

## Firmware overview

- **Boot**: mount SD card, parse all `.lscore` files into RAM event lists
- **Idle**: display set picker, allow CURSOR navigation, await RUN
- **Playing**: count clock edges, fire events at bar/beat boundaries,
  expand continuous automations into DAC writes and I²C broadcasts
- **Hold**: freeze section advance, keep clock running, allow live tweaks
- **Jump**: navigate to any section in the loaded set, snap to next bar
- **Cue**: fire one-shot events tagged `@cue` in the score, can be selected during set with encoder
- **Panic**: broadcast mute-all to every module, stop CV automation,
  return to idle

Event timing is computed in MCU ticks from the clock count; I²C messages
are queued a few ms ahead of their target boundary so the slaves have
time to receive and arm.

## Build status

- [ ] Schematic
- [ ] PCB layout (110 mm tall constraint, components within panel-bounded zone)
- [ ] Panel artwork (SVG → KiCad footprint for silkscreen, or aluminum panel separate)
- [ ] Firmware skeleton (HAL setup, clock counter, SD mount, OLED driver)
- [ ] Score parser
- [ ] I²C dispatcher with quantization
- [ ] First slave retrofit (Quantizzle) for end-to-end test
- [ ] 5-minute live test set
- [ ] 30-minute set composition

## Open questions

- Display size: 1.3″ (compact, fits aesthetic) vs 2.42″ (more readable
  under stage conditions). ==> 2.42″ for live performability.
- Roman numeral count on the panel ring: ==> 24 (astrolabe-authentic, busier).
- Snapshot / recall: should the front panel save state to SD (current
  section + parameter snapshot) so a crashed set can be resumed? ==> yes (Continue from last section / phase?)

## Naming

**Ordo** — Latin, "order, rank, arrangement, sequence."
The conductor's job is to impose order on time. The Vitruvian panel
asserts the same: geometry as the visible expression of musical order.

---

*Part of the Lenimal Eurorack series. MMXXVI.*