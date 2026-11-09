# Set the minimum version of CMake that can be used
cmake_minimum_required(VERSION 3.20)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
set(CMAKE_C_COMPILER_FORCED TRUE)
set(CMAKE_CXX_COMPILER_FORCED TRUE)

# Toolchain setup
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/target.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/gcc-arm-none-eabi.cmake)

project(STM32H750VBT6 C CXX ASM)

set(PROJECT_NAME STM32H750VBT6)
add_executable(${PROJECT_NAME})

# Load architecture flags and project sources
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/flags.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/files.cmake)

# Modern target-scoped defines
target_compile_definitions(${PROJECT_NAME} PRIVATE
  STM32H750xx
  USE_HAL_DRIVER
)

# Optimize for size to conserve internal 128KB flash
target_compile_options(${PROJECT_NAME} PRIVATE -Os)

# Modern target-scoped includes
target_include_directories(${PROJECT_NAME} PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/Inc
  ${CMAKE_CURRENT_SOURCE_DIR}/Inc/core
  ${CMAKE_CURRENT_SOURCE_DIR}/Inc/bsp
  ${CMAKE_CURRENT_SOURCE_DIR}/Inc/sensors
  ${CMAKE_CURRENT_SOURCE_DIR}/Inc/usb
  ${CMAKE_CURRENT_SOURCE_DIR}/Inc/telemetry
  # HAL Includes
  ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Core/Include
  ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32H7xx/Include
  ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Inc
  # FreeRTOS Includes
  ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/FreeRTOS-Kernel/include
  ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/FreeRTOS-Kernel/portable/GCC/ARM_CM7/r0p1
  # USB Middleware Includes
  ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/ST/STM32_USB_Device_Library/Core/Inc
  ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc
  # FatFs Includes
  ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/FatFs
)

# Add the HAL C source files to the build
target_sources(${PROJECT_NAME} PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/Src/core/system_stm32h7xx.c
  ${CMAKE_CURRENT_SOURCE_DIR}/Src/bsp/bsp_board.c
  ${CMAKE_CURRENT_SOURCE_DIR}/Src/bsp/bsp_qspi.c
  ${CMAKE_CURRENT_SOURCE_DIR}/Src/bsp/bsp_sdcard.c
  ${CMAKE_CURRENT_SOURCE_DIR}/Src/bsp/sd_csv_logger.c
  ${CMAKE_CURRENT_SOURCE_DIR}/Src/sensors/bmi160.c
  ${CMAKE_CURRENT_SOURCE_DIR}/Src/sensors/i2c_bus.c
  ${CMAKE_CURRENT_SOURCE_DIR}/Src/sensors/bmp280.c
  ${CMAKE_CURRENT_SOURCE_DIR}/Src/sensors/qmc5883l.c
  ${CMAKE_CURRENT_SOURCE_DIR}/Src/telemetry/telemetry_uart.c
  # HAL Sources
  ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal.c
  ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_cortex.c
  ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc.c
  ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc_ex.c
  ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_gpio.c
  ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr.c
  ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr_ex.c
  ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_qspi.c
  ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_spi.c
  ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_spi_ex.c
  ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2c.c
  ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2c_ex.c
  ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_uart.c
  ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_uart_ex.c
  ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma.c
  ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma_ex.c
  ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_sd.c
  ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_sdmmc.c
  # FreeRTOS Core Sources
  ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/FreeRTOS-Kernel/tasks.c
  ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/FreeRTOS-Kernel/queue.c
  ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/FreeRTOS-Kernel/list.c
  ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/FreeRTOS-Kernel/timers.c
  ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/FreeRTOS-Kernel/event_groups.c
  ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/FreeRTOS-Kernel/stream_buffer.c
  # FreeRTOS Memory Manager (heap_4) & Cortex-M7 Port
  ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/FreeRTOS-Kernel/portable/MemMang/heap_4.c
  ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/FreeRTOS-Kernel/portable/GCC/ARM_CM7/r0p1/port.c
  # USB App Sources
  ${CMAKE_CURRENT_SOURCE_DIR}/Src/usb/usb_device.c
  ${CMAKE_CURRENT_SOURCE_DIR}/Src/usb/usbd_conf.c
  ${CMAKE_CURRENT_SOURCE_DIR}/Src/usb/usbd_desc.c
  ${CMAKE_CURRENT_SOURCE_DIR}/Src/usb/usbd_cdc_if.c
  # ST HAL USB & PCD Drivers
  ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pcd.c
  ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pcd_ex.c
  ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_usb.c
  # ST USB Middleware Core & CDC
  ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_core.c
  ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_ctlreq.c
  ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_ioreq.c
  ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Src/usbd_cdc.c
  # FatFs Sources
  ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/FatFs/ff.c
  ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/FatFs/diskio.c
)

target_link_libraries(${PROJECT_NAME} PRIVATE m)

# Post-build step to produce flashable artifacts and view memory usage
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}> ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.bin
  COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${PROJECT_NAME}> ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.hex
  COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${PROJECT_NAME}>
  COMMENT "Generating .bin, .hex, and printing memory allocation"
)

add_custom_target(flash
  COMMAND openocd -f interface/stlink.cfg -c "reset_config none" -f target/stm32h7x.cfg -c "program ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.elf verify reset exit"
  DEPENDS ${PROJECT_NAME}
  COMMENT "Flashing target over OpenOCD"
)
