# Hardware Design & Specification: DevEBox STM32H7XX-M

This directory contains the complete hardware specifications, schematics, datasheets, and pin mappings for the **DevEBox STM32H7XX-M (STM32H750VBT6)** board.

## Directory Layout

- **[`PINOUT.md`](./PINOUT.md)**: Authoritative pinout reference table (LEDs, buttons, QSPI flash, SD card, TFT header, and expansion headers).
- **[`PINOUT_CAPABILITIES.md`](./PINOUT_CAPABILITIES.md)**: Exhaustive pin capability matrix, alternate function mapping (PWM, UART, SPI, I2C, CAN, ADC/DAC), and conflict analysis.
- **`schematics/`**:
  - `schematic-STM32H7XX-M-en.pdf`: Complete schematic fully translated to English.
  - `pinout-diagram.svg`: Full-color vector graphical pinout diagram.
  - `pinout-diagram.png`: High-resolution graphic pinout diagram (7292 × 5730).
  - `pinout_viewer.html`: Interactive in-browser pinout explorer and search tool.
  - `original-schematic-STM32H7XX-M.pdf`: Original board schematic.
- **`datasheets/`**:
  - `STM32H750VB-Datasheet.pdf`: Official MCU electrical, pin, and package specifications (ST).
  - `PM0253-CortexM7-Programming-Manual.pdf`: Arm Cortex-M7 core, FPU, MPU, and NVIC programming guide (ST).
  - `ES0396-STM32H750-Errata.pdf`: STM32H750 silicon errata, known silicon bugs, and workarounds.
  - `W25Q64JV.pdf`: 8 MiB (64 Mbit) Winbond Quad-SPI Flash memory.
  - `ST7735S-TFT-LCD-Controller.pdf`: Controller datasheet for the onboard J4 TFT display header.
  - `AMS1117.pdf`: Onboard 3.3V 1A LDO voltage regulator.
- **`reference_docs/`**:
  - `STM32H7XX_M.jpg`: High-resolution photograph of the board layout.
  - Historical reference documentation and captures from stm32-base.org.

## Key Hardware Points for Firmware Developers

1. **User LED (D2)**: Connected to **`PA1`**. It is active **LOW** (current sink to ground). Driving `PA1` LOW turns the LED ON.
2. **User Buttons**:
   - `K1`: Connected to **`PE3`** (Active LOW).
   - `K2`: Connected to **`PC5`** (Active LOW).
3. **External QSPI Flash**: Winbond W25Q64JV (8MB) connected to STM32 QUADSPI Bank 1:
   - `PB6`: CS
   - `PB2`: CLK
   - `PD11`: IO0
   - `PD12`: IO1
   - `PE2`: IO2
   - `PD13`: IO3
4. **Oscillators**:
   - High Speed External (HSE): **25.0 MHz**
   - Low Speed External (LSE): **32.768 kHz**
