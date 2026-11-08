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

# Modern target-scoped includes
target_include_directories(${PROJECT_NAME} PRIVATE
  Inc
  # HAL Includes
  Drivers/CMSIS/Core/Include
  Drivers/CMSIS/Device/ST/STM32H7xx/Include
  Drivers/STM32H7xx_HAL_Driver/Inc
  # FreeRTOS Includes
  Middlewares/FreeRTOS-Kernel/include
  Middlewares/FreeRTOS-Kernel/portable/GCC/ARM_CM7/r0p1
  # USB Middleware Includes
  Middlewares/ST/STM32_USB_Device_Library/Core/Inc
  Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc
)

# Add the HAL C source files to the build
target_sources(${PROJECT_NAME} PRIVATE
  Src/bsp_board.c
  Src/bsp_qspi.c
  Src/system_stm32h7xx.c
  # HAL Sources
  Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal.c
  Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_cortex.c
  Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc.c
  Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc_ex.c
  Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_gpio.c
  Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr.c
  Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr_ex.c
  Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_qspi.c
  # FreeRTOS Core Sources
  Middlewares/FreeRTOS-Kernel/tasks.c
  Middlewares/FreeRTOS-Kernel/queue.c
  Middlewares/FreeRTOS-Kernel/list.c
  Middlewares/FreeRTOS-Kernel/timers.c
  Middlewares/FreeRTOS-Kernel/event_groups.c
  Middlewares/FreeRTOS-Kernel/stream_buffer.c
  # FreeRTOS Memory Manager (heap_4) & Cortex-M7 Port
  Middlewares/FreeRTOS-Kernel/portable/MemMang/heap_4.c
  Middlewares/FreeRTOS-Kernel/portable/GCC/ARM_CM7/r0p1/port.c
  # USB App Sources
  Src/usb_device.c
  Src/usbd_conf.c
  Src/usbd_desc.c
  Src/usbd_cdc_if.c
  # ST HAL USB & PCD Drivers
  Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pcd.c
  Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pcd_ex.c
  Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_usb.c
  # ST USB Middleware Core & CDC
  Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_core.c
  Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_ctlreq.c
  Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_ioreq.c
  Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Src/usbd_cdc.c
)

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
