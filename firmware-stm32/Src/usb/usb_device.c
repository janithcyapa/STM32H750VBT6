#include "usb_device.h"
#include "usbd_cdc.h"
#include "usbd_cdc_if.h"
#include "usbd_core.h"
#include "usbd_desc.h"

USBD_HandleTypeDef hUsbDeviceFS;

void MX_USB_DEVICE_Init(void) {
  // 1. Force USB host re-enumeration: drive PA12 (DP) LOW for 50ms
  __HAL_RCC_GPIOA_CLK_ENABLE();
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
  HAL_Delay(50);

  // 2. Power internal USB 3.3V PHY transceiver and wait for ready
  HAL_PWREx_EnableUSBVoltageDetector();
  while (!__HAL_PWR_GET_FLAG(PWR_FLAG_USB33RDY))
    ;

  // 3. Enable HSI48 internal 48MHz clock for USB
  __HAL_RCC_HSI48_ENABLE();
  while (!__HAL_RCC_GET_FLAG(RCC_FLAG_HSI48RDY))
    ;

  // 4. Route HSI48 to USB Peripheral
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);

  // 5. Initialize USB Device Stack
  USBD_Init(&hUsbDeviceFS, &FS_Desc, 0);
  USBD_RegisterClass(&hUsbDeviceFS, &USBD_CDC);
  USBD_CDC_RegisterInterface(&hUsbDeviceFS, &USBD_Interface_fops_FS);
  USBD_Start(&hUsbDeviceFS);
}