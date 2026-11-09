#ifndef TELEMETRY_UART_H
#define TELEMETRY_UART_H

#include "stm32h7xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

#define TELEMETRY_UART_BAUDRATE_DEFAULT  115200

// Initialize UART4 on PD1 (TX) and PD0 (RX)
bool Telemetry_UART_Init(uint32_t baudrate);

// Check if Telemetry UART is initialized and ready
bool Telemetry_UART_IsReady(void);

// Transmit raw buffer over UART4
HAL_StatusTypeDef Telemetry_UART_Send(const uint8_t *data, uint16_t len);

// Transmit null-terminated string over UART4
HAL_StatusTypeDef Telemetry_UART_SendString(const char *str);

// Process received characters and forward completed lines/chunks to USB CDC
void Telemetry_UART_Process(void);

// FreeRTOS Task function for Telemetry Bridge
void Task_Telemetry_Bridge(void *pvParameters);

#endif // TELEMETRY_UART_H

