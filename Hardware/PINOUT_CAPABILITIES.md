# DevEBox STM32H750VBT6 (STM32H7XX-M) Pinout & Capability Reference Guide

This document is the authoritative, comprehensive pinout and peripheral multiplexing reference for the **DevEBox STM32H750VBT6 (STM32H7XX-M)** core board.

---

## Visual Reference Files

- **Vector SVG Diagram**: [`schematics/pinout-diagram.svg`](./schematics/pinout-diagram.svg)
- **High-Resolution PNG Diagram**: [`schematics/pinout-diagram.png`](./schematics/pinout-diagram.png) (7292 × 5730 pixels)
- **Interactive Browser Pinout Viewer**: [`schematics/pinout_viewer.html`](./schematics/pinout_viewer.html) (Search, filter by PWM/UART/SPI/I2C/ADC, inspect AF0–AF15)
- **Translated Schematic PDF**: [`schematics/schematic-STM32H7XX-M-en.pdf`](./schematics/schematic-STM32H7XX-M-en.pdf)

---

## 1. TFT LCD / OLED Header (J4, 1x8 SIP-2.54mm)

The J4 header is pre-routed specifically for small TFT LCD and OLED displays (e.g., ST7735, ST7789, ILI9341, SSD1306). All signal lines are shared with MCU Port B pins.

| Pin # | Header Label | STM32 Pin | Default Function | Alternate Functions & Capabilities | Notes / Driver Wiring |
|:---:|:---:|:---:|:---:|:---|:---|
| **1** | `3V3` | +3.3V Rail | Power Supply | 3.3V DC power rail from onboard AMS1117-3.3 | Display VCC |
| **2** | `GND` | Ground Plane | Ground | Ground reference | Display GND |
| **3** | `SDI / MOSI` | **PB15** | SPI2_MOSI | SPI2_MOSI (AF5), I2S2_SDO (AF5), TIM1_CH3N (AF1), TIM8_CH3N (AF3), DCMI_D13 (AF13) | Connect to LCD **SDA / MOSI** |
| **4** | `SCL / SCK` | **PB13** | SPI2_SCK | SPI2_SCK (AF5), I2S2_CK (AF5), TIM1_CH1N (AF1), UART5_TX (AF8), ETH_TXD1 (AF11) | Connect to LCD **SCL / SCK** |
| **5** | `CS` | **PB12** | SPI2_NSS | SPI2_NSS (AF5), I2S2_WS (AF5), TIM1_BKIN (AF1), UART5_RX (AF8), FDCAN2_RX (AF9) | Connect to LCD **CS** (Active Low) |
| **6** | `SDO / MISO` | **PB14** | SPI2_MISO | SPI2_MISO (AF5), TIM1_CH2N (AF1), TIM8_CH2N (AF3), USART1_TX (AF4), TIM12_CH1 (AF2) | Connect to LCD **MISO / SDO** (if present) |
| **7** | `RS / DC` | **PB1** | GPIO Out | GPIO Data/Command select, TIM1_CH3N (AF1), TIM3_CH4 (AF2), ADC12_INP5 | HIGH = Data, LOW = Command |
| **8** | `BLK / LED` | **PB0** | TIM3_CH3 (PWM) | Backlight Dimming PWM, TIM1_CH2N (AF1), TIM3_CH3 (AF2), TIM8_CH2N (AF3), ADC12_INP9 | Drive HIGH or use TIM3_CH3 PWM for dimming |

### Typical Display Controller Connection Guide

| Display Module Type | J4 Pin 1 (3V3) | J4 Pin 2 (GND) | J4 Pin 3 (SDI) | J4 Pin 4 (SCL) | J4 Pin 5 (CS) | J4 Pin 6 (SDO) | J4 Pin 7 (RS/DC) | J4 Pin 8 (BLK) |
|:---|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
| **0.96" / 1.3" OLED (SSD1306 SPI)** | VCC | GND | D1 (MOSI) | D0 (CLK) | CS | N/C | DC | N/C |
| **0.96" / 1.3" OLED (SSD1306 I2C)** | VCC | GND | SDA (PB15 software I2C) | SCL (PB13 software I2C) | N/C | N/C | N/C | N/C |
| **1.44" / 1.8" TFT (ST7735S)** | VCC | GND | SDA | SCL | CS | N/C | A0 / DC | LED / BLK |
| **1.3" / 2.0" IPS TFT (ST7789)** | VCC | GND | SDA | SCL | CS | N/C | DC | BLK |
| **2.4" / 2.8" TFT (ILI9341)** | VCC | GND | MOSI | SCK | CS | MISO | DC | LED |

---

## 2. Left Expansion Header (J3, 2x22 Pins Dual-Row)

- **Header Label**: `J3` (Left edge of board)
- **Pin Count**: 44 Pins (Odd pins outer row, Even pins inner row)

| Pin # | MCU Pin | LQFP100 | 5V Tol | Onboard Wiring | PWM / Timers | ADC / DAC | UART / USART | SPI / I2S | I2C | CAN | Extra / High-Speed |
|:---:|:---:|:---:|:---:|:---|:---|:---|:---|:---|:---|:---|:---|
| **1** | `3V3` | - | N/A | Power Rail | - | - | - | - | - | - | - |
| **2** | `3V3` | - | N/A | Power Rail | - | - | - | - | - | - | - |
| **3** | `GND` | - | N/A | Ground Plane | - | - | - | - | - | - | - |
| **4** | `GND` | - | N/A | Ground Plane | - | - | - | - | - | - | - |
| **5** | `VDDA` | - | N/A | Power Rail | - | - | - | - | - | - | - |
| **6** | `VREF+` | - | N/A | Power Rail | - | - | - | - | - | - | - |
| **7** | `PB15` | 54 | 5V | TFT LCD J4-3 (SPI2_MOSI) | TIM1 CH3N, TIM12 CH2 | - | USART1 RX, UART4 CTS | SPI2 MOSI, I2S2 SDO | - | - | SDMMC2 D1, OTG HS DP |
| **8** | `PB14` | 53 | 5V | TFT LCD J4-6 (SPI2_MISO) | TIM1 CH2N, TIM12 CH1 | - | USART1 TX, USART3 RTS | SPI2 MISO, I2S2 SDI | - | - | SDMMC2 D0, OTG HS DM |
| **9** | `PB13` | 52 | 5V | TFT LCD J4-4 (SPI2_SCK) | TIM1 CH1N, LPTIM2 OUT | - | USART3 CTS, USART3 NSS | SPI2 SCK, I2S2 CK | - | FDCAN2 TX | OTG HS ULPI D6 |
| **10** | `PB12` | 51 | 5V | TFT LCD J4-5 (SPI2_NSS / CS) | - | - | USART3 CK, UART5 RX | SPI2 NSS, I2S2 WS | I2C2 SMBA | FDCAN2 RX | OTG HS ULPI D5, OTG HS ID |
| **11** | `PB11` | 47 | 5V |  | TIM2 CH4, HRTIM SCIN | - | USART3 RX | - | I2C2 SDA | - | OTG HS ULPI D4 |
| **12** | `PB10` | 46 | 5V |  | TIM2 CH3, HRTIM SCOUT | - | USART3 TX | SPI2 SCK, I2S2 CK | I2C2 SCL | - | OTG HS ULPI D3, QUADSPI BK1 NCS |
| **13** | `PE15` | 45 | 5V |  | - | - | - | - | - | - | FMC D12, FMC DA12 |
| **14** | `PE14` | 44 | 5V |  | TIM1 CH4 | - | - | SPI4 MOSI | - | - | FMC D11, FMC DA11 |
| **15** | `PE13` | 43 | 5V |  | TIM1 CH3 | - | - | SPI4 MISO | - | - | FMC D10, FMC DA10 |
| **16** | `PE12` | 42 | 5V |  | TIM1 CH3N | - | - | SPI4 SCK | - | - | FMC D9, FMC DA9 |
| **17** | `PE11` | 41 | 5V |  | TIM1 CH2 | - | - | SPI4 NSS | - | - | FMC D8, FMC DA8 |
| **18** | `PE10` | 40 | 5V |  | TIM1 CH2N | - | UART7 CTS | QUADSPI BK2 IO3 | - | - | QUADSPI BK2 IO3, FMC D7 |
| **19** | `PE9` | 39 | 3.3V |  | TIM1 CH1 | - | UART7 RTS, UART7 DE | QUADSPI BK2 IO2 | - | - | QUADSPI BK2 IO2, FMC D6 |
| **20** | `PE8` | 38 | 3.3V |  | TIM1 CH1N | - | UART7 TX | QUADSPI BK2 IO1 | - | - | QUADSPI BK2 IO1, FMC D5 |
| **21** | `PE7` | 37 | 3.3V |  | - | - | UART7 RX | QUADSPI BK2 IO0 | - | - | QUADSPI BK2 IO0, FMC D4 |
| **22** | `PB2` | 36 | 5V | W25Q64 QSPI Flash CLK | - | - | - | SPI3 MOSI, I2S3 SDO | - | - | QUADSPI CLK |
| **23** | `PB1` | 35 | 3.3V | TFT LCD J4-7 (RS / DC) | TIM1 CH3N, TIM3 CH4 | ADC12 INP5 | - | - | - | - | OTG HS ULPI D2 |
| **24** | `PB0` | 34 | 5V | TFT LCD J4-8 (BLK / Backlight PWM) | TIM1 CH2N, TIM3 CH3 | ADC12 INN5, ADC12 INP9 | UART4 CTS | - | - | - | OTG HS ULPI D1 |
| **25** | `PC5` | 33 | 3.3V | User Button K2 (Active LOW) | - | ADC12 INN4, ADC12 INP8 | - | - | - | - | FMC SDCK E0 |
| **26** | `PC4` | 32 | 3.3V |  | - | ADC12 INP4 | - | I2S1 MCK | - | - | FMC SDNE 0 |
| **27** | `PA7` | 31 | 3.3V |  | TIM1 CH1N, TIM3 CH2 | ADC12 INN3, ADC12 INP7 | - | SPI1 MOSI, I2S1 SDO | - | - | FMC SDN WE |
| **28** | `PA6` | 30 | 5V |  | TIM3 CH1, TIM13 CH1 | ADC12 INP3 | - | SPI1 MISO, I2S1 SDI | - | - | DCMI PIX CLK |
| **29** | `PA5` | 29 | 3.3V |  | TIM2 CH1, TIM8 CH1N | ADC12 INN18, ADC12 INP19 | - | SPI1 SCK, I2S1 CK | - | - | OTG HS ULPI CK |
| **30** | `PA4` | 28 | 3.3V |  | - | ADC12 INP18, DAC1 OUT1 | USART2 CK | SPI1 NSS, I2S1 WS | - | - | DCMI HSYNC, OTG HS SOF |
| **31** | `PA3` | 25 | 5V |  | TIM2 CH4, TIM5 CH4 | ADC12 INP15 | USART2 RX | - | - | - | OTG HS ULPI D0 |
| **32** | `PA2` | 24 | 5V |  | TIM2 CH3, TIM5 CH3 | ADC12 INP14 | USART2 TX | - | - | - | WKUP1 |
| **33** | `PA1` | 23 | 5V | User LED D2 (Active LOW) | TIM2 CH2, TIM5 CH2 | ADC1 INN16, ADC1 INP17 | USART2 RTS, USART2 DE | QUADSPI BK1 IO3 | - | - | QUADSPI BK1 IO3 |
| **34** | `PA0` | 22 | 5V |  | TIM2 CH1, TIM5 CH1 | ADC1 INP16 | USART2 CTS, USART2 NSS | - | - | - | SDMMC2 CMD, WKUP0 |
| **35** | `PC3` | - | 5V |  | - | ADC12 INN12, ADC12 INP13 | - | SPI2 MOSI, I2S2 SDO | - | - | OTG HS ULPI NXT, FMC SDCK E0 |
| **36** | `PC2` | - | 5V |  | - | ADC123  INN11, ADC123  INP12 | - | SPI2 MISO, I2S2 SDI | - | - | OTG HS ULPI DIR, FMC SDNE 0 |
| **37** | `PC1` | 16 | 5V |  | - | ADC123  INN10, ADC123  INP11 | - | SPI2 MOSI, I2S2 SDO | - | - | SDMMC2 CK, RTC TAMP3/  WKUP5 |
| **38** | `PC0` | 15 | 5V |  | - | ADC123  INP10 | - | - | - | - | OTG HS ULPI STP, FMC SDNWE |
| **39** | `PC13` | 7 | 5V |  | - | - | - | - | - | - | RTC TAMP1/  RTC TS/WKUP2 |
| **40** | `PE6` | 5 | 5V |  | TIM15 CH2 | - | - | SPI4 MOSI | - | - | DCMI D7, FMC A22 |
| **41** | `PE5` | 4 | 5V |  | TIM15 CH1 | - | - | SPI4 MISO | - | - | DCMI D6, FMC A21 |
| **42** | `PE4` | 3 | 5V |  | TIM15 CH1 N | - | - | SPI4 NSS | - | - | DCMI D4, FMC A20 |
| **43** | `PE3` | 2 | 5V | User Button K1 (Active LOW) | - | - | - | - | - | - | FMC A19 |
| **44** | `PE2` | 1 | 5V | W25Q64 QSPI Flash /WP (IO2) | - | - | - | SPI4 SCK, QUADSPI BK1 IO2 | - | - | QUADSPI BK1 IO2, FMC A23 |

---

## 3. Right Expansion Header (J2, 2x22 Pins Dual-Row)

- **Header Label**: `J2` (Right edge of board)
- **Pin Count**: 44 Pins (Odd pins inner row, Even pins outer row)

| Pin # | MCU Pin | LQFP100 | 5V Tol | Onboard Wiring | PWM / Timers | ADC / DAC | UART / USART | SPI / I2S | I2C | CAN | Extra / High-Speed |
|:---:|:---:|:---:|:---:|:---|:---|:---|:---|:---|:---|:---|:---|
| **1** | `5V` | - | N/A | Power Rail | - | - | - | - | - | - | - |
| **2** | `5V` | - | N/A | Power Rail | - | - | - | - | - | - | - |
| **3** | `3V3` | - | N/A | Power Rail | - | - | - | - | - | - | - |
| **4** | `3V3` | - | N/A | Power Rail | - | - | - | - | - | - | - |
| **5** | `GND` | - | N/A | Ground Plane | - | - | - | - | - | - | - |
| **6** | `GND` | - | N/A | Ground Plane | - | - | - | - | - | - | - |
| **7** | `PD9` | 56 | 5V |  | - | - | USART3 RX | - | - | FDCAN2 R XFD MODE | FMC D14, FMC DA14 |
| **8** | `PD8` | 55 | 5V |  | - | - | USART3 TX | - | - | - | FMC D13, FMC DA13 |
| **9** | `PD11` | 58 | 5V | W25Q64 QSPI Flash DI (IO0) | LPTIM2 IN2 | - | USART3 CTS, USART3 N SS | QUADSPI BK1 IO0 | I2C4 SMBA | - | QUADSPI BK1 IO0, FMC A16 |
| **10** | `PD10` | 57 | 5V |  | - | - | USART3 CK | - | - | FDCAN2 T XFD MODE | FMC D15, FMC DA15 |
| **11** | `PD13` | 60 | 5V | W25Q64 QSPI Flash /HOLD (IO3) | LPTIM1 OUT, TIM4 CH2 | - | - | QUADSPI BK1 IO3 | I2C4 SDA | - | QUADSPI BK1 IO3, FMC A18 |
| **12** | `PD12` | 59 | 5V | W25Q64 QSPI Flash DO (IO1) | LPTIM1 IN1, TIM4 CH1 | - | USART3 RTS, USART3 DE | QUADSPI BK1 IO1 | I2C4 SCL | - | QUADSPI BK1 IO1, FMC A17 |
| **13** | `PD15` | 62 | 5V |  | TIM4 CH4 | - | UART8 RTS, UART8 DE | - | - | - | FMC D1, FMC DA1 |
| **14** | `PD14` | 61 | 5V |  | TIM4 CH3 | - | UART8 CTS | - | - | - | FMC D0, FMC DA0 |
| **15** | `PC7` | 64 | 5V |  | HRTIM CH A2, TIM3 CH2 | - | USART6 RX | I2S3 MCK | - | - | SDMMC1 D123DIR, SDMMC2 D7 |
| **16** | `PC6` | 63 | 5V |  | HRTIM CH A1, TIM3 CH1 | - | USART6 TX | I2S2 MCK | - | - | SDMMC1 D0DIR, SDMMC2 D6 |
| **17** | `PC9` | 66 | 5V | MicroSD DAT1 (SDMMC1_D1) | TIM3 CH4, TIM8 CH4 | - | UART5 CTS | I2S CKIN, QUADSPI BK1 IO0 | I2C3 SDA | - | SDMMC1 D1, DCMI D3 |
| **18** | `PC8` | 65 | 5V | MicroSD DAT0 (SDMMC1_D0) | HRTIM CH B1, TIM3 CH3 | - | USART6 CK, UART5 RTS | - | - | - | SDMMC1 D0, DCMI D2 |
| **19** | `PA9` | 68 | 5V | USART1_TX (Default Serial Console) | TIM1 CH2, HRTIM CH C1 | - | LPUART1 TX, USART1 TX | SPI2 SCK, I2S2 CK | I2C3 SMBA | FDCAN1 RXFD MODE | DCMI D0 |
| **20** | `PA8` | 67 | 5V |  | TIM1 CH1, HRTIM CH B2 | - | USART1 CK, UART7 RX | - | I2C3 SCL | - | OTG FS SOF |
| **21** | `PA11` | 70 | 5V | USB OTG_FS DM (Micro-USB Port) | TIM1 CH4, HRTIM CH D1 | - | LPUART1 CTS, UART4 RX | SPI2 NSS, I2S2 WS | - | FDCAN1 RX | OTG FS DM |
| **22** | `PA10` | 69 | 5V | USART1_RX (Default Serial Console) | TIM1 CH3, HRTIM CH C2 | - | LPUART1 RX, USART1 RX | - | - | FDCAN1 TXFD MODE | DCMI D1, OTG FS ID |
| **23** | `PA15` |  | 3.3V |  | TIM2 CH1, HRTIM FLT1 | - | UART4 RTS, UART4 DE | SPI1 NSS, I2S1 WS | - | - | - |
| **24** | `PA12` | 71 | 5V | USB OTG_FS DP (Micro-USB Port) | HRTIM CH D2 | - | LPUART1 RTS, LPUART1 DE | SPI2 SCK, I2S2 CK | - | FDCAN1 TX | OTG FS DP |
| **25** | `PC11` | 79 | 5V | MicroSD DAT3 (SDMMC1_D3) | HRTIM FLT2 | - | USART3 RX, UART4 RX | SPI3 MISO, I2S3 SDI | - | - | SDMMC1 D3, DCMI D4 |
| **26** | `PC10` | 78 | 5V | MicroSD DAT2 (SDMMC1_D2) | HRTIM EEV1 | - | USART3 TX, UART4 TX | SPI3 SCK, I2S3 CK | - | - | SDMMC1 D2, DCMI D8 |
| **27** | `PD0` | 81 | 5V |  | - | - | UART4 RX | - | - | FDCAN1 RX | FMC D2, FMC DA2 |
| **28** | `PC12` | 80 | 5V | MicroSD CLK (SDMMC1_CK) | HRTIM EEV2 | - | USART3 CK, UART5 TX | SPI3 MOSI, I2S3 SDO | - | - | SDMMC1 CK, DCMI D9 |
| **29** | `PD2` | 83 | 5V | MicroSD CMD (SDMMC1_CMD) | - | - | UART5 RX | - | - | - | SDMMC1 CMD, DCMI D11 |
| **30** | `PD1` | 82 | 5V |  | - | - | UART4 TX | - | - | FDCAN1 TX | FMC D3, FMC DA3 |
| **31** | `PD4` | 85 | 5V |  | HRTIM FLT3 | - | USART2 RTS, USART2 DE | - | - | FDCAN1 R XFD MODE | FMC NOE |
| **32** | `PD3` | 84 | 5V |  | - | - | USART2 CTS, USART2 NSS | SPI2 SCK, I2S2 CK | - | - | DCMI D5, FMC CLK |
| **33** | `PD6` | 87 | 5V |  | - | - | USART2 RX | SPI3 MOSI, I2S3 SDO | - | FDCAN2 R XFD MODE | SDMMC2 CK, DCMI D10 |
| **34** | `PD5` | 86 | 5V |  | HRTIM EEV3 | - | USART2 TX | - | - | FDCAN1 T XFD MODE | FMC NWE |
| **35** | `PB3` |  | 3.3V |  | TIM2 CH2, HRTIM FLT4 | - | UART7 RX | SPI1 SCK, I2S1 CK | - | - | SDMMC2 D2 |
| **36** | `PD7` | 88 | 5V |  | - | - | USART2 CK | SPI1 MOSI, I2S1 SDO | - | - | SDMMC2 CMD, FMC NE1 |
| **37** | `PB5` | 91 | 5V |  | TIM3 CH2, HRTIM EEV7 | - | UART5 RX | SPI1 MOSI, I2S1 SDO | I2C1 SMBA, I2C4 SMBA | FDCAN2 RX | DCMI D10, OTG HS ULPI D7 |
| **38** | `PB4` |  | 3.3V |  | TIM3 CH1, HRTIM EEV6 | - | UART7 TX | SPI1 MISO, I2S1 SDI | - | - | SDMMC2 D3 |
| **39** | `PB7` | 93 | 5V |  | TIM17 CH1N, TIM4 CH2 | - | USART1 RX, LPUART1 RX | - | I2C1 SDA, I2C4 SDA | FDCAN2 TXFD MODE | DCMI VSYNC, FMC NL |
| **40** | `PB6` | 92 | 5V | W25Q64 QSPI Flash /CS | TIM16 CH1N, TIM4 CH1 | - | USART1 TX, LPUART1 TX | QUADSPI BK1 NCS | I2C1 SCL, I2C4 SCL | FDCAN2 TX | DCMI D5, QUADSPI BK1 NCS |
| **41** | `PB9` | 96 | 5V |  | TIM17 CH1, TIM4 CH4 | - | UART4 TX | SPI2 NSS, I2S2 WS | I2C1 SDA, I2C4 SDA | FDCAN1 TX | SDMMC1 CDIR, SDMMC2 D5 |
| **42** | `PB8` | 95 | 5V |  | TIM16 CH1, TIM4 CH3 | - | UART4 RX | - | I2C1 SCL, I2C4 SCL | FDCAN1 RX | SDMMC1 CKIN, SDMMC2 D4 |
| **43** | `PE1` | 98 | 5V |  | LPTIM1 IN2, HRTIM SCOUT | - | UART8 TX | - | - | FDCAN1 TXFD MODE | DCMI D3, FMC NBL1 |
| **44** | `PE0` | 97 | 5V |  | LPTIM1 ETR, HRTIM SCIN | - | UART8 RX | - | - | FDCAN1 RXFD MODE | DCMI D2, FMC NBL0 |

---

## 4. SWD Debug Header (J1, 1x5 Pin) & BOOT Configuration

| Pin # | Label | STM32 Pin | Function | Description |
|:---:|:---:|:---:|:---|:---|
| **1** | `BT0` | **BOOT0** | Boot Mode Select | Pulled down to GND via 10 kΩ resistor R7 by default |
| **2** | `3V3` | **+3.3V** | Target VCC | Reference voltage output for ST-Link / J-Link debugger |
| **3** | `GND` | **GND** | Ground | Ground reference for debugger |
| **4** | `SWDIO` | **PA13** | SWD Data | Serial Wire Debug Data Input/Output (internal pull-up) |
| **5** | `SWCLK` | **PA14** | SWD Clock | Serial Wire Debug Clock (internal pull-down) |

### BOOT Mode Truth Table

| BOOT0 (J1 Pin 1) | BOOT1 (Option Byte) | Boot Space | Description |
|:---:|:---:|:---|:---|
| **0** (Default) | X (Don't care) | **Main User Flash** | Normal execution from 0x08000000 |
| **1** (Tied to 3.3V) | **0** | **System Memory** | Built-in ST ROM bootloader (UART1 / USB DFU download) |
| **1** (Tied to 3.3V) | **1** | **Embedded SRAM** | Execution from SRAM at 0x20000000 |

---

## 5. Onboard Peripheral Conflict & Pin Sharing Matrix

When designing custom circuits, be aware of pins shared between header pins and onboard components:

| STM32 Pin | Header Pin | Onboard Hardware | Dedicated / Shared | Recommended Handling |
|:---|:---:|:---|:---:|:---|
| **PA1** | J3-33 | **User LED D2** | Shared (Active Low) | Driving PA1 LOW turns LED D2 ON. If used as input, state is pulled up by LED circuit. |
| **PE3** | J3-43 | **User Button K1** | Shared (Active Low) | Pressing K1 shorts PE3 to GND. Enable internal pull-up resistor. |
| **PC5** | J3-25 | **User Button K2** | Shared (Active Low) | Pressing K2 shorts PC5 to GND. Enable internal pull-up resistor. |
| **PB6** | J2-40 | **QSPI Flash /CS** | Dedicated to QSPI | Do NOT use as GPIO if external SPI/QSPI flash is accessed. |
| **PB2** | J3-22 | **QSPI Flash CLK** | Dedicated to QSPI | High-speed clock line for W25Q64. |
| **PD11** | J2-9 | **QSPI Flash IO0 (DI)** | Dedicated to QSPI | Quad-SPI Data 0. |
| **PD12** | J2-12 | **QSPI Flash IO1 (DO)** | Dedicated to QSPI | Quad-SPI Data 1. |
| **PE2** | J3-44 | **QSPI Flash IO2 (/WP)**| Dedicated to QSPI | Quad-SPI Data 2. |
| **PD13** | J2-11 | **QSPI Flash IO3 (/HOLD)**| Dedicated to QSPI | Quad-SPI Data 3. |
| **PC8** | J2-18 | **MicroSD DAT0** | Shared with SD Card | SDMMC1_D0. |
| **PC9** | J2-17 | **MicroSD DAT1** | Shared with SD Card | SDMMC1_D1. |
| **PC10** | J2-26 | **MicroSD DAT2** | Shared with SD Card | SDMMC1_D2. |
| **PC11** | J2-25 | **MicroSD DAT3** | Shared with SD Card | SDMMC1_D3. |
| **PC12** | J2-28 | **MicroSD CLK** | Shared with SD Card | SDMMC1_CK. |
| **PD2** | J2-29 | **MicroSD CMD** | Shared with SD Card | SDMMC1_CMD. |
| **PA9** | J2-19 | **USART1_TX** | Header & Console | Serial output for debug prints. |
| **PA10** | J2-22 | **USART1_RX** | Header & Console | Serial input for shell / REPL. |
| **PA11** | J2-21 | **USB OTG_FS DM** | Shared with USB Port | USB Data Minus (D-). |
| **PA12** | J2-24 | **USB OTG_FS DP** | Shared with USB Port | USB Data Plus (D+). |
| **PB15** | J3-7 & J4-3 | **TFT Display MOSI** | Shared | Display SPI2_MOSI. |
| **PB13** | J3-9 & J4-4 | **TFT Display SCK** | Shared | Display SPI2_SCK. |
| **PB12** | J3-10 & J4-5| **TFT Display CS** | Shared | Display SPI2_NSS / CS. |
| **PB14** | J3-8 & J4-6 | **TFT Display MISO** | Shared | Display SPI2_MISO. |
| **PB1** | J3-23 & J4-7| **TFT Display RS/DC**| Shared | Display Data/Command select GPIO. |
| **PB0** | J3-24 & J4-8| **TFT Display Backlight**| Shared | Display Backlight Control (TIM3_CH3 PWM). |

---

## 6. Power Distribution & Electrical Constraints

1. **5V Rail (J2 Pins 1 & 2)**:
   - Fed directly from the Micro-USB 5V line through Schottky diode D3 (500mA rating) or external 5V input header.
   - Used to supply the onboard AMS1117-3.3V regulator and can supply external 5V modules (sensors, 5V displays, relays).
2. **3.3V Rail (J2 Pins 3 & 4, J3 Pins 1 & 2, J4 Pin 1, J1 Pin 2)**:
   - Regulated +3.3V output from AMS1117-3.3 LDO (Max continuous output: 1.0 A).
   - Supplies the STM32 MCU, QSPI Flash, MicroSD, and TFT header.
3. **VDDA & VREF+ (J3 Pins 5 & 6)**:
   - **VDDA (J3-5)**: Analog supply voltage for ADC1/2/3, DAC1/2, OPAMP, and COMP.
   - **VREF+ (J3-6)**: Positive analog voltage reference. Connected to 3.3V by default. An external precision voltage reference (e.g. 2.048V, 2.5V, 3.0V) can be supplied if ultra-accurate analog conversion is required.
4. **5V Tolerance**:
   - Pins labeled **`5V` (FT)** are 5V tolerant when configured as digital inputs.
   - Pins labeled **`3.3V` (TT / Analog)** (e.g., PA4, PA5, PC2, PC3) must **NEVER** exceed `VDD + 0.3V` (approx 3.6V).
