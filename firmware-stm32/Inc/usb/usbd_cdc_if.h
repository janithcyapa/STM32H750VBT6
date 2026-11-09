#ifndef __USBD_CDC_IF_H__
#define __USBD_CDC_IF_H__

#include "usbd_cdc.h"
#include <stdbool.h>
#include <stddef.h>

#define APP_RX_DATA_SIZE 2048
#define APP_TX_DATA_SIZE 2048

extern USBD_CDC_ItfTypeDef USBD_Interface_fops_FS;

uint8_t CDC_Transmit_FS(uint8_t *Buf, uint16_t Len);
bool CDC_Transmit_FS_Blocking(const uint8_t *Buf, uint16_t Len, uint32_t timeout_ms);

// Check if a line-terminated command was received over USB CDC
bool CDC_HasCommand(void);

// Retrieve the received command
bool CDC_GetCommand(char *out_cmd, size_t max_len);

#endif /* __USBD_CDC_IF_H__ */