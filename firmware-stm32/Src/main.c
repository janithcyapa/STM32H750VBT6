#include "bsp_board.h"

void SysTick_Handler(void) {
    HAL_IncTick();
}

int main(void)
{
    HAL_Init();
    BSP_LED_Init();
    BSP_Buttons_Init();

    while (1)
    {
        // If User Button K1 is held down, blink fast (100ms)
        // Otherwise blink normal (500ms)
        if (BSP_Button_K1_Read()) {
            BSP_LED_Toggle();
            HAL_Delay(100);
        } else {
            BSP_LED_Toggle();
            HAL_Delay(500);
        }
    }
}