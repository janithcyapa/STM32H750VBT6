#include "telemetry_uart.h"
#include "usbd_cdc_if.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <string.h>

#define TELEMETRY_RX_RING_SIZE 512

static UART_HandleTypeDef huart4 = {0};
static bool s_telemetry_ready = false;

static volatile uint8_t s_rx_ring[TELEMETRY_RX_RING_SIZE];
static volatile uint16_t s_rx_head = 0;
static volatile uint16_t s_rx_tail = 0;

static char s_latest_telemetry[160] = "NO_DATA";
static volatile uint32_t s_rx_packet_count = 0;
static volatile uint32_t s_last_rx_packet_tick = 0;

static Telemetry_Data_t s_telemetry_data = {0};
static uint16_t s_global_uav_id = 100;

bool Telemetry_UART_Init(uint32_t baudrate) {
    // 1. Configure Kernel Clock for UART4 (D2PCLK1 / APB1)
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_UART4;
    PeriphClkInitStruct.Usart234578ClockSelection = RCC_USART234578CLKSOURCE_D2PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
        return false;
    }

    // 2. Enable GPIO Port D and UART4 Peripheral Clocks
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_UART4_CLK_ENABLE();

    // 3. Configure GPIO Pins for UART4
    //    PD1 -> UART4_TX (AF8)
    //    PD0 -> UART4_RX (AF8)
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF8_UART4;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF8_UART4;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    // 4. Configure UART4 Peripheral
    huart4.Instance = UART4;
    huart4.Init.BaudRate = (baudrate > 0) ? baudrate : TELEMETRY_UART_BAUDRATE_DEFAULT;
    huart4.Init.WordLength = UART_WORDLENGTH_8B;
    huart4.Init.StopBits = UART_STOPBITS_1;
    huart4.Init.Parity = UART_PARITY_NONE;
    huart4.Init.Mode = UART_MODE_TX_RX;
    huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart4.Init.OverSampling = UART_OVERSAMPLING_16;
    huart4.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart4.Init.ClockPrescaler = UART_PRESCALER_DIV1;
    huart4.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

    if (HAL_UART_Init(&huart4) != HAL_OK) {
        return false;
    }

    // 5. Configure NVIC for UART4 Interrupts
    HAL_NVIC_SetPriority(UART4_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(UART4_IRQn);

    // 6. Enable RX Not Empty (RXNE) Interrupt
    __HAL_UART_ENABLE_IT(&huart4, UART_IT_RXNE);

    s_telemetry_ready = true;

    // 7. Send initial startup greeting text over UART4 (PD1)
    const char *greeting = "This is STM32H750 Telemetry UART Ready\r\n";
    HAL_UART_Transmit(&huart4, (const uint8_t *)greeting, (uint16_t)strlen(greeting), 100);

    return true;
}

bool Telemetry_UART_IsReady(void) {
    return s_telemetry_ready;
}

HAL_StatusTypeDef Telemetry_UART_Send(const uint8_t *data, uint16_t len) {
    if (!s_telemetry_ready || data == NULL || len == 0) {
        return HAL_ERROR;
    }
    return HAL_UART_Transmit(&huart4, data, len, 100);
}

HAL_StatusTypeDef Telemetry_UART_SendString(const char *str) {
    if (str == NULL) {
        return HAL_ERROR;
    }
    return Telemetry_UART_Send((const uint8_t *)str, (uint16_t)strlen(str));
}

void UART4_IRQHandler(void) {
    // Check if RXNE flag is set (data ready in RDR)
    if (__HAL_UART_GET_FLAG(&huart4, UART_FLAG_RXNE)) {
        uint8_t byte = (uint8_t)(huart4.Instance->RDR & 0xFF);
        uint16_t next_head = (s_rx_head + 1) % TELEMETRY_RX_RING_SIZE;
        if (next_head != s_rx_tail) {
            s_rx_ring[s_rx_head] = byte;
            s_rx_head = next_head;
        }
    }

    // Clear error flags to prevent lockup
    if (__HAL_UART_GET_FLAG(&huart4, UART_FLAG_ORE)) {
        __HAL_UART_CLEAR_FLAG(&huart4, UART_CLEAR_OREF);
    }
    if (__HAL_UART_GET_FLAG(&huart4, UART_FLAG_NE)) {
        __HAL_UART_CLEAR_FLAG(&huart4, UART_CLEAR_NEF);
    }
    if (__HAL_UART_GET_FLAG(&huart4, UART_FLAG_FE)) {
        __HAL_UART_CLEAR_FLAG(&huart4, UART_CLEAR_FEF);
    }
    if (__HAL_UART_GET_FLAG(&huart4, UART_FLAG_PE)) {
        __HAL_UART_CLEAR_FLAG(&huart4, UART_CLEAR_PEF);
    }
}

static uint64_t parse_u64(const char *p, const char **end) {
    uint64_t val = 0;
    while (*p == ' ') p++;
    while (*p >= '0' && *p <= '9') {
        val = val * 10 + (uint64_t)(*p - '0');
        p++;
    }
    if (end) *end = p;
    return val;
}

static int32_t parse_i32(const char *p, const char **end) {
    int32_t sign = 1;
    int32_t val = 0;
    while (*p == ' ') p++;
    if (*p == '-') { sign = -1; p++; }
    else if (*p == '+') { p++; }
    while (*p >= '0' && *p <= '9') {
        val = val * 10 + (*p - '0');
        p++;
    }
    if (end) *end = p;
    return val * sign;
}

void Telemetry_Parse_HL_RAW(const char *line) {
    if (!line || strncmp(line, "$HL_RAW,", 8) != 0) return;
    const char *p = line + 8;

    // Field 1: uav_id
    uint16_t uav_id = (uint16_t)parse_i32(p, &p);
    if (*p == ',') p++;

    // Field 2: anchor_id (1..5)
    int32_t anchor_id = parse_i32(p, &p);
    if (*p == ',') p++;
    if (anchor_id < 1 || anchor_id > 5) return;
    int idx = (int)(anchor_id - 1);

    // Field 3: t1_ps (uint64)
    uint64_t t1 = parse_u64(p, &p);
    if (*p == ',') p++;

    // Field 4: t2_ps (uint64)
    uint64_t t2 = parse_u64(p, &p);
    if (*p == ',') p++;

    // Field 5: t3_ps (uint64)
    uint64_t t3 = parse_u64(p, &p);
    if (*p == ',') p++;

    // Field 6: t4_ps (uint64)
    uint64_t t4 = parse_u64(p, &p);
    if (*p == ',') p++;

    // Field 7: rtt_ps (uint32)
    uint32_t rtt = (uint32_t)parse_u64(p, &p);
    if (*p == ',') p++;

    // Field 8: dist_cm (int32)
    int32_t dist = parse_i32(p, &p);
    if (*p == ',') p++;

    // Field 9: rssi (int16)
    int16_t rssi = (int16_t)parse_i32(p, &p);
    if (*p == ',') p++;

    // Field 10: ppm (int32)
    int32_t ppm = parse_i32(p, &p);

    taskENTER_CRITICAL();
    s_telemetry_data.anchors[idx].valid = true;
    s_telemetry_data.anchors[idx].last_tick = HAL_GetTick();
    s_telemetry_data.anchors[idx].uav_id = uav_id;
    s_telemetry_data.anchors[idx].anchor_id = (uint8_t)anchor_id;
    s_telemetry_data.anchors[idx].t1_ps = t1;
    s_telemetry_data.anchors[idx].t2_ps = t2;
    s_telemetry_data.anchors[idx].t3_ps = t3;
    s_telemetry_data.anchors[idx].t4_ps = t4;
    s_telemetry_data.anchors[idx].rtt_ps = rtt;
    s_telemetry_data.anchors[idx].dist_cm = dist;
    s_telemetry_data.anchors[idx].rssi = rssi;
    s_telemetry_data.anchors[idx].ppm = ppm;
    s_global_uav_id = uav_id;
    taskEXIT_CRITICAL();
}

static void SaveLatestPacket(const char *pkt) {
    if (pkt == NULL || pkt[0] == '\0') return;

    taskENTER_CRITICAL();
    strncpy(s_latest_telemetry, pkt, sizeof(s_latest_telemetry) - 1);
    s_latest_telemetry[sizeof(s_latest_telemetry) - 1] = '\0';
    s_rx_packet_count++;
    s_last_rx_packet_tick = HAL_GetTick();
    taskEXIT_CRITICAL();

    // Automatically parse $HL_RAW frames as they arrive
    Telemetry_Parse_HL_RAW(pkt);
}

void Telemetry_UART_Process(void) {
    static char line_buf[192];
    static uint16_t line_len = 0;
    static uint32_t last_rx_tick = 0;

    while (s_rx_head != s_rx_tail) {
        uint8_t c = s_rx_ring[s_rx_tail];
        s_rx_tail = (s_rx_tail + 1) % TELEMETRY_RX_RING_SIZE;
        last_rx_tick = HAL_GetTick();

        if (c == '\r') {
            continue; // discard carriage return
        } else if (c == '\n') {
            if (line_len > 0) {
                line_buf[line_len] = '\0';
                SaveLatestPacket(line_buf);
                line_len = 0;
            }
        } else {
            if (line_len < sizeof(line_buf) - 1) {
                line_buf[line_len++] = (char)c;
            } else {
                // Buffer full, save packet
                line_buf[line_len] = '\0';
                SaveLatestPacket(line_buf);
                line_len = 0;
            }
        }
    }

    // Flush any pending data if line was not terminated and 50ms elapsed
    if (line_len > 0 && (HAL_GetTick() - last_rx_tick > 50)) {
        line_buf[line_len] = '\0';
        SaveLatestPacket(line_buf);
        line_len = 0;
    }
}

bool Telemetry_UART_GetLatestPacket(char *out_buf, size_t max_len, uint32_t *age_ms) {
    if (out_buf == NULL || max_len == 0) return false;

    taskENTER_CRITICAL();
    strncpy(out_buf, s_latest_telemetry, max_len - 1);
    out_buf[max_len - 1] = '\0';
    if (age_ms != NULL) {
        if (s_rx_packet_count > 0) {
            *age_ms = HAL_GetTick() - s_last_rx_packet_tick;
        } else {
            *age_ms = 0xFFFFFFFF;
        }
    }
    bool has_data = (s_rx_packet_count > 0);
    taskEXIT_CRITICAL();

    return has_data;
}

bool Telemetry_UART_GetLatestData(Telemetry_Data_t *out, uint16_t *uav_id) {
    if (!out) return false;

    taskENTER_CRITICAL();
    memcpy(out, (const void *)&s_telemetry_data, sizeof(Telemetry_Data_t));
    if (uav_id) *uav_id = s_global_uav_id;
    taskEXIT_CRITICAL();

    uint32_t now = HAL_GetTick();
    bool any_fresh = false;
    for (int i = 0; i < 5; i++) {
        if (out->anchors[i].valid && (now - out->anchors[i].last_tick < 1500)) {
            any_fresh = true;
        }
    }
    return any_fresh;
}

uint32_t Telemetry_UART_GetRxCount(void) {
    return s_rx_packet_count;
}

void Task_Telemetry_Bridge(void *pvParameters) {
    (void)pvParameters;

    uint32_t hb_count = 0;
    TickType_t last_hb_tick = xTaskGetTickCount();

    // Small delay before initial active announcement on UART4 TX
    vTaskDelay(pdMS_TO_TICKS(500));
    Telemetry_UART_SendString("This is STM32H750 Telemetry UART Ready\r\n");

    while (1) {
        // 1. Process UART4 RX (PD0) from HiveLink
        Telemetry_UART_Process();

        // 2. Periodic hardcoded heartbeat over PD1 (UART4 TX) every 1000ms (silent to USB)
        if ((xTaskGetTickCount() - last_hb_tick) >= pdMS_TO_TICKS(1000)) {
            last_hb_tick = xTaskGetTickCount();
            hb_count++;

            char hb_msg[96];
            int hb_len = snprintf(hb_msg, sizeof(hb_msg),
                                  "[STM32 HEARTBEAT #%lu] Hello from STM32H750 (PD1 TX)!\r\n",
                                  hb_count);
            Telemetry_UART_Send((const uint8_t *)hb_msg, (uint16_t)hb_len);
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

