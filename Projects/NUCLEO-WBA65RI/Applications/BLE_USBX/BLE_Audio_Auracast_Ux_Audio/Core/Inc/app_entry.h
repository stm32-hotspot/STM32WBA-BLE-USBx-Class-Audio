/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_entry.h
  * @author  MCD Application Team
  * @brief   Interface to the application
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#ifndef APP_ENTRY_H
#define APP_ENTRY_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

/* Private includes ----------------------------------------------------------*/
#include "app_common.h"
/* USER CODE BEGIN Includes */
#include "pbp_app.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef enum {
  AUDIO_DRIVER_CONFIG_HEADSET = 0x00,
  AUDIO_DRIVER_CONFIG_LINEIN = 0x01,
  AUDIO_DRIVER_CONFIG_SPEAKER = 0x02,
  AUDIO_DRIVER_CONFIG_NO_DRIVER = 0x03
} AudioDriverConfig;

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
#define WPAN_SUCCESS 0u

/* USER CODE BEGIN EC */
#define USB_STATE_DISCONNECTED    0
#define USB_STATE_CONNECTED       1
#define USB_STATE_STREAMING       2

#define AURACAST_STATE_OFF        0
#define AURACAST_STATE_CONFIGURED 1
#define AURACAST_STATE_STREAMING  2
/* USER CODE END EC */

/* Exported variables --------------------------------------------------------*/
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Exported macros ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void MX_APPE_Config(void);
uint32_t MX_APPE_Init(void *p_param);
void MX_APPE_Process(void);

/* USER CODE BEGIN EFP */
void PLL_Ready_ProcessIT(void);
void MX_AudioInit(Audio_Role_t role,
                  Sampling_Freq_t sampling_frequency,
                  Frame_Duration_t frame_duration,
                  uint8_t *pSnkBuff,
                  uint8_t *pSrcBuff,
                  AudioDriverConfig driver_config);
void MX_AudioDeInit(void);
int32_t Start_TxAudio(void);
void Stop_TxAudio(void);
int32_t Start_RxAudio(void);
void Stop_RxAudio(void);
void AudioClock_Init(uint32_t frequency);
void PLL_Exit(void);
void APP_NotifyFrameCplt(void);

#if (CFG_LED_SUPPORTED == 1)
/**
  * @brief Set current USB state to display it on the LCD and run LCD Draw task
  * @param state: Current USB state
  * @retval None
  */
void Set_USB_State(uint8_t state);

/**
  * @brief Set current Auracast state to display it on the LCD and run LCD Draw task
  * @param state: Current USB state
  * @retval None
  */
void Set_Auracast_State(uint8_t state);
#endif /* (CFG_LED_SUPPORTED == 1) */
/* USER CODE END EFP */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*APP_ENTRY_H */
