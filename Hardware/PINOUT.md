# DevEBox STM32H7XX-M (STM32H750VBT6) Hardware Reference & Pinout

This document serves as the **Hardware Source of Truth** for the DevEBox STM32H7XX-M board equipped with the STM32H750VBT6 microcontroller.

For complete alternate function mapping (PWM, UART, SPI, I2C, CAN, ADC, DAC, SDMMC, DCMI) and visual graphical diagrams, see:
- **[Full Pin Capabilities & Alternate Function Matrix](./PINOUT_CAPABILITIES.md)**
- **[Visual Vector Pinout Diagram (SVG)](./schematics/pinout-diagram.svg)**
- **[High-Resolution Pinout Graphic (PNG)](./schematics/pinout-diagram.png)**
- **[Interactive Browser Pinout Viewer (HTML)](./schematics/pinout_viewer.html)**
- **[English Translated Schematic PDF](./schematics/schematic-STM32H7XX-M-en.pdf)**

---

## 1. Board Summary

| Specification | Value |
|---|---|
| **MCU** | STM32H750VBT6 (Arm Cortex-M7 @ 400/480 MHz) |
| **Internal Flash** | 128 KiB |
| **Internal SRAM** | 1024 KiB (AXI SRAM: 512K, DTCM: 128K, ITCM: 64K, D2: 288K, D3: 64K) |
| **External Flash** | Winbond W25Q64JV (8 MiB / 64 Mbit Quad-SPI NOR Flash) |
| **Power Supply** | AMS1117-3.3 LDO (Max input +15V, outputs 3.3V @ 1A) |
| **High Speed Oscillator (HSE)** | 25.000 MHz Crystal |
| **Low Speed Oscillator (LSE)** | 32.768 kHz RTC Crystal |
| **Internal Oscillators** | HSI (64 MHz), LSI (32 kHz) |

---

## 2. Onboard Peripherals & Dedicated Pins

### A. LEDs and Buttons

| Component | Label | Pin | Logic / Polarity | Notes |
|---|---|---|---|---|
| **User LED** | D2 | **PA1** | **Active Low (Current Sink)** | Set PA1 LOW to turn LED ON |
| **Power LED** | D1 | +3.3V rail | Always ON | Hardwired to 3.3V rail |
| **User Button 1** | K1 | **PE3** | **Active Low** | Internal/External Pull-up needed |
| **User Button 2** | K2 | **PC5** | **Active Low** | Internal/External Pull-up needed |
| **Reset Button** | RST | **NRST** | Active Low | Hardware reset pin |

### B. External Quad-SPI Flash (Winbond W25Q64JV, SOIC-8)

| Flash Pin | Signal | STM32 Pin | Alternate Function |
|---|---|---|---|
| Pin 1 | /CS (Chip Select) | **PB6** | QUADSPI_BK1_NCS |
| Pin 2 | DO (IO1) | **PD12** | QUADSPI_BK1_IO1 |
| Pin 3 | /WP (IO2) | **PE2** | QUADSPI_BK1_IO2 |
| Pin 4 | GND | Ground | Ground Plane |
| Pin 5 | DI (IO0) | **PD11** | QUADSPI_BK1_IO0 |
| Pin 6 | CLK | **PB2** | QUADSPI_CLK |
| Pin 7 | /HOLD (IO3) | **PD13** | QUADSPI_BK1_IO3 |
| Pin 8 | VCC | +3.3V | Power Rail |

### C. MicroSD Card Socket (SDMMC1)

| Pin | Signal | STM32 Pin |
|---|---|---|
| 1 | DAT2 | **PC10** |
| 2 | CD / DAT3 | **PC11** |
| 3 | CMD | **PD2** |
| 4 | VDD | +3.3V |
| 5 | CLK | **PC12** |
| 6 | VSS | GND |
| 7 | DAT0 | **PC8** |
| 8 | DAT1 | **PC9** |
| 9 | Card Detect (CD) | N.C. |

### D. TFT LCD / OLED Header (J4, 8-Pin 2.54mm Female)

| Pin # | Header Label | STM32 Pin | Typical Function |
|---|---|---|---|
| 1 | 3V3 | +3.3V | Power |
| 2 | GND | GND | Ground |
| 3 | SDI | **PB15** | SPI MOSI |
| 4 | SCL | **PB13** | SPI SCK |
| 5 | CS | **PB12** | SPI Chip Select |
| 6 | SDO | **PB14** | SPI MISO |
| 7 | D/C | **PB1** | Data / Command Select |
| 8 | BLK | **PB0** | Backlight Control (PWM or GPIO) |

### E. SWD Debug Header (J1, 5-Pin 2.54mm Male)

| Pin # | Pin Label | STM32 Pin | Description |
|---|---|---|---|
| 1 | BT0 | BOOT0 | Boot mode select |
| 2 | 3V3 | +3.3V | Target 3.3V reference |
| 3 | GND | GND | Ground |
| 4 | SWDIO | **PA13** | SWD Data Input/Output |
| 5 | SWCLK | **PA14** | SWD Clock |

### F. Default Serial Console (USART1)

| Signal | STM32 Pin | Header Location |
|---|---|---|
| **USART1_TX** | **PA9** | Header 2, Pin 19 |
| **USART1_RX** | **PA10** | Header 2, Pin 22 |

---

## 3. Expansion Headers (2 x 44 Pin Headers)

### Header 1 (Left Header)

| Pin | Label | STM32 Pin | Pin | Label | STM32 Pin |
|---|---|---|---|---|---|
| **1** | 3V3 | +3.3V | **2** | 3V3 | +3.3V |
| **3** | GND | GND | **4** | GND | GND |
| **5** | VDDA | VDDA | **6** | VREF+ | VREF+ |
| **7** | PB15 | PB15 | **8** | PB14 | PB14 |
| **9** | PB13 | PB13 | **10** | PB12 | PB12 |
| **11** | PB11 | PB11 | **12** | PB10 | PB10 |
| **13** | PE15 | PE15 | **14** | PE14 | PE14 |
| **15** | PE13 | PE13 | **16** | PE12 | PE12 |
| **17** | PE11 | PE11 | **18** | PE10 | PE10 |
| **19** | PE9 | PE9 | **20** | PE8 | PE8 |
| **21** | PE7 | PE7 | **22** | PB2 | PB2 (QSPI_CLK) |
| **23** | PB1 | PB1 | **24** | PB0 | PB0 |
| **25** | PC5 | PC5 (K2 Button)| **26** | PC4 | PC4 |
| **27** | PA7 | PA7 | **28** | PA6 | PA6 |
| **29** | PA5 | PA5 | **30** | PA4 | PA4 |
| **31** | PA3 | PA3 | **32** | PA2 | PA2 |
| **33** | PA1 | PA1 (D2 LED) | **34** | PA0 | PA0 |
| **35** | PC3 | PC3 | **36** | PC2 | PC2 |
| **37** | PC1 | PC1 | **38** | PC0 | PC0 |
| **39** | PC13 | PC13 | **40** | PE6 | PE6 |
| **41** | PE5 | PE5 | **42** | PE4 | PE4 |
| **43** | PE3 | PE3 (K1 Button)| **44** | PE2 | PE2 (QSPI_IO2) |

---

### Header 2 (Right Header)

| Pin | Label | STM32 Pin | Pin | Label | STM32 Pin |
|---|---|---|---|---|---|
| **1** | 5V | +5V Rail | **2** | 5V | +5V Rail |
| **3** | 3V3 | +3.3V | **4** | 3V3 | +3.3V |
| **5** | GND | GND | **6** | GND | GND |
| **7** | PD9 | PD9 | **8** | PD8 | PD8 |
| **9** | PD11 | PD11 (QSPI_IO0)| **10** | PD10 | PD10 |
| **11** | PD13 | PD13 (QSPI_IO3)| **12** | PD12 | PD12 (QSPI_IO1)|
| **13** | PD15 | PD15 | **14** | PD14 | PD14 |
| **15** | PC7 | PC7 | **16** | PC6 | PC6 |
| **17** | PC9 | PC9 (SD_D1) | **18** | PC8 | PC8 (SD_D0) |
| **19** | PA9 | PA9 (UART1_TX) | **20** | PA8 | PA8 |
| **21** | PA11 | PA11 (USB_DM) | **22** | PA10 | PA10 (UART1_RX)|
| **23** | PA15 | PA15 | **24** | PA12 | PA12 (USB_DP) |
| **25** | PC11 | PC11 (SD_D3) | **26** | PC10 | PC10 (SD_D2) |
| **27** | PD0 | PD0 | **28** | PC12 | PC12 (SD_CLK) |
| **29** | PD2 | PD2 (SD_CMD) | **30** | PD1 | PD1 |
| **31** | PD4 | PD4 | **32** | PD3 | PD3 |
| **33** | PD6 | PD6 | **34** | PD5 | PD5 |
| **35** | PB3 | PB3 (SWO) | **36** | PD7 | PD7 |
| **37** | PB5 | PB5 | **38** | PB4 | PB4 |
| **39** | PB7 | PB7 | **40** | PB6 | PB6 (QSPI_CS) |
| **41** | PB9 | PB9 | **42** | PB8 | PB8 |
| **43** | PE1 | PE1 | **44** | PE0 | PE0 |
