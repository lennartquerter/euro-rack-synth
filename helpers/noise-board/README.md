# Noise Board — reusable piggyback noise source

**Status: ✅ Ready for production (v1).** Routed, DRC clean, 25 × 35 mm, sent
to JLCPCB. Full build/calibration guide: [`SCHEMATIC.md`](./SCHEMATIC.md).
A tiny SMD daughterboard carrying one known-good white-noise generator. It mounts on the **back** of a host module
via board-to-board headers — no front panel, no main-board real estate — and
is meant to be **reused across projects** (drums, wind/ocean FX, sample-and-
hold randomness, etc.).

SMD throughout except the header and mounting hardware.

---

## Why a separate board

- **One good source, built once.** Weak/drifty noise was the root cause of
  the dead snare and clap. Solving it on a dedicated, carefully laid-out
  little board means you never have to re-tune finicky junction-noise gain on
  every project again.
- **Noise doesn't need matching.** It's random — two separate generators
  sound identical — so unlike the tuned oscillator cores there is no reason
  to have more than one. One board, fan it out.
- **Saves space + is portable.** Frees HP/area on the host; can also be used
  standalone on a breadboard via flying leads.

---

## Circuit blocks

```
+12V ─[Rbias 1–10M]─┐
                    │ (reverse-biased B-E junction, avalanche/Zener noise)
                  Q1 E───► ~mV wideband noise
                    │
                    ▼
  AC-couple ─► GAIN A (×~100, TL072) ─► GAIN B (×~10, TL072) ─► BUFFER ─► WHITE OUT
                                                          └─► [opt. pink RC] ─► PINK OUT
```

1. **Noise generator** — an NPN with its **base-emitter junction reverse-
   biased into breakdown** (collector left open), fed from the +12 V rail
   through a large bias resistor (~1–10 M). Produces a few mV of wideband
   noise. The board carries **two parallel footprints — SOT-23 (MMBT3904)
   and TO-92 (2N3904) — populate exactly one**; junction noise varies part-
   to-part, so socket the TO-92 and test-select (see "tuning" below).
2. **Gain stage A** — AC-coupled high-gain amp (~40–60 dB), TL072.
3. **Gain stage B** — second stage to bring white noise up to a healthy
   **~5–8 Vpp**, with a gentle bandwidth limit to keep it controllable.
4. **Output buffer** — low output impedance so it can drive the header and a
   short cable into the host without level loss or hum pickup. AC-coupled.
5. **Optional pink output** — a 4-leg −3 dB/oct RC ladder off the white node
   plus a ×4.9 buffer on a **second TL072 (U2)**. Sim-verified at
   **−9.97 dB/decade, 1.0 dB ripple** (20 Hz–20 kHz). Independently
   populatable: leave U2 + the ladder unfitted for a white-only board.
   Nice for toms/“rumble” voices.
6. **Local rail decoupling** (100 nF + small electrolytic on each rail), plus
   an RC filter on the +12 V feeding the noise source for clean avalanche.

---

## Header standard — "Lenimal Noise Header v1"

Define the pinout **once** so every future host just drops the mating
footprint. **Two 2.54 mm headers:**

**J1 — power (1×3):**

| Pin | Net      |
|-----|----------|
| 1   | **+12V** |
| 2   | **GND**  |
| 3   | **−12V** |

**J2 — output (1×2):**

| Pin | Net                   | Notes                                  |
|-----|-----------------------|----------------------------------------|
| 1   | **NOISE_OUT** (white) | buffered, low-Z, AC-coupled, ~6 Vpp    |
| 2   | **PINK_OUT**          | dead unless the pink section is fitted |

Signal **return for J2 is the GND on J1** (shared system ground — fine for a
short, rigid piggyback). Full net-by-net design + values:
[`SCHEMATIC.md`](./SCHEMATIC.md).

- **Mechanical:** mount parallel to the host PCB on the back. For rigidity
  use **two headers** (one at each end) or **one header + a nylon standoff /
  M2 screw**. Keep components low-profile and mind total **depth behind the
  panel** — skiff cases are shallow (~25–30 mm), so this board adds to the
  host's depth budget. Orient the header so NOISE_OUT lands near the host's
  Noise-In pad.
- **Standalone use:** the same 5 pins work as flying leads to a breadboard.

**Host side:** a host that uses noise places matching 1×3 + 1×2 sockets and a
**Noise In** net. Design the host so that with *no* board fitted that voice
simply has no noise (normal Noise-In to ground), not a broken module.

---

## Tuning / gotchas

- **Transistor selection.** B-E breakdown noise is device-dependent. Socket
  Q1 (or provide a 2nd footprint) and test-select for the loudest, evenest
  hiss. A two-transistor cascade is an option if one stage is too quiet.
- **Sensitive front end.** Keep Q1 and Gain A traces short and away from any
  clock/digital lines on the host; this is a µV–mV node.
- **Set level with a trimmer** on Gain B so every host receives the same
  calibrated noise level.

---

## BOM (SMD unless noted)

Summary below; the **full per-reference BOM with values is in
[`SCHEMATIC.md`](./SCHEMATIC.md)**.

| Part                       | Pkg                | Use                                             |
|----------------------------|--------------------|-------------------------------------------------|
| 2× TL072                   | SOIC-8             | U1 gain stages; U2 pink (optional)              |
| MMBT3904 **and** 2N3904    | SOT-23 **+** TO-92 | noise junction — fit **one** (socket the TO-92) |
| 0805 R/C                   | 0805               | bias, gain, decoupling, pink ladder Rs          |
| Bias resistor 1 M          | 0805               | sets junction current (try up to 4M7)           |
| Trimmer 1 M                | RM-065 THT         | white-level calibration (RV1)                   |
| AC-couple / pink caps      | film (THT)         | C1/C4, C6/C25 out, pink legs C20–C23            |
| Electrolytics 10 µF        | THT                | rail bulk + noise-supply filter                 |
| J1 1×3 + J2 1×2 + standoff | THT                | board-to-board mount                            |

---

## Consumers in this repo

| Host module                               | Needs noise?              | Connection                              |
|-------------------------------------------|---------------------------|-----------------------------------------|
| Snare / Clap (`modules/drums/2-clap`)     | **Yes** (snares + clap)   | Noise-In header                         |
| Percussion (`modules/drums/8-percussion`) | **Yes** (shaker, rimshot) | Noise-In header                         |
| Cymbals & Hats (`modules/drums/3-hi-hat`) | Optional (a little "air") | Noise-In header, unpopulated by default |
| Toms / Subkick (`modules/drums/5-toms`)   | No                        | —                                       |
