/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ux_device_audio_play.c
  * @author  MCD Application Team
  * @brief   USBX Device Video applicative source file
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

/* Includes ------------------------------------------------------------------*/
#include "ux_device_audio_play.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "log_module.h"
#include "app_usbx.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* Set BufferCtl start address */
#if defined ( __ICCARM__ ) /* IAR Compiler */
#pragma location = ".AudioStreamBufferSection"
#else
__attribute__((section(".AudioStreamBufferSection")))
#endif
/* Double BUFFER for Output Audio stream */
__ALIGN_BEGIN AUDIO_OUT_BufferTypeDef  BufferCtl __ALIGN_END;
__ALIGN_BEGIN uint8_t DMABuffer[SAI_AUDIO_BUF_SIZE] __ALIGN_END;
uint8_t FeedbackComputeDelay = 0;
UX_DEVICE_CLASS_AUDIO_STREAM *pAudioPlayStream;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static uint32_t GetBufferLevel(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  USBD_AUDIO_PlaybackStreamChange
  *         This function is invoked to inform application that the
  *         alternate setting are changed.
  * @param  audio_play_stream: Pointer to audio playback class stream instance.
  * @param  alternate_setting: interface alternate setting.
  * @retval none
  */
VOID USBD_AUDIO_PlaybackStreamChange(UX_DEVICE_CLASS_AUDIO_STREAM *audio_play_stream,
                                     ULONG alternate_setting)
{
  /* USER CODE BEGIN USBD_AUDIO_PlaybackStreamChange */
  UINT ret;
  UNUSED(ret);

  LOG_INFO_APP("USBD_AUDIO_PlaybackStreamChange - alternate_setting=%d\n", alternate_setting);

  if (alternate_setting == 0)
  {
    /* Stop Audio (stream stopped) */
    USBD_AUDIO_Stop();
#if (CFG_LCD_SUPPORTED == 1)
    Set_USB_State(USB_STATE_CONNECTED);
#endif /* (CFG_LCD_SUPPORTED == 1) */
  }
  else
  {
    /* Start reception (stream opened)  */
    ret = ux_device_class_audio_reception_start(audio_play_stream);
    LOG_INFO_APP("ux_device_class_audio_reception_start with status %d\n", ret);
  }
  /* USER CODE END USBD_AUDIO_PlaybackStreamChange */

  return;
}

/**
  * @brief  USBD_AUDIO_PlaybackStreamFrameDone
  *         This function is invoked whenever a USB packet (audio frame) is received
  *         from the host.
  * @param  audio_play_stream: Pointer to audio playback class stream instance.
  * @param  length: transfer length.
  * @retval none
  */
VOID USBD_AUDIO_PlaybackStreamFrameDone(UX_DEVICE_CLASS_AUDIO_STREAM *audio_play_stream,
                                        ULONG length)
{
  /* USER CODE BEGIN USBD_AUDIO_PlaybackStreamFrameDone */

  UCHAR         *frame_buffer;
  ULONG         frame_length;
  int32_t       ret;

  UNUSED(ret);

  /* Get access to first audio input frame.  */
  ux_device_class_audio_read_frame_get(audio_play_stream, &frame_buffer, &frame_length);

  if (length)
  {
    if (BufferCtl.wr_ptr > USB_AUDIO_BUF_SIZE - frame_length)
    {
      /* Not enough space in buffer: copy remaining space available and rollback */
      uint32_t partial_copy_len = USB_AUDIO_BUF_SIZE - BufferCtl.wr_ptr;

      ux_utility_memory_copy(&BufferCtl.buff[BufferCtl.wr_ptr], frame_buffer, partial_copy_len);
      ux_utility_memory_copy(&BufferCtl.buff[0], frame_buffer + partial_copy_len, frame_length - partial_copy_len);
      BufferCtl.wr_ptr = frame_length - partial_copy_len;
    }
    else
    {
      /* Enough space in buffer: copy data */
      ux_utility_memory_copy(&BufferCtl.buff[BufferCtl.wr_ptr], frame_buffer, frame_length);
      BufferCtl.wr_ptr += frame_length;
    }

    /* Check trigger for Audio DMA start (half of buffer full) */
    if ((BufferCtl.rd_enable == 0) && ((BufferCtl.wr_ptr - BufferCtl.rd_ptr) > USB_AUDIO_BUF_SIZE / 2))
    {
      /* Start Audio DMA */
      BufferCtl.rd_enable = 1;

      memcpy(&DMABuffer[0], &BufferCtl.buff[0], SAI_AUDIO_BUF_SIZE);
      BufferCtl.rd_ptr = SAI_AUDIO_BUF_SIZE;

      /*DMA stream from output double buffer to codec in Circular mode launch*/
      ret = BSP_AUDIO_OUT_Play(0, &DMABuffer[0], SAI_AUDIO_BUF_SIZE);
      LOG_INFO_APP("BSP_AUDIO_OUT_Play with status %d\n", ret);

#if (CFG_LCD_SUPPORTED == 1)
      Set_USB_State(USB_STATE_STREAMING);
#endif /* (CFG_LCD_SUPPORTED == 1) */
    }
  }

  /* Save audio stream pointer */
  pAudioPlayStream = audio_play_stream;

  /* Re-free the first audio input frame for transfer.  */
  ux_device_class_audio_read_frame_free(audio_play_stream);

  /* Compute USB Drift */
  USBD_ComputeUSBFeedback();

  /* USER CODE END USBD_AUDIO_PlaybackStreamFrameDone */

  return;
}

/**
  * @brief  USBD_AUDIO_PlaybackStreamGetMaxFrameBufferNumber
  *         This function is invoked to Set audio playback stream max Frame buffer number.
  * @param  none
  * @retval max frame buffer number
  */
ULONG USBD_AUDIO_PlaybackStreamGetMaxFrameBufferNumber(VOID)
{
  ULONG max_frame_buffer_number = 0U;

  /* USER CODE BEGIN USBD_AUDIO_PlaybackStreamGetMaxFrameBufferNumber */
  max_frame_buffer_number = 3U;

  /* USER CODE END USBD_AUDIO_PlaybackStreamGetMaxFrameBufferNumber */

  return max_frame_buffer_number;
}

/**
  * @brief  USBD_AUDIO_PlaybackStreamGetMaxFrameBufferSize
  *         This function is invoked to Set audio playback stream max Frame buffer size.
  * @param  none
  * @retval max frame buffer size
  */
ULONG USBD_AUDIO_PlaybackStreamGetMaxFrameBufferSize(VOID)
{
  ULONG max_frame_buffer_size = 0U;

  /* USER CODE BEGIN USBD_AUDIO_PlaybackStreamGetMaxFrameBufferSize */

  max_frame_buffer_size = USBD_AUDIO_PLAY_EPOUT_HS_MPS;

  /* USER CODE END USBD_AUDIO_PlaybackStreamGetMaxFrameBufferSize */

  return max_frame_buffer_size;
}

/* USER CODE BEGIN 1 */

/**
  * @brief Reset USB buffers and stop Audio DMA
  * @param none
  * @retval none
  */
void USBD_AUDIO_Stop(void)
{
  /* Reset buffer */
  memset(&BufferCtl.buff[0], 0, USB_AUDIO_BUF_SIZE);

  if (BufferCtl.rd_enable == 1)
  {
    uint8_t ret;
    UNUSED(ret);
    /* Stop DMA */
    ret = BSP_AUDIO_OUT_Stop(0);
    LOG_INFO_APP("BSP_AUDIO_OUT_Stop with status %d\n", ret);
  }

  /* Reset buffer */
  BufferCtl.wr_ptr = 0;
  BufferCtl.rd_ptr = 0;
  BufferCtl.rd_enable = 0;
}

/**
  * @brief Compute current USB feedback value and send it to USBX stack
  * @retval None
  */
void USBD_ComputeUSBFeedback(void)
{
  /* Do not execute USB feedback computation at each frame */
  FeedbackComputeDelay ++;

  if (FeedbackComputeDelay == FEEDBACK_COMPUTE_DELAY_MAX)
  {
    FeedbackComputeDelay = 0;

    /* Check offset of sample */
    int32_t sampleOffset = ((int32_t) GetBufferLevel()) - ((int32_t) USB_AUDIO_BUF_SIZE / 2);
    uint32_t frameDiv = (ux_device_class_audio_speed_get(pAudioPlayStream) == UX_HIGH_SPEED_DEVICE) ? 8000U : 1000U; /* 1000 for FULL_SPEED, 8000 for HIGH_SPEED */
    sampleOffset /= 4; /* Divide per sample size to get number of samples */

    int32_t curFrequency = 48000 - sampleOffset; /* Process actual frequency */
    uint32_t feedback = (uint32_t)(((float)curFrequency / (float)frameDiv) * 65536.0f); /* Process feedback according to USB spec */

    /* Send feedback to USBX stack */
    ux_device_class_audio_feedback_set(pAudioPlayStream, (UCHAR*) &feedback);
  }
}

/**
  * @brief Callback coming from Audio BSP to notify DMA has reached the half of the buffer
  * @param instance: Audio BSP instance ID
  * @retval None
  */
void BSP_AUDIO_OUT_HalfTransfer_CallBack(uint32_t instance)
{
  /* Copy USB data to completed region of the buffer */
  memcpy(&DMABuffer[0], &BufferCtl.buff[BufferCtl.rd_ptr], SAI_AUDIO_BUF_SIZE / 2);

  if (BufferCtl.rd_ptr + SAI_AUDIO_BUF_SIZE / 2 < USB_AUDIO_BUF_SIZE)
  {
    /* Increment read pointer */
    BufferCtl.rd_ptr += SAI_AUDIO_BUF_SIZE / 2;
  }
  else
  {
    /* Roll back read pointer */
    BufferCtl.rd_ptr = 0;
  }
}


/**
  * @brief Callback coming from Audio BSP to notify DMA has reached the end of the buffer
  * @param instance: Audio BSP instance ID
  * @retval None
  */
void BSP_AUDIO_OUT_TransferComplete_CallBack(uint32_t instance)
{
  /* Copy USB data to completed region of the buffer */
  memcpy(&DMABuffer[SAI_AUDIO_BUF_SIZE / 2], &BufferCtl.buff[BufferCtl.rd_ptr], SAI_AUDIO_BUF_SIZE / 2);

  if (BufferCtl.rd_ptr + SAI_AUDIO_BUF_SIZE / 2 < USB_AUDIO_BUF_SIZE)
  {
    /* Increment read pointer */
    BufferCtl.rd_ptr += SAI_AUDIO_BUF_SIZE / 2;
  }
  else
  {
    /* Roll back read pointer */
    BufferCtl.rd_ptr = 0;
  }
}

/**
  * @brief Feedback task ran by USBX stack when necessary
  * @param stream: pointer to USBX Class Audio Stream
  * @retval status of the operation
  */
UINT USBD_AUDIO_Feedback_task_function(UX_DEVICE_CLASS_AUDIO_STREAM *stream)
{
  if (BufferCtl.rd_enable == 1)
  {
    /* Run task only when read is enabled to avoid stuck USB packets */
    return _ux_device_class_audio_feedback_task_function(stream);
  }
  else
  {
    return UX_STATE_WAIT;
  }
}

/**
  * @brief Returns current level of the buffer
  * @param none
  * @retval buffer level
  */
static uint32_t GetBufferLevel(void)
{
  if (BufferCtl.wr_ptr > BufferCtl.rd_ptr)
  {
    return BufferCtl.wr_ptr - BufferCtl.rd_ptr;
  }
  else
  {
    return USB_AUDIO_BUF_SIZE - BufferCtl.rd_ptr + BufferCtl.wr_ptr;
  }
}
/* USER CODE END 1 */
