#include "usb_device.h"
#include "usbd_cdc.h"
#include "usbd_cdc_if.h"
#include "usbd_core.h"
#include "usbd_desc.h"

USBD_HandleTypeDef hUsbDeviceFS;

void MX_USB_DEVICE_Init(void) {
  // 1. CRITICAL for STM32H7: Power the internal USB 3.3V PHY transceiver
  HAL_PWREx_EnableUSBVoltageDetector();

  // 2. Enable HSI48 internal 48MHz clock for USB
  __HAL_RCC_HSI48_ENABLE();
  while (!__HAL_RCC_GET_FLAG(RCC_FLAG_HSI48RDY))
    ;

  // 3. Route HSI48 to USB Peripheral
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);

  // 4. Initialize USB Device Stack
  USBD_Init(&hUsbDeviceFS, &FS_Desc, 0);
  USBD_RegisterClass(&hUsbDeviceFS, &USBD_CDC);
  USBD_CDC_RegisterInterface(&hUsbDeviceFS, &USBD_Interface_fops_FS);
  USBD_Start(&hUsbDeviceFS);
}