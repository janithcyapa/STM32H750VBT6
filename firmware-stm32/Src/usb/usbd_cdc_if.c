#include "usbd_cdc_if.h"
#include "telemetry_uart.h"

uint8_t UserRxBufferFS[APP_RX_DATA_SIZE];
uint8_t UserTxBufferFS[APP_TX_DATA_SIZE];

extern USBD_HandleTypeDef hUsbDeviceFS;

static int8_t CDC_Init_FS(void);
static int8_t CDC_DeInit_FS(void);
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t *pbuf, uint16_t length);
static int8_t CDC_Receive_FS(uint8_t *pbuf, uint32_t *Len);
static int8_t CDC_TransmitCplt_FS(uint8_t *Buf, uint32_t *Len, uint8_t epnum) {
  (void)Buf;
  (void)Len;
  (void)epnum;
  return (USBD_OK);
}

USBD_CDC_ItfTypeDef USBD_Interface_fops_FS = {CDC_Init_FS, CDC_DeInit_FS,
                                              CDC_Control_FS, CDC_Receive_FS,
                                              CDC_TransmitCplt_FS};

static int8_t CDC_Init_FS(void) {
  USBD_CDC_SetTxBuffer(&hUsbDeviceFS, UserTxBufferFS, 0);
  USBD_CDC_SetRxBuffer(&hUsbDeviceFS, UserRxBufferFS);
  return (USBD_OK);
}

static int8_t CDC_DeInit_FS(void) { return (USBD_OK); }

static int8_t CDC_Control_FS(uint8_t cmd, uint8_t *pbuf, uint16_t length) {
  (void)cmd;
  (void)pbuf;
  (void)length;
  return (USBD_OK);
}

#define USB_CMD_BUF_SIZE 256
static char s_usb_cmd_buf[USB_CMD_BUF_SIZE];
static uint16_t s_usb_cmd_len = 0;
static volatile bool s_usb_cmd_ready = false;

static int8_t CDC_Receive_FS(uint8_t *Buf, uint32_t *Len) {
  uint32_t len = *Len;
  for (uint32_t i = 0; i < len; i++) {
    char c = (char)Buf[i];
    if (c == '\r' || c == '\n') {
      if (s_usb_cmd_len > 0) {
        s_usb_cmd_buf[s_usb_cmd_len] = '\0';
        s_usb_cmd_ready = true;
      }
    } else if (!s_usb_cmd_ready && s_usb_cmd_len < (USB_CMD_BUF_SIZE - 1)) {
      s_usb_cmd_buf[s_usb_cmd_len++] = c;
    }
  }

  // Bridge USB RX data over to Telemetry UART (PD1 TX)
  Telemetry_UART_Send(Buf, (uint16_t)*Len);

  USBD_CDC_SetRxBuffer(&hUsbDeviceFS, &Buf[0]);
  USBD_CDC_ReceivePacket(&hUsbDeviceFS);
  return (USBD_OK);
}

bool CDC_HasCommand(void) {
  return s_usb_cmd_ready;
}

bool CDC_GetCommand(char *out_cmd, size_t max_len) {
  if (!s_usb_cmd_ready || out_cmd == NULL || max_len == 0) {
    return false;
  }
  strncpy(out_cmd, s_usb_cmd_buf, max_len - 1);
  out_cmd[max_len - 1] = '\0';
  s_usb_cmd_len = 0;
  s_usb_cmd_ready = false;
  return true;
}

uint8_t CDC_Transmit_FS(uint8_t *Buf, uint16_t Len) {
  uint8_t result = USBD_OK;
  USBD_CDC_HandleTypeDef *hcdc =
      (USBD_CDC_HandleTypeDef *)hUsbDeviceFS.pClassData;
  if (hcdc == NULL) {
    return USBD_FAIL;
  }

  // Wait for previous transmission to complete (up to 25ms timeout)
  uint32_t start = HAL_GetTick();
  while (hcdc->TxState != 0) {
    if ((HAL_GetTick() - start) > 25) {
      return USBD_BUSY;
    }
  }

  USBD_CDC_SetTxBuffer(&hUsbDeviceFS, Buf, Len);
  result = USBD_CDC_TransmitPacket(&hUsbDeviceFS);
  return result;
}

bool CDC_Transmit_FS_Blocking(const uint8_t *Buf, uint16_t Len, uint32_t timeout_ms) {
  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef *)hUsbDeviceFS.pClassData;
  if (hcdc == NULL || Buf == NULL || Len == 0) return false;

  uint32_t start = HAL_GetTick();
  while (hcdc->TxState != 0) {
    if ((HAL_GetTick() - start) > timeout_ms) {
      return false;
    }
  }

  USBD_CDC_SetTxBuffer(&hUsbDeviceFS, (uint8_t *)Buf, Len);
  if (USBD_CDC_TransmitPacket(&hUsbDeviceFS) != USBD_OK) {
    return false;
  }

  start = HAL_GetTick();
  while (hcdc->TxState != 0) {
    if ((HAL_GetTick() - start) > timeout_ms) {
      return false;
    }
  }
  return true;
}