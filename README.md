# DevEBox STM32H7XX-M (STM32H750VBT6) Board Repository

A comprehensive, board-centric repository for the **DevEBox STM32H7XX-M** development board based on the **STM32H750VBT6** microcontroller (ARM Cortex-M7 @ 400/480 MHz).

---

## Repository Structure

```
.
├── Hardware/               # Hardware schematics, datasheets, and pin mapping
│   ├── PINOUT.md           # Authoritative pinout reference table
│   ├── README.md           # Hardware overview
│   ├── datasheets/         # AMS1117 LDO, W25Q64JV Flash datasheets
│   ├── schematics/         # Board schematics PDF
│   └── reference_docs/     # Reference captures and documentation
│
├── bootloader/             # Stage-1 Bootloader (PX4 / MCUboot / Custom DFU)
│
├── firmware-stm32/         # Bare-metal & Official ST HAL firmware (CMake based)
│
├── firmware-zephyr/        # Zephyr RTOS out-of-tree board definition & samples
│
├── firmware-nuttx/         # Apache NuttX / PX4 Autopilot board definition
│
└── tools/                  # Flashing, OpenOCD scripts, and udev rules
```

---

## Hardware Quick Reference

- **MCU**: STM32H750VBT6 (128 KB internal Flash, 1 MB internal SRAM)
- **External Flash**: Winbond W25Q64JV (8 MB Quad-SPI)
- **User LED (D2)**: `PA1` (Active LOW / Current Sink)
- **User Button 1 (K1)**: `PE3` (Active LOW)
- **User Button 2 (K2)**: `PC5` (Active LOW)
- **HSE Crystal**: 25.0 MHz
- **LSE Crystal**: 32.768 kHz
- **Debug Port**: SWD on Header J1 (`PA13` SWDIO, `PA14` SWCLK)

See [`Hardware/PINOUT.md`](./Hardware/PINOUT.md) for full pin tables and bus connections.
