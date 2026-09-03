/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ux_device_audio_play.h
  * @author  MCD Application Team
  * @brief   USBX Device audio plyaback applicative header file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#ifndef __UX_DEVICE_AUDIO_PLAY_H__
#define __UX_DEVICE_AUDIO_PLAY_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "ux_api.h"
#include "ux_device_class_audio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "main.h"
#include "ux_device_class_audio20.h"
#include "ux_device_descriptors.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
VOID USBD_AUDIO_PlaybackStreamChange(UX_DEVICE_CLASS_AUDIO_STREAM *audio_play_stream,
                                     ULONG alternate_setting);
VOID USBD_AUDIO_PlaybackStreamFrameDone(UX_DEVICE_CLASS_AUDIO_STREAM *audio_play_stream,
                                        ULONG length);
ULONG USBD_AUDIO_PlaybackStreamGetMaxFrameBufferNumber(VOID);
ULONG USBD_AUDIO_PlaybackStreamGetMaxFrameBufferSize(VOID);
/* USER CODE BEGIN EFP */
/**
  * @brief Reset USB buffers and stop Audio Timer
  * @param none
  * @retval none
  */
void USBD_AUDIO_Stop(void);

/**
  * @brief Compute current USB feedback value and send it to USBX stack
  * @retval None
  */
void USBD_ComputeUSBFeedback(void);

/**
  * @brief Feedback task ran by USBX stack when necessary
  * @param stream: pointer to USBX Class Audio Stream
  * @retval status of the operation
  */
UINT USBD_AUDIO_Feedback_task_function(UX_DEVICE_CLASS_AUDIO_STREAM *stream);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define USB_AUDIO_BUF_SIZE   48*20*2*2 /* 2*10ms 48KHz stereo 16bit buffer */
#define SAI_AUDIO_BUF_SIZE   48*2*2*2 /* 2x1ms 48KHz stereo 16bit frame */

#define FEEDBACK_COMPUTE_DELAY_MAX 5

typedef enum
{
  AUDIOPLAYER_STOP = 0,
  AUDIOPLAYER_START,
  AUDIOPLAYER_PLAY,
  AUDIOPLAYER_PAUSE,
  AUDIOPLAYER_EOF,
  AUDIOPLAYER_ERROR,
}AUDIOPLAYER_StateTypedef;

typedef struct
{
  uint32_t        volume;
  uint32_t        mute;
  uint32_t        freq;
  AUDIOPLAYER_StateTypedef state;
}AUDIO_ProcessTypdef ;

/* Audio buffer control structure */
typedef struct {
  uint8_t buff[USB_AUDIO_BUF_SIZE];
  uint8_t rd_enable;
  uint32_t rd_ptr;
  uint32_t wr_ptr;
}AUDIO_OUT_BufferTypeDef;
/* USER CODE END PD */

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

#ifdef __cplusplus
}
#endif
#endif  /* __UX_DEVICE_AUDIO_PLAY_H__ */
