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

void Telemetry_UART_Process(void) {
    static char line_buf[128];
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
                char usb_msg[160];
                int n = snprintf(usb_msg, sizeof(usb_msg), "[TELEMETRY RX]: %s\r\n", line_buf);
                CDC_Transmit_FS((uint8_t *)usb_msg, (uint16_t)n);
                line_len = 0;
            }
        } else {
            if (line_len < sizeof(line_buf) - 1) {
                line_buf[line_len++] = (char)c;
            } else {
                // Buffer full, flush line
                line_buf[line_len] = '\0';
                char usb_msg[160];
                int n = snprintf(usb_msg, sizeof(usb_msg), "[TELEMETRY RX]: %s\r\n", line_buf);
                CDC_Transmit_FS((uint8_t *)usb_msg, (uint16_t)n);
                line_len = 0;
            }
        }
    }

    // Flush any pending data if line was not terminated and 50ms elapsed
    if (line_len > 0 && (HAL_GetTick() - last_rx_tick > 50)) {
        line_buf[line_len] = '\0';
        char usb_msg[160];
        int n = snprintf(usb_msg, sizeof(usb_msg), "[TELEMETRY RX]: %s\r\n", line_buf);
        CDC_Transmit_FS((uint8_t *)usb_msg, (uint16_t)n);
        line_len = 0;
    }
}

void Task_Telemetry_Bridge(void *pvParameters) {
    (void)pvParameters;

    uint32_t hb_count = 0;
    TickType_t last_hb_tick = xTaskGetTickCount();

    // Small delay before initial active announcement
    vTaskDelay(pdMS_TO_TICKS(500));
    Telemetry_UART_SendString("This is STM32H750 Telemetry UART Ready\r\n");

    while (1) {
        // 1. Process UART4 RX (PD0) from HiveLink and USB CDC -> UART4 TX queue
        Telemetry_UART_Process();

        // 2. Periodic hardcoded heartbeat over PD1 (UART4 TX) every 1000ms
        if ((xTaskGetTickCount() - last_hb_tick) >= pdMS_TO_TICKS(1000)) {
            last_hb_tick = xTaskGetTickCount();
            hb_count++;

            // Hardcoded heartbeat string for PD1 (HiveLink)
            char hb_msg[96];
            int hb_len = snprintf(hb_msg, sizeof(hb_msg),
                                  "[STM32 HEARTBEAT #%lu] Hello from STM32H750 (PD1 TX)!\r\n",
                                  hb_count);

            HAL_StatusTypeDef status = Telemetry_UART_Send((const uint8_t *)hb_msg, (uint16_t)hb_len);

            // Log heartbeat transmission status to USB CDC
            char usb_log[128];
            int ul_len = snprintf(usb_log, sizeof(usb_log),
                                  "[TELEMETRY TX #%lu] -> PD1: %s (%s)\r\n",
                                  hb_count,
                                  status == HAL_OK ? "SENT" : "FAILED",
                                  status == HAL_OK ? "OK" : "ERR");
            CDC_Transmit_FS((uint8_t *)usb_log, (uint16_t)ul_len);
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

