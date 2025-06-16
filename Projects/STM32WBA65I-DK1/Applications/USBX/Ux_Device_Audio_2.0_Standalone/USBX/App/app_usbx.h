/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_usbx.h
  * @author  MCD Application Team
  * @brief   USBX applicative header file
  ******************************************************************************
   * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_USBX_H__
#define __APP_USBX_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "ux_api.h"

#include "app_usbx_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "app_conf.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
#define USBX_APP_MEM_POOL_SIZE       60 * 1024
#define USBX_MEMORY_STACK_SIZE       53 * 1024

/* USER CODE BEGIN EC */
#define USB_STATE_DISCONNECTED 0
#define USB_STATE_CONNECTED    1
#define USB_STATE_STREAMING    2
/* USER CODE END EC */

/* Exported functions prototypes ---------------------------------------------*/
UINT MX_USBX_Init(VOID);

/* USER CODE BEGIN 1 */
#if (CFG_LCD_SUPPORTED == 1)
/**
  * @brief Called in background to update the USB state on the LCD
  * @retval None
  */
void DrawStatusLCD(void);

/**
  * @brief Set current USB state to display it on the LCD
  * @param state: Current USB state
  * @retval None
  */
void Set_USB_State(uint8_t state);
#endif /* (CFG_LCD_SUPPORTED == 1) */

/* USER CODE END 1 */

#ifdef __cplusplus
}
#endif
#endif /* __APP_USBX_H__ */
