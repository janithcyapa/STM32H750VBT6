#include "stm32h7xx_hal.h"

// Required by HAL for HAL_Delay() timing
void SysTick_Handler(void)
{
    HAL_IncTick();
}

static void GPIO_Init(void)
{
    // Enable GPIOA clock
    __HAL_RCC_GPIOA_CLK_ENABLE();

    // Configure PA1 (User LED D2) as Output Push-Pull
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

int main(void)
{
    // Initialize Flash interface, SysTick, and NVIC
    HAL_Init();

    // Initialize LED GPIO
    GPIO_Init();

    while (1)
    {
        // Toggle User LED (D2 on PA1)
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_1);

        // 500 ms delay using HAL SysTick
        HAL_Delay(500);
    }
}