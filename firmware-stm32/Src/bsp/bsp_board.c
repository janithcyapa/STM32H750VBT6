#include "bsp_board.h"

void BSP_LED_Init(void)
{
    BSP_LED_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = BSP_LED_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(BSP_LED_PORT, &GPIO_InitStruct);

    // Default state: LED OFF (Since it's Active Low, HIGH = OFF)
    BSP_LED_Off();
}

void BSP_LED_Toggle(void)
{
    HAL_GPIO_TogglePin(BSP_LED_PORT, BSP_LED_PIN);
}

void BSP_LED_On(void)
{
    // Active low: Drive pin LOW to turn ON
    HAL_GPIO_WritePin(BSP_LED_PORT, BSP_LED_PIN, GPIO_PIN_RESET);
}

void BSP_LED_Off(void)
{
    // Active low: Drive pin HIGH to turn OFF
    HAL_GPIO_WritePin(BSP_LED_PORT, BSP_LED_PIN, GPIO_PIN_SET);
}

void BSP_Buttons_Init(void)
{
    BSP_BUTTON_K1_CLK_ENABLE();
    BSP_BUTTON_K2_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP; // Active low buttons need Pull-up
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    // Button K1 on PE3 (Active LOW)
    GPIO_InitStruct.Pin = BSP_BUTTON_K1_PIN;
    HAL_GPIO_Init(BSP_BUTTON_K1_PORT, &GPIO_InitStruct);

    // Button K2 on PC5 (Active LOW)
    GPIO_InitStruct.Pin = BSP_BUTTON_K2_PIN;
    HAL_GPIO_Init(BSP_BUTTON_K2_PORT, &GPIO_InitStruct);
}

uint8_t BSP_Button_K1_Read(void)
{
    // Returns 1 if pressed (pin is LOW), 0 if released
    return (HAL_GPIO_ReadPin(BSP_BUTTON_K1_PORT, BSP_BUTTON_K1_PIN) == GPIO_PIN_RESET) ? 1 : 0;
}

uint8_t BSP_Button_K2_Read(void)
{
    // Returns 1 if pressed (PC5 is LOW), 0 if released
    return (HAL_GPIO_ReadPin(BSP_BUTTON_K2_PORT, BSP_BUTTON_K2_PIN) == GPIO_PIN_RESET) ? 1 : 0;
}