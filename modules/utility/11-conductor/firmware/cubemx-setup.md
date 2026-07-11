# Ordo — STM32CubeMX setup reference

Target MCU: **STM32H7B0VBTx** (LQFP100, 128 KB flash, 1.4 MB SRAM, 280 MHz)

Walk through this top-to-bottom in CubeMX. Each section corresponds to a tab
or panel in the tool.

---

## 1. Pinout & Configuration → Peripherals

Enable each of the following. The order matters slightly — turn on RCC and
SYS first so the clock tree has somewhere to land, then peripherals so their
pins lock in before any GPIO assignments fight over them.

| CubeMX category   | Peripheral  | Mode                                       | Notes                                                  |
|-------------------|-------------|--------------------------------------------|--------------------------------------------------------|
| System Core       | RCC         | HSE = Crystal/Ceramic Resonator, LSE = Disable | 20 MHz crystal on PH0/PH1; no RTC in v1            |
| System Core       | SYS         | Timebase Source = SysTick                  | H7 has no "Debug" dropdown — SWD is enabled by pin AF (see §2) |
| Connectivity      | SDMMC1      | SD 4-bits Wide                             | PC8–11/PC12/PD2; IDMA built into peripheral            |
| Connectivity      | I2C1        | I2C                                        | PB6/PB7; for MCP4728 DAC                               |
| Connectivity      | I2C2        | I2C                                        | PB10/PB11; for slave-module broadcast bus              |
| Connectivity      | SPI1        | Transmit Only Master                       | PA5/PA7; OLED — no MISO                                |
| Connectivity      | USART3      | Asynchronous                               | PD8/PD9; debug serial                                  |
| Connectivity      | USB_OTG_FS  | Device_Only (Internal FS PHY)              | PA11/PA12; reserved header                             |
| Timers            | TIM1        | Combined Channels → Encoder Mode           | PA8/PA9; CURSOR encoder                                |
| Timers            | TIM2        | Channel1 → Input Capture direct mode       | PA0; CLK_IN edge timing (32-bit timer)                 |
| Analog            | ADC1        | IN14 single-ended                          | PA2; +12V rail monitor                                 |

### Peripheral parameter settings

#### SDMMC1
- **Clock divider**: 2 (gives 23 MHz from 46.7 MHz kernel; safe for hand-soldered traces, ~12 MB/s)
- **Hardware flow control**: Enable
- **Bus wide**: 4 bits
- Enable **SDMMC1 global interrupt**

#### I2C1 (DAC bus)
- **I2C Speed Mode**: Fast Mode
- **I2C Speed Frequency**: 400 kHz
- **Rise time / Fall time**: leave default (100/10 ns) — internal pull-ups
- Enable **I2C1 event** and **I2C1 error** interrupts

#### I2C2 (slave bus)
- Same as I2C1 (Fast Mode, 400 kHz)
- Enable both event + error interrupts

#### SPI1 (OLED)
- **Frame Format**: Motorola
- **Data Size**: 8 bits
- **First Bit**: MSB First
- **CPOL**: Low
- **CPHA**: 1 Edge
- **NSS Signal Type**: Disable (software-managed; we drive PA4 as GPIO)
- **Prescaler**: 16 (gives ~8.75 MHz on a 140 MHz APB2; plenty for OLED)

#### TIM1 (encoder)
- **Encoder Mode**: TI1 and TI2
- **Polarity**: Rising on both
- **Counter Period**: 0xFFFF (full 16-bit, software wraps it)
- **Prescaler**: 0
- Enable **TIM1 update interrupt** if you want overflow notifications (optional)

#### TIM2 (clock-in capture)
- **Prescaler**: 0 → tick at 280 MHz (3.57 ns resolution)
- **Counter Period**: 0xFFFFFFFF (full 32-bit; rolls over once per ~15 s — handle in ISR)
- **Channel 1**: Input Capture direct mode
- **Polarity**: Rising edge
- **IC Selection**: Direct
- **IC Prescaler**: 1
- **IC Filter**: 4 (debounces fast spikes from the 74HC14)
- Enable **TIM2 global interrupt**

#### USART3
- **Baud Rate**: 115200
- **Word Length**: 8 Bits (incl. parity)
- **Parity**: None
- **Stop Bits**: 1
- (DMA optional; polling is fine for occasional debug prints)

#### USB_OTG_FS
- **Mode**: Device_Only
- **Speed**: Full Speed 12 MBit/s
- (See Middleware → USB_DEVICE for class)

#### ADC1
- **Resolution**: 12 bits
- **IN14** (= PA2) → Single-ended
- **Continuous Conversion Mode**: Disable
- **End of Conversion Selection**: End of single conversion
- (Trigger from software; polled once a second is plenty for rail monitor)

---

## 2. Pinout view — GPIO assignments

Peripheral pins auto-assign when you enable the peripheral. **Verify each
matches the table below** (CubeMX sometimes picks a different alternate
function pin if it sees a conflict).

### SWD debug pins (set manually in Pinout view — H7 has no SYS Debug toggle)

| Pin   | Alternate Function    | Notes                                          |
|-------|-----------------------|------------------------------------------------|
| PA13  | `SYS_JTMS-SWDIO`      | Required — SWD data                            |
| PA14  | `SYS_JTCK-SWCLK`      | Required — SWD clock                           |
| PB3   | `SYS_JTDO-SWO`        | Optional — single-wire trace output            |

Click each pin in the Pinout chip view → pick the AF from the menu. There's
no Mode/Configuration panel for SWD itself; the pin assignments are the
enable.

### Peripheral pins (should be set automatically when you enable each peripheral)

| Pin   | Signal               | Peripheral                  |
|-------|----------------------|-----------------------------|
| PA0   | TIM2_CH1             | TIM2 IC                     |
| PA2   | ADC1_INP14           | ADC1 IN14                   |
| PA5   | SPI1_SCK             | SPI1                        |
| PA7   | SPI1_MOSI            | SPI1                        |
| PA8   | TIM1_CH1             | TIM1 encoder                |
| PA9   | TIM1_CH2             | TIM1 encoder                |
| PA11  | USB_OTG_FS_DM        | USB FS                      |
| PA12  | USB_OTG_FS_DP        | USB FS                      |
| PB6   | I2C1_SCL             | I2C1 (DAC)                  |
| PB7   | I2C1_SDA             | I2C1 (DAC)                  |
| PB10  | I2C2_SCL             | I2C2 (slaves)               |
| PB11  | I2C2_SDA             | I2C2 (slaves)               |
| PC8   | SDMMC1_D0            | SDMMC1                      |
| PC9   | SDMMC1_D1            | SDMMC1                      |
| PC10  | SDMMC1_D2            | SDMMC1                      |
| PC11  | SDMMC1_D3            | SDMMC1                      |
| PC12  | SDMMC1_CK            | SDMMC1                      |
| PD2   | SDMMC1_CMD           | SDMMC1                      |
| PD8   | USART3_TX            | USART3                      |
| PD9   | USART3_RX            | USART3                      |
| PH0   | RCC_OSC_IN           | HSE                         |
| PH1   | RCC_OSC_OUT          | HSE                         |

### GPIO pins (set manually — click each pin, choose mode + user label)

| Pin   | Mode                                   | Pull  | Default | User Label  |
|-------|----------------------------------------|-------|---------|-------------|
| PA1   | GPIO_EXTI1 (External Interrupt)        | Up    | —       | RST_IN      |
| PA4   | GPIO_Output                            | None  | High    | OLED_CS     |
| PB0   | GPIO_Output                            | None  | Low     | OLED_DC     |
| PB1   | GPIO_Output                            | None  | High    | OLED_RST    |
| PB8   | GPIO_EXTI8                             | Up    | —       | SD_CD       |
| PB9   | GPIO_Output                            | None  | High    | DAC_LDAC    |
| PE2   | GPIO_Output                            | None  | Low     | GATE1       |
| PE3   | GPIO_Output                            | None  | Low     | GATE2       |
| PE4   | GPIO_Output                            | None  | Low     | GATE3       |
| PE5   | GPIO_Output                            | None  | Low     | GATE4       |
| PE6   | GPIO_EXTI6                             | Up    | —       | ENC_SW      |
| PE7   | GPIO_EXTI7                             | Up    | —       | BTN_RUN     |
| PE8   | GPIO_EXTI8                             | Up    | —       | BTN_HOLD    |
| PE9   | GPIO_EXTI9                             | Up    | —       | BTN_JUMP    |
| PE10  | GPIO_EXTI10                            | Up    | —       | BTN_CUE     |
| PE11  | GPIO_Output                            | None  | Low     | LED_RUN     |
| PE12  | GPIO_Output                            | None  | Low     | LED_HOLD    |
| PE13  | GPIO_Output                            | None  | Low     | LED_JUMP    |
| PE14  | GPIO_Output                            | None  | Low     | LED_CUE     |

> ⚠️ PE7 and PB8 both map to EXTI line 8. Since they share the line, you'll
> need either two different pins, or accept that you can't tell BTN_RUN from
> SD_CD at the interrupt level (you'd read both in the ISR). Easy fix: move
> BTN_RUN to PE7 → EXTI7 (already correct in the table above; PE7 uses EXTI7,
> not EXTI8 — the EXTI line number tracks the pin number, not the port).
> Verify in CubeMX that no two pins share an EXTI line.

### EXTI line collisions (double-check this)

The H7's EXTI lines 0–15 are shared across ports. With our pinout:

- EXTI1: PA1 (RST_IN)
- EXTI6: PE6 (ENC_SW)
- EXTI7: PE7 (BTN_RUN)
- EXTI8: PB8 (SD_CD) **or** PE8 (BTN_HOLD) — **collision**
- EXTI9: PE9 (BTN_JUMP)
- EXTI10: PE10 (BTN_CUE)

**Fix the EXTI8 collision** by switching SD_CD off PB8. Move it to a free pin
on a port not using EXTI8 anywhere else. Candidates: **PE0** or **PE1**
(both spare). Change in CubeMX, update the pin table in the schematic notes,
done.

---

## 3. Clock Configuration tab

Target: **280 MHz SYSCLK, VOS0, all peripheral clocks valid.**

| Block | Setting |
|-------|---------|
| HSE                           | 20 MHz, on                            |
| PLL Source Mux                | HSE                                   |
| PLL1 M (input divider)        | /4 → 5 MHz VCO input                  |
| PLL1 N (multiplier)           | ×112 → 560 MHz VCO output             |
| PLL1 P (sys output divider)   | /2 → **280 MHz**                      |
| PLL1 Q (peripheral divider)   | /12 → ~46.7 MHz                       |
| PLL1 R (spare)                | /2 → 280 MHz                          |
| System Clock Mux              | PLL1P → SYSCLK                        |
| D1 Domain (CPU, AXI, AHB)     | /1 → 280 MHz                          |
| APB1, APB2, APB3, APB4        | /2 → 140 MHz                          |
| Power Regulator Voltage Scale | **VOS0** (required for 280 MHz)       |
| Flash Latency                 | 7 WS + WRHIGHFREQ=3 (auto)            |
| Power supply mode             | LDO (unless you've laid out for SMPS) |
| **PLL2**                      | **Off** — nothing in this design needs it |
| **PLL3**                      | **Off** — nothing in this design needs it |

### Activate the side oscillators

In RCC properties:

- **Activate HSI48 RC oscillator** — for USB 48 MHz clock
- **Activate Clock Recovery System** — locks HSI48 to USB SOF for tight accuracy

### Peripheral kernel-clock muxes

CubeMX exposes these under **RCC → "Mux" entries on the right** or
**Clock Configuration → individual peripheral muxes**:

| Peripheral  | Kernel clock source     | Why                                                |
|-------------|-------------------------|----------------------------------------------------|
| SDMMC1      | PLL1Q (~46.7 MHz)       | Stable, divides cleanly to SDMMC bus clock         |
| I2C1, I2C2  | HSI (64 MHz)            | Independent of PLL — survives clock changes        |
| SPI1        | APB2 (140 MHz)          | SPI prescaler divides down                         |
| USART3      | APB1 (140 MHz)          | Standard                                           |
| USB_OTG_FS  | **HSI48**               | Must be exactly 48 MHz; CRS auto-trims to USB SOF  |
| ADC1        | PER (any)               | Slow, not picky                                    |

---

## 4. DMA configuration

Per peripheral (click the peripheral → DMA Settings tab) or via the global
DMA panel. All are memory→peripheral except ADC and SDMMC.

| Stream         | Peripheral     | Direction    | Mode     | Data width | Priority |
|----------------|----------------|--------------|----------|------------|----------|
| DMA1 Stream 0  | SPI1_TX        | Mem → Periph | Normal   | Byte → Byte | Low      |
| DMA1 Stream 1  | I2C2_TX        | Mem → Periph | Normal   | Byte → Byte | Medium   |
| DMA1 Stream 2  | I2C1_TX        | Mem → Periph | Normal   | Byte → Byte | Low      |
| DMA2 Stream 0  | ADC1           | Periph → Mem | Circular | Half → Half | Low      |

Notes:
- **SDMMC1**: has its own IDMA inside the peripheral. **Don't** add a DMA
  channel for it — just enable the SDMMC1 global interrupt.
- The I2C2 RX direction is unused (we never receive from slaves) so no DMA
  needed there.

---

## 5. NVIC tab — Interrupt priorities

Priority group: **4 bits preemption, 0 bits sub-priority** (the H7 default).

| Interrupt                            | Pre-emption priority | Rationale                          |
|--------------------------------------|---------------------:|------------------------------------|
| TIM2 global (capture/compare)        | 0                    | Master clock edge timing           |
| SDMMC1 global                        | 1                    | Don't drop SD blocks               |
| I2C2 event / I2C2 error              | 2                    | Slave broadcasts                   |
| USB_OTG_FS                           | 3                    | DFU when active                    |
| I2C1 event / I2C1 error              | 3                    | DAC writes                         |
| DMA1 Stream 0/1/2                    | 3                    | Match the peripherals they serve   |
| EXTI1 (RST_IN)                       | 4                    | Reset is rare                      |
| EXTI6 (ENC_SW), EXTI7-10 (buttons)   | 5                    | UI is slow                         |
| EXTI line ‑ (SD_CD, once relocated)  | 5                    | Card insert/eject is slow          |
| USART3                               | 6                    | Debug only                         |
| SysTick                              | 15                   | HAL tick (lowest)                  |

---

## 6. Middleware

| Middleware       | Enable? | Settings                                          |
|------------------|---------|---------------------------------------------------|
| **FATFS**        | Yes     | Mode: SD Card. `USE_LFN = Enabled (RAM)`, `MAX_SS = 512`, `_FS_TINY = 0`, `_USE_STRFUNC = 1` (for f_printf in score parser if useful) |
| **USB_DEVICE**   | Optional, only if you populate USB header | Class: DFU. Bootloader-style flashing over USB.   |
| **FreeRTOS**     | Skip    | Bare-metal main loop saves ~10 KB flash; this app doesn't need an RTOS |

---

## 7. Project Manager

### Project tab

| Setting             | Value                                |
|---------------------|--------------------------------------|
| Project Name        | `ordo`                               |
| Project Location    | `modules/utility/11-conductor/firmware/` |
| Toolchain / IDE     | STM32CubeIDE *(or Makefile if you prefer)* |
| Linker Settings → Min Heap Size | `0x400` |
| Linker Settings → Min Stack Size | `0x800` |

### Code Generator tab

| Setting | Value |
|---------|-------|
| STM32Cube MCU packages... → Copy only the necessary library files | **On** (saves repo size) |
| Generated files → Generate peripheral initialization as a pair of `.c/.h` | **On** |
| HAL Settings → Set all free pins as analog | **On** (lowest power for unused pins) |
| HAL Settings → Enable Full Assert | Off in release, On during bring-up |

### Advanced Settings tab

Leave defaults unless you have a reason. CubeMX picks sensible drivers for
each peripheral.

---

## 8. Flash budget reality check

With 128 KB of internal flash you're working in a tight space. Rough estimate
of the firmware once everything's in:

| Component                                                | Estimate    |
|----------------------------------------------------------|-------------|
| HAL drivers (SDMMC, I2C×2, SPI, TIM×2, GPIO, RCC, USART, ADC) | ~35 KB |
| FatFs core + LFN                                         | ~12 KB      |
| Score parser + event scheduler                           | ~8 KB       |
| OLED driver + small font + UI                            | ~10 KB      |
| I2C dispatcher + protocol                                | ~4 KB       |
| DAC + gate output handling                               | ~3 KB       |
| Startup, vectors, libc stubs                             | ~3 KB       |
| **Subtotal (without USB)**                               | **~75 KB**  |
| USB DFU class (if enabled)                               | +15 KB      |
| FreeRTOS (if enabled)                                    | +10 KB      |
| **Headroom from 128 KB**                                 | **~38–50 KB** |

Three knobs if you start running tight:
1. **Swap HAL → LL** on the hot paths (I2C, SPI, TIM). Each LL swap saves
   1–3 KB.
2. **Drop USB DFU**, flash over SWD only.
3. **Compile with `-Os`** (CubeMX defaults to `-O2`).

If you push past 128 KB anyway, the H7B0 supports XiP from external OCTOSPI
flash — but that's a v2 board respin.

---

## 9. After CubeMX generates the project

Things to do in code, not in CubeMX:

1. In `main.c` after `MX_GPIO_Init()`, set initial LED states for visual
   feedback that boot reached user code.
2. Implement a tiny `printf` retarget over USART3 so `printf("...")` lands on
   the debug serial.
3. In the TIM2 capture ISR, store the captured CCR value and post a
   "clock edge" event to the main loop.
4. In the SDMMC IRQ handler, let HAL do its thing — FatFs takes care of the
   rest.
5. Start a small "blink LED_RUN at 1 Hz" smoke test before adding any of
   the application logic.

---

## 10. Open issues to resolve before generating code

- [ ] **EXTI8 collision**: relocate `SD_CD` from PB8 to PE0 (or PE1).
- [ ] **USB connector**: confirmed reserved-only for v1, no DFU class
      generated. *(toggle ON if/when you fit the USB-C breakout header)*
- [ ] **Encoder pin choice**: PA8/PA9 are also TIM1_CH1N candidates etc.
      Confirm CubeMX maps them to TIM1_CH1/CH2 alternate function 1.
- [ ] **VOS0 vs VOS1**: 280 MHz needs VOS0 *and* `SYSCFG_PWRCR_ODEN` set
      from code (CubeMX usually does it, but verify in `SystemClock_Config()`).
