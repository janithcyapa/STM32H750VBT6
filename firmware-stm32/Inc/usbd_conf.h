#ifndef __USBD_CONF_H
#define __USBD_CONF_H

#include "stm32h7xx.h"
#include "stm32h7xx_hal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define USBD_MAX_NUM_INTERFACES     1
#define USBD_MAX_NUM_CONFIGURATION  1
#define USBD_MAX_STR_SPEC_SIZ       512
#define USBD_SUPPORT_USER_STRING_DESC 0
#define USBD_SELF_POWERED           1
#define USBD_DEBUG_LEVEL            0

/* CDC Class Config */
#define USBD_CDC_INTERVAL           1000

/* Memory management */
#define USBD_malloc               malloc
#define USBD_free                 free
#define USBD_memset               memset
#define USBD_memcpy               memcpy
#define USBD_Delay                HAL_Delay

#endif /* __USBD_CONF_H */