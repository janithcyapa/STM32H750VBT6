# Autonomous Flight & Rover Controller: System Architecture & Hardware Specification

**Target Platform**: DevEBox STM32H7XX-M (STM32H750VBT6 Cortex-M7 @ 480 MHz)  
**Target Firmware Ecosystem**: PX4 Autopilot / Apache NuttX / FreeRTOS  
**Target Vehicles**: Quadcopters, Hexacopters, 2-Wheel Differential Rovers, 4-Wheel/8-Wheel Skid-Steer & Steered Rovers  
**Revision**: 2.0 (Dual-Magnetometer Architecture, Isolated Power Domains & Hardware Safety Revisions)

---

## 1. Executive Summary & Aim

The primary objective of this architecture is to transform the cost-effective, high-performance **DevEBox STM32H750VBT6** development board into a **universal, professional-grade autonomous vehicle controller** capable of running advanced PX4 Autopilot / NuttX flight and ground navigation stacks.

### Core Capabilities:
1. **Multi-Vehicle Scalability**: Up to **8 hardware-timed PWM / DShot motor outputs**, scaling seamlessly from a 4-motor Quadcopter or 6-motor Hexacopter to an 8-wheel robotic rover.
2. **High-Rate State Estimation**: A dedicated high-speed SPI bus running the **BMI160 IMU** with a hardware **Data-Ready Interrupt** (`PE4`) to feed the PX4 Extended Kalman Filter (EKF2) at 1 kHz with microsecond-level determinism.
3. **Dual-Magnetometer Architecture (Dual I2C Buses)**:
   - **Primary Compass (Mag 1)**: External GPS Mast Magnetometer on dedicated **I2C1** (`PB8`/`PB9`), physically elevated on a carbon mast away from motor and battery DC electromagnetic fields.
   - **Secondary / Frame Compass (Mag 2)**: Local frame magnetometer (**GY-271 / QMC5883L**) on dedicated **I2C2** (`PB10`/`PB11`), co-located with the **BMP280 Barometer** (`0x76`).
   - *Independent I2C buses prevent the fatal I2C address conflict* (`0x0D` address collision) that occurs when two QMC5883L/HMC5883L magnetometers share a single bus.
4. **High-Speed Companion Computer Link**: Direct hardware serial channel to a **Raspberry Pi 4/5** running ROS2 / MAVROS at 921,600 baud for offboard control, obstacle avoidance, and computer vision.
5. **Wireless Telemetry & Pilot Link**: Dedicated UARTs for long-range RC control (**ELRS / Crossfire** on UART7) and **ESP32** WiFi/Bluetooth bridge (on UART4).
6. **Preserved Onboard High-Speed Bus**: Complete preservation of the **8 MB Winbond Quad-SPI Flash** (for XIP application execution) and the **Native Micro-USB port** (for QGroundControl bench connection and DFU flashing).
7. **Robust Power Isolation & Protection**: External Schottky diode isolation for external 5V BEC rails, 10 kΩ pull-downs on all motor PWM gates, and 11:1 battery voltage sensing.

---

## 2. Master System Architecture Block Diagram

```mermaid
flowchart TD
    subgraph Core ["STM32H750VBT6 Core (480 MHz, 1024 KB SRAM)"]
        CPU["ARM Cortex-M7 CPU"]
        SRAM["1024 KB High-Speed SRAM"]
        QSPI_CTRL["Quad-SPI Controller"]
        USB_CORE["USB OTG_FS Core (Micro-USB)"]
        TIMERS["Hardware Timers (TIM1, TIM3)"]
        UARTS["Hardware UARTs (USART2, USART3, UART4, UART7)"]
        SPI_CORE["SPI Controllers (SPI1, SPI2)"]
        I2C_CTRL1["I2C1 Controller (PB8, PB9)"]
        I2C_CTRL2["I2C2 Controller (PB10, PB11)"]
        ADC_CORE["ADC1 (16-bit Precision)"]
    end

    subgraph Memory ["High-Capacity Storage"]
        W25Q64["Winbond W25Q64JV 8MB Flash<br>(Mapped at 0x90000000 XIP)"]
    end

    subgraph PrimarySensors ["Primary Flight Sensors"]
        BMI160["BMI160 6-Axis IMU<br>(Gyro + Accel via SPI1)"]
    end

    subgraph FrameSensors ["Frame Sensors (I2C2 Bus)"]
        BMP280["BMP280 Barometer<br>(Addr: 0x76, Altitude)"]
        GY271["GY-271 QMC5883L Compass (Mag 2)<br>(Addr: 0x0D, Backup/Bench)"]
    end

    subgraph MastSensors ["External GPS & Mast Puck (I2C1 Bus)"]
        GPS["u-blox M8N / M10 GNSS<br>(USART3 @ 115200)"]
        MAST_MAG["External Mast Compass (Mag 1)<br>(Addr: 0x0D, Primary Flight Heading)"]
    end

    subgraph Comms ["Communications & Companion"]
        RPi["Raspberry Pi Companion<br>(ROS2 / Vision @ 921.6k)"]
        ESP32["ESP32 Telemetry Bridge<br>(WiFi / MAVLink @ 115.2k)"]
        RC_RX["ELRS / CRSF Receiver<br>(Pilot Sticks @ 420k)"]
        PC_GCS["PC / QGroundControl<br>(Bench Calibration & DFU)"]
    end

    subgraph Actuators ["Actuator & Motor Outputs (1-8)"]
        M1_4["Motors 1-4 (TIM1 CH1-4)<br>PE9, PE11, PE13, PE14 + 10k Pull-Downs"]
        M5_8["Motors 5-8 (TIM3 CH1-4)<br>PC6, PC7, PB0, PB1 + 10k Pull-Downs"]
    end

    subgraph Power ["Power & Battery Sensing"]
        BATT_SENSE["Battery Voltage & Current<br>PA0 (11:1 Div), PC0 (3.3V Shunt Amp)"]
        EXT_BEC["External 5V 3A BEC<br>with Schottky Diode Isolation"]
    end

    %% Interconnects
    QSPI_CTRL <== "6-Wire QSPI (PB6, PB2, PD11-13, PE2)" ==> W25Q64
    SPI_CORE <== "SPI1 (PA4-PA7) + PE4 (INT1)" ==> BMI160
    I2C_CTRL2 <== "I2C2 (PB10, PB11)" ==> BMP280
    I2C_CTRL2 <== "I2C2 (PB10, PB11)" ==> GY271
    I2C_CTRL1 <== "I2C1 (PB8, PB9)" ==> MAST_MAG
    UARTS <== "USART3 (PD8, PD9)" ==> GPS
    UARTS <== "USART2 (PD5, PD6) @ 921.6k" ==> RPi
    UARTS <== "UART4 (PD0, PD1)" ==> ESP32
    UARTS <== "UART7 (PE7, PE8)" ==> RC_RX
    USB_CORE <== "Micro-USB (PA11, PA12)" ==> PC_GCS
    TIMERS ==> M1_4
    TIMERS ==> M5_8
    ADC_CORE <== BATT_SENSE
    EXT_BEC -.-> Core
```

---

## 3. Circuit Schematics & Interconnect Details

### A. Primary Flight IMU (BMI160 via SPI1 + Interrupt)

The BMI160 is wired as a **4-wire Hardware SPI device**. Dedicated Chip Select (`PA4`) and Data-Ready Interrupt (`PE4`) ensure deterministic, low-latency sampling.

```
       DevEBox Header 1                                BMI160 Breakout Module
   +-----------------------+                         +------------------------+
   | Pin 1 / 2 : +3.3V     |------------------------>| VIN                    |
   | Pin 3 / 4 : GND       |------------------------>| GND                    |
   | Pin 30    : PA4 (CS)  |------------------------>| CS  (Chip Select)      |
   | Pin 29    : PA5 (SCK) |------------------------>| SCL (SPI Clock)        |
   | Pin 27    : PA7 (MOSI)|------------------------>| SDA (SPI MOSI / DI)    |
   | Pin 28    : PA6 (MISO)|<------------------------| SAO (SPI MISO / DO)    |
   | Pin 42    : PE4 (INT) |<------------------------| INT1 (Data Ready Pulse)|
   +-----------------------+                         +------------------------+
```

> [!TIP]
> **Wiring Practice**: High-speed SPI at 10 MHz is sensitive to parasitic capacitance. Keep jumper wires under 5 cm, or use twisted-pair ribbon with GND between signals.

---

### B. Dual-Magnetometer & Sensor Buses (I2C1 & I2C2)

#### The Address Collision Problem
Standard magnetometers (GY-271 with QMC5883L, HMC5883L, IST8310) hardcode their 7-bit I2C slave address to **`0x0D`** (or `0x1E`). If an external GPS mast puck and an onboard magnetometer are wired to the same I2C bus, both acknowledge simultaneously, corrupting every single reading.

#### The Dual-Bus Solution:
1. **Mast Compass (Mag 1 - Primary)** on **I2C1** (`PB8` / `PB9` on Header 2). Elevated on a mast away from battery high-current loops and motor ESC switching noise.
2. **Frame Compass (Mag 2 - Secondary)** + **BMP280 Barometer** on **I2C2** (`PB10` / `PB11` on Header 1).

```
   DevEBox Header 2 (I2C1)                           External GPS Mast Puck
   +-----------------------+                         +----------------------------+
   | Pin 1 / 2 : +5V (BEC) |------------------------>| 5V VCC                     |
   | Pin 5 / 6 : GND       |------------------------>| GND                        |
   | Pin 42    : PB8 (SCL) |------------------------>| Compass SCL (Addr: 0x0D)   |
   | Pin 41    : PB9 (SDA) |------------------------>| Compass SDA (Addr: 0x0D)   |
   | Pin 8     : PD8 (TX)  |------------------------>| GPS RX                     |
   | Pin 7     : PD9 (RX)  |<------------------------| GPS TX                     |
   +-----------------------+                         +----------------------------+

   DevEBox Header 1 (I2C2)                           Frame Sensors (Baro + Mag 2)
   +-----------------------+                         +----------------------------+
   | Pin 1 / 2 : +3.3V     |---+-------------------->| BMP280 VCC                 |
   | Pin 3 / 4 : GND       |---|---+---------------->| BMP280 GND                 |
   | Pin 12    : PB10(SCL) |---|---|---+------------>| BMP280 SCL (Addr: 0x76)    |
   | Pin 11    : PB11(SDA) |---|---|---|---+-------->| BMP280 SDA                 |
   +-----------------------+   |   |   |   |         +----------------------------+
                               |   |   |   |
                               |   |   |   |          Local GY-271 Mag (Addr: 0x0D)
                               |   |   |   |         +----------------------------+
                               +---|---|---|-------->| VCC (3.3V)                 |
                                   +---|---|-------->| GND                        |
                                       +---|-------->| SCL                        |
                                           +-------->| SDA                        |
                                                     +----------------------------+
```

---

### C. Actuator & Motor Output Grouping (TIM1 & TIM3)

All motor outputs must have **10 kΩ pull-down resistors to GND** connected at the signal gate. This keeps the ESC control lines grounded while the MCU boots or when pins float during reset, preventing accidental motor spins.

```
DevEBox Pins                     10k Pull-down              Vehicle Mapping
PE9  (Pin 19, H1, TIM1_CH1) ─────┬───[ 10k ]─── GND ───────> Motor 1 / Front-Right / Rover Left 1
                                 │
PE11 (Pin 17, H1, TIM1_CH2) ─────┼───[ 10k ]─── GND ───────> Motor 2 / Rear-Left   / Rover Left 2
                                 │
PE13 (Pin 15, H1, TIM1_CH3) ─────┼───[ 10k ]─── GND ───────> Motor 3 / Front-Left  / Rover Left 3
                                 │
PE14 (Pin 14, H1, TIM1_CH4) ─────┼───[ 10k ]─── GND ───────> Motor 4 / Rear-Right  / Rover Left 4
                                 │
PC6  (Pin 16, H2, TIM3_CH1) ─────┼───[ 10k ]─── GND ───────> Motor 5 / Hexa M5     / Rover Right 1
                                 │
PC7  (Pin 15, H2, TIM3_CH2) ─────┼───[ 10k ]─── GND ───────> Motor 6 / Hexa M6     / Rover Right 2
                                 │
PB0  (Pin 24, H1, TIM3_CH3) ─────┼───[ 10k ]─── GND ───────> Motor 7 / Octo M7     / Rover Right 3
                                 │
PB1  (Pin 23, H1, TIM3_CH4) ─────┴───[ 10k ]─── GND ───────> Motor 8 / Octo M8     / Rover Right 4
```

---

### D. Communication Ports Interconnect

```
1. Raspberry Pi Companion Computer (USART2 @ 921,600 Baud)
   DevEBox Header 2                                  Raspberry Pi (40-Pin Header)
   Pin 34 : PD5 (USART2_TX) ------------------------> Pin 10 : GPIO 15 (UART_RX)
   Pin 33 : PD6 (USART2_RX) <------------------------ Pin 8  : GPIO 14 (UART_TX)
   Pin 5  : GND             ------------------------- Pin 6  : GND

2. ESP32 Wireless Telemetry Bridge (UART4 @ 115,200 Baud)
   DevEBox Header 2                                  ESP32 Dev Module
   Pin 30 : PD1 (UART4_TX)  ------------------------> RXD0 / GPIO 3
   Pin 27 : PD0 (UART4_RX)  <------------------------ TXD0 / GPIO 1
   Pin 5  : GND             ------------------------- GND

3. RC Radio Receiver: ELRS / Crossfire (UART7 @ 420,000 Baud)
   DevEBox Header 1                                  Receiver (ELRS / CRSF)
   Pin 20 : PE8 (UART7_TX)  ------------------------> RX (Telemetry out)
   Pin 21 : PE7 (UART7_RX)  <------------------------ TX (Channel data in)
   Pin 1  : 3V3 (or Ext 5V) ------------------------> VCC
   Pin 3  : GND             ------------------------> GND
```

---

### E. Battery Voltage & Current Sensing Circuit (ADC1)

To safely measure a 3S–8S LiPo battery (11.1V – 33.6V) using the STM32 3.3V ADC inputs, an external precision divider with low-pass filtering is required:

```
  LiPo (+) Battery Rail
         │
        ┌┴┐
        │ │ R1 = 10 kΩ (0.1% or 1% Metal Film)
        └┬┘
         ├──────────────────────────────> DevEBox PA0 (Header 1, Pin 34)
        ┌┴┐       │                        (ADC1_INP16: Voltage Sense)
        │ │ R2    ═ C1 (100 nF Ceramic)
        │ │ 1 kΩ  │
        └┬┘       │
         ├────────┴─────────────────────> System GND
         │
  
  Division Ratio: V_out = V_batt * (1k / (10k + 1k)) = V_batt / 11
  Max measurable voltage = 3.3V * 11 = 36.3V (Safe for up to 8S LiPo!)

  Current Sensor (3.3V-scaled Hall/Shunt Output):
  Current Sensor V_OUT ─────────────────> DevEBox PC0 (Header 1, Pin 38)
                                           (ADC1_INP10: Current Sense)
```

> [!WARNING]
> **Current Sensor Voltage Clamp**: 5V current sensors (like bare ACS712-05B modules) output up to 4.5V at full scale. Direct connection will damage STM32 ADC pins! Use a standard drone power module (e.g., PM02/PM07) that outputs 0–3.3V, or add a 2:1 resistive divider on `PC0`.

---

### F. Power Architecture & Back-Feed Prevention

```
                     +---------------------------------------+
                     |           LiPo Battery (3S-8S)        |
                     +-------------------+-------------------+
                                         |
                       +-----------------+-----------------+
                       |                                   |
                +------v-------+                   +-------v-------+
                | 5V 3A UBEC   |                   |  High-Current |
                | (Flight Rail)|                   |  ESC / Motors |
                +------+-------+                   +---------------+
                       |
                  [ Schottky ] (SS14 or BAT54: Prevents USB Back-Feed)
                       |
                       +───────────────────────────> DevEBox 5V Pin (Header 2, Pin 1-2)
                                                                 │
                                                       [ AMS1117-3.3V LDO ]
                                                                 │
                                                                 v
                                                    3.3V MCU & Sensor Rail
```

> [!CAUTION]
> **DevEBox Diode D3 Note**: Diode `D3` on the DevEBox schematic is a 5.1V Zener clamp, NOT a reverse-blocking Schottky diode. If you connect an external 5V BEC directly to the 5V pins while a PC is plugged into the Micro-USB port, current will back-feed through `L1` into your computer's USB port! Always install an external series Schottky diode on the BEC 5V output.

---

## 4. Master Pinout Table & Header Mapping

### Header 1 (Left 44-Pin Connector)

| Pin # | Label | STM32 Pin | Assigned Function | Connected Device / Subsystem |
|---|---|---|---|---|
| **1, 2** | 3V3 | +3.3V Rail | Power Supply | 3.3V Sensor Power Bus |
| **3, 4** | GND | Ground | Ground | Common System Ground |
| **5** | VDDA | VDDA | Analog Power | Filtered 3.3V for ADC |
| **6** | VREF+ | VREF+ | ADC Reference | ADC Voltage Reference |
| **11** | PB11 | **PB11** | `I2C2_SDA` | **Frame Sensors: GY-271 (Mag 2) + BMP280** |
| **12** | PB10 | **PB10** | `I2C2_SCL` | **Frame Sensors: GY-271 (Mag 2) + BMP280** |
| **14** | PE14 | **PE14** | `TIM1_CH4` | **Motor 4 Output** (PE14 + 10k Pull-down) |
| **15** | PE13 | **PE13** | `TIM1_CH3` | **Motor 3 Output** (PE13 + 10k Pull-down) |
| **17** | PE11 | **PE11** | `TIM1_CH2` | **Motor 2 Output** (PE11 + 10k Pull-down) |
| **19** | PE9 | **PE9** | `TIM1_CH1` | **Motor 1 Output** (PE9 + 10k Pull-down) |
| **20** | PE8 | **PE8** | `UART7_TX` | **RC Receiver Telemetry** |
| **21** | PE7 | **PE7** | `UART7_RX` | **RC Receiver Data (ELRS / CRSF)** |
| **23** | PB1 | **PB1** | `TIM3_CH4` | **Motor 8 Output** (PB1 + 10k Pull-down) |
| **24** | PB0 | **PB0** | `TIM3_CH3` | **Motor 7 Output** (PB0 + 10k Pull-down) |
| **27** | PA7 | **PA7** | `SPI1_MOSI` | **BMI160 IMU: SDA (MOSI)** |
| **28** | PA6 | **PA6** | `SPI1_MISO` | **BMI160 IMU: SAO (MISO)** |
| **29** | PA5 | **PA5** | `SPI1_SCK` | **BMI160 IMU: SCL (SCK)** |
| **30** | PA4 | **PA4** | `SPI1_NSS` | **BMI160 IMU: CS (Chip Select)** |
| **33** | PA1 | **PA1** | GPIO Output | **Onboard Blue User LED (D2)** |
| **34** | PA0 | **PA0** | `ADC1_INP16` | **Battery Voltage Sense** (11:1 Divider) |
| **38** | PC0 | **PC0** | `ADC1_INP10` | **Battery Current Sense** (0–3.3V scaled) |
| **42** | PE4 | **PE4** | `EXTI4` | **BMI160 IMU: INT1 (Data Ready Interrupt)** |
| **43** | PE3 | **PE3** | GPIO Input | **Onboard User Button K1** |
| **22, 44**| PB2, PE2 | PB2, PE2 | QSPI Bus | *Reserved for Onboard W25Q64JV Flash* |

---

### Header 2 (Right 44-Pin Connector)

| Pin # | Label | STM32 Pin | Assigned Function | Connected Device / Subsystem |
|---|---|---|---|---|
| **1, 2** | 5V | +5V Rail | Power Supply | 5V Power for GPS, RC, and Sensors |
| **3, 4** | 3V3 | +3.3V Rail | Power Supply | 3.3V Logic Reference |
| **5, 6** | GND | Ground | Ground | Common System Ground |
| **7** | PD9 | **PD9** | `USART3_RX` | **GPS Module: TX Output** |
| **8** | PD8 | **PD8** | `USART3_TX` | **GPS Module: RX Input** |
| **15** | PC7 | **PC7** | `TIM3_CH2` | **Motor 6 Output** (PC7 + 10k Pull-down) |
| **16** | PC6 | **PC6** | `TIM3_CH1` | **Motor 5 Output** (PC6 + 10k Pull-down) |
| **19** | PA9 | **PA9** | `USART1_TX` | *Debug UART Console (Optional / FTDI)* |
| **21** | PA11 | **PA11** | `USB_OTG_FS_DM`| **Onboard Micro-USB Data Minus** |
| **22** | PA10 | **PA10** | `USART1_RX` | *Debug UART Console (Optional / FTDI)* |
| **24** | PA12 | **PA12** | `USB_OTG_FS_DP`| **Onboard Micro-USB Data Plus** |
| **27** | PD0 | **PD0** | `UART4_RX` | **ESP32 Telemetry: TX Output** |
| **30** | PD1 | **PD1** | `UART4_TX` | **ESP32 Telemetry: RX Input** |
| **33** | PD6 | **PD6** | `USART2_RX` | **Raspberry Pi: TX (GPIO 14)** |
| **34** | PD5 | **PD5** | `USART2_TX` | **Raspberry Pi: RX (GPIO 15)** |
| **41** | PB9 | **PB9** | `I2C1_SDA` | **GPS Mast Compass (Mag 1 SDA)** |
| **42** | PB8 | **PB8** | `I2C1_SCL` | **GPS Mast Compass (Mag 1 SCL)** |
| **9-12** | PD11-13, PB6 | - | QSPI Bus | *Reserved for Onboard W25Q64JV Flash* |

---

## 5. Software Architecture & PX4 Porting Strategy

### The 128 KB Internal Flash Challenge
The STM32H750VBT6 contains only **128 KB of internal NOR flash**, but features **1024 KB of ultra-fast internal SRAM** and the onboard **8 MB Winbond W25Q64JV** Quad-SPI NOR Flash. Standard PX4 flight images average 1.5–2.0 MB, meaning the full PX4 OS cannot fit in internal flash alone.

### Two-Stage Bootloader Implementation
```
 +──────────────────────────────────────────────────────────────────────────+
 | 1. Stage-1 Internal Flash Bootloader (<= 128 KB)                          |
 |    - Brought up at reset (0x08000000)                                    |
 |    - Initializes 480 MHz Core Clock & Power Domains                      |
 |    - Configures QUADSPI Peripheral (PB2, PB6, PD11-13, PE2)              |
 |    - Sends Winbond W25Q64 Fast Read Quad I/O (0xEB) Command              |
 |    - Switches QSPI into Memory-Mapped Mode (XIP at 0x90000000)           |
 |    - Configures Arm Cortex-M7 MPU Cache Attributes (Normal, Cacheable)   |
 |    - Validates PX4 image checksum & jumps to 0x90000000                  |
 +──────────────────────────────────────────────────────────────────────────+
                                      │
                                      ▼
 +──────────────────────────────────────────────────────────────────────────+
 | 2. PX4 Autopilot / NuttX OS Execution (XIP from QSPI or RAM Execution)   |
 |    - Critical sensor interrupts & EKF2 loops run with Cortex-M7 I/D Cache|
 |    - Flash write/erase routines (parameters) run from SRAM (`ramfunc`)   |
 |    - USB VBUS Sense disabled in NuttX (board lacks VBUS on PA9)          |
 +──────────────────────────────────────────────────────────────────────────+
```

---

## 6. Interactive Deliverables

- **Visual Circuit & Schematic Interconnect (SVG)**: [`flight_rover_schematic.svg`](file:///home/jazz/Projects/STM32H750VBT6/Hardware/schematics/flight_rover_schematic.svg)
- **Interactive Circuit Viewer**: [`flight_rover_viewer.html`](file:///home/jazz/Projects/STM32H750VBT6/Hardware/schematics/flight_rover_viewer.html)
- **Pinout & Alternate Functions Reference**: [`pinout-diagram.svg`](file:///home/jazz/Projects/STM32H750VBT6/Hardware/schematics/pinout-diagram.svg)
- **Interactive Pinout Viewer**: [`pinout_viewer.html`](file:///home/jazz/Projects/STM32H750VBT6/Hardware/schematics/pinout_viewer.html)
