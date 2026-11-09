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

// Process received characters and store completed telemetry packets
void Telemetry_UART_Process(void);

// Retrieve latest telemetry string packet (e.g. "AP1-> RSSI: -52, FTM: 1185 | AP2-> RSSI: -47, FTM: 630")
// Returns true if a packet has ever been received, false otherwise.
// age_ms outputs the milliseconds since the packet was received.
bool Telemetry_UART_GetLatestPacket(char *out_buf, size_t max_len, uint32_t *age_ms);

// Get total count of complete telemetry packets received
uint32_t Telemetry_UART_GetRxCount(void);

// Parsed Telemetry Data per Anchor from $HL_RAW frames
typedef struct {
    bool     valid;
    uint32_t last_tick;
    uint16_t uav_id;
    uint8_t  anchor_id;
    uint64_t t1_ps;     // 64-bit Anchor frame TX timestamp (ps)
    uint64_t t2_ps;     // 64-bit UAV frame RX timestamp (ps)
    uint64_t t3_ps;     // 64-bit UAV ACK TX timestamp (ps)
    uint64_t t4_ps;     // 64-bit Anchor ACK RX timestamp (ps)
    uint32_t rtt_ps;    // 32-bit Round-Trip Time in ps
    int32_t  dist_cm;   // 32-bit Calculated distance in cm
    int16_t  rssi;      // 8-bit/16-bit RSSI in dBm
    int32_t  ppm;       // Clock frequency drift offset in PPM
} Telemetry_Anchor_t;

typedef struct {
    Telemetry_Anchor_t anchors[5]; // Up to 5 Anchors (index 0=A1 .. 4=A5)
} Telemetry_Data_t;

// Retrieve snapshot of all parsed anchor data
bool Telemetry_UART_GetLatestData(Telemetry_Data_t *out, uint16_t *uav_id);

// Parse an incoming raw $HL_RAW frame into anchor structure
void Telemetry_Parse_HL_RAW(const char *line);

// FreeRTOS Task function for Telemetry Bridge
void Task_Telemetry_Bridge(void *pvParameters);

#endif // TELEMETRY_UART_H

