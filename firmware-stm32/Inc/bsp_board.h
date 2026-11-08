#ifndef BSP_BOARD_H
#define BSP_BOARD_H

#include "stm32h7xx_hal.h"

/* --- Onboard LED Aliases (D2) --- */
#define BSP_LED_PIN               GPIO_PIN_1
#define BSP_LED_PORT              GPIOA
#define BSP_LED_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()

/* --- Onboard Push Button Aliases --- */
// User Button K1 (PE3, Active LOW)
#define BSP_BUTTON_K1_PIN         GPIO_PIN_3
#define BSP_BUTTON_K1_PORT        GPIOE
#define BSP_BUTTON_K1_CLK_ENABLE() __HAL_RCC_GPIOE_CLK_ENABLE()

// User Button K2 (PC5, Active LOW)
#define BSP_BUTTON_K2_PIN         GPIO_PIN_5
#define BSP_BUTTON_K2_PORT        GPIOC
#define BSP_BUTTON_K2_CLK_ENABLE() __HAL_RCC_GPIOC_CLK_ENABLE()

/* --- BSP Functions --- */
void BSP_LED_Init(void);
void BSP_LED_Toggle(void);
void BSP_LED_On(void);
void BSP_LED_Off(void);

void BSP_Buttons_Init(void);
uint8_t BSP_Button_K1_Read(void);
uint8_t BSP_Button_K2_Read(void);

#endif // BSP_BOARD_H