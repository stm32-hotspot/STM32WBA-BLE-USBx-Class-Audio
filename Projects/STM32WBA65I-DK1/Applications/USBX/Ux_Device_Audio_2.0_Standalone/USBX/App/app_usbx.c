/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_usbx.c
  * @author  MCD Application Team
  * @brief   USBX applicative file
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

/* Includes ------------------------------------------------------------------*/
#include "app_usbx.h"

/* USER CODE BEGIN UX_Memory_Buffer */
#include "stm32_lcd.h"
#include "stm32wba65i_discovery_bus.h"
#include "stm32wba65i_discovery_lcd.h"
#include "ssd1315.h"
#include "log_module.h"

/* USER CODE END UX_Memory_Buffer */
#if defined ( __ICCARM__ )
#pragma data_alignment=4
#endif
__ALIGN_BEGIN static UCHAR ux_byte_pool_buffer[USBX_APP_MEM_POOL_SIZE] __ALIGN_END;

#if (CFG_LCD_SUPPORTED == 1)
static void LCD_Init( void );
static int32_t LCD_DrawBitmapArray(uint8_t xpos, uint8_t ypos, uint8_t xlen, uint8_t ylen, uint8_t *data);

uint8_t gUsb_state = USB_STATE_DISCONNECTED;
uint8_t gUsb_displayed_state = USB_STATE_DISCONNECTED;
#endif /* (CFG_LCD_SUPPORTED == 1) */

/**
  * @brief  Application USBX Initialization.
  * @param  none
  * @retval status
  */
UINT MX_USBX_Init(VOID)
{
  UINT ret = UX_SUCCESS;
  UCHAR *pointer;

  /* USER CODE BEGIN MX_USBX_Init0 */
#if (CFG_LCD_SUPPORTED == 1)
  /* LCD Initialisation */
  LCD_Init();
#endif /* (CFG_LCD_SUPPORTED == 1) */
  /* USER CODE END MX_USBX_Init0 */

  pointer = ux_byte_pool_buffer;

  /* Initialize USBX Memory */
  if (ux_system_initialize(pointer, USBX_MEMORY_STACK_SIZE, UX_NULL, 0) != UX_SUCCESS)
  {
    /* USER CODE BEGIN USBX_SYSTEM_INITIALIZE_ERROR */
    return UX_ERROR;
    /* USER CODE END USBX_SYSTEM_INITIALIZE_ERROR */
  }

  if (MX_USBX_Device_Init() != UX_SUCCESS)
  {
  /* USER CODE BEGIN MX_USBX_Device_Init_Error */
    while(1)
    {
    }
  /* USER CODE END MX_USBX_Device_Init_Error */
  }

  /* USER CODE BEGIN MX_USBX_Init1 */

  /* USER CODE END MX_USBX_Init1 */

  return ret;
}
/**
  * @brief  _ux_utility_interrupt_disable
  *         USB utility interrupt disable.
  * @param  none
  * @retval interrupt save
  */
ALIGN_TYPE _ux_utility_interrupt_disable(VOID)
{
  UINT interrupt_save = 0;
  /* USER CODE BEGIN _ux_utility_interrupt_disable */
  interrupt_save = __get_PRIMASK();
  __disable_irq();
  /* USER CODE END _ux_utility_interrupt_disable */

  return interrupt_save;
}

/**
  * @brief  _ux_utility_interrupt_restore
  *         USB utility interrupt restore.
  * @param  flags
  * @retval none
  */
VOID _ux_utility_interrupt_restore(ALIGN_TYPE flags)
{

  /* USER CODE BEGIN _ux_utility_interrupt_restore */
  __set_PRIMASK(flags);
  /* USER CODE END _ux_utility_interrupt_restore */
}

/**
  * @brief  _ux_utility_time_get
  *         Get Time Tick for host timing.
  * @param  none
  * @retval time tick
  */
ULONG _ux_utility_time_get(VOID)
{
  ULONG time_tick = 0U;

  /* USER CODE BEGIN _ux_utility_time_get */

  /* USER CODE END _ux_utility_time_get */

  return time_tick;
}

/* USER CODE BEGIN 1 */
#if (CFG_LCD_SUPPORTED == 1)
/**
  * @brief Perform init of LCD bsp and display initial text and images
  * @retval None
  */
static void LCD_Init( void )
{
  extern uint8_t usb_logo[];
  extern uint8_t nok_icon[];
  int32_t   iStatus;

  BSP_SPI3_Init();

  iStatus = BSP_LCD_Init(0, LCD_ORIENTATION_LANDSCAPE );

  if ( iStatus == BSP_ERROR_NONE )
  {
    iStatus = BSP_LCD_DisplayOn(0);
  }

  if ( iStatus == BSP_ERROR_NONE )
  {
    /* Set LCD Foreground Layer  */
    UTIL_LCD_SetFuncDriver(&LCD_Driver); /* SetFunc before setting device */
    UTIL_LCD_SetDevice(0);               /* SetDevice after funcDriver is set */

    BSP_LCD_DisplayOn(0);

    BSP_LCD_Clear(0,SSD1315_COLOR_BLACK);
    BSP_LCD_Refresh(0);

    /* Set the LCD Text Color */
    UTIL_LCD_SetFont(&Font12);
    UTIL_LCD_SetTextColor(SSD1315_COLOR_WHITE);
    UTIL_LCD_SetBackColor(SSD1315_COLOR_BLACK);
    BSP_LCD_Refresh(0);

    LCD_DrawBitmapArray(35 , 32, 32, 32, &usb_logo[0]);
    LCD_DrawBitmapArray(75 , 40, 16, 16, &nok_icon[0]);

    DrawStatusLCD();

    UTIL_LCD_DisplayStringAt(0, 5, (uint8_t *)"USB Device", CENTER_MODE);
    UTIL_LCD_DisplayStringAt(0, 5+12+2, (uint8_t *)"Audio Class", CENTER_MODE);

    BSP_LCD_Refresh(0);
  }

  /* release bus for power optimisation */
  BSP_SPI3_DeInit();
}

/**
 * @brief Draw an array of bits at the specified offsets starting from corner top left. Ensure xlen is multiple of 8
 * @param xpos: X coordinate to print at
 * @param ypos: Y coordinate to print at
 * @param xlen: Width of the bitmap array
 * @param ylen: Height of the bitmap array
 * @param data: Pointer to the bitmap array
 * @retval 0 if success, -1 if the coordinates are out of screen
 */
static int32_t LCD_DrawBitmapArray(uint8_t xpos, uint8_t ypos, uint8_t xlen, uint8_t ylen, uint8_t *data)
{

  int32_t i,j,k;
  uint8_t mask;
  uint8_t* pdata = data;

  if (((xpos+xlen) > 128) || ((ypos+ylen) > 64))
  {
    /*out of screen*/
    return -1;
  }

  for (j=0 ; j < ylen ; j++)
  {
    for (i=0 ; i < xlen/8 ; i++)
    {
      mask = 0x80;
      for (k=0 ; k < 8 ; k++)
      {
        if ( mask & *pdata)
        {
          UTIL_LCD_SetPixel(i*8+k+xpos, j+ypos, SSD1315_COLOR_WHITE);
        }
        else
        {
          UTIL_LCD_SetPixel(i*8+k+xpos, j+ypos, SSD1315_COLOR_BLACK);
        }
        mask = mask >> 1;
      }
      pdata++;
    }
  }

  BSP_LCD_Refresh(0);

  return 0;
}

/**
  * @brief Called in background to update the USB state on the LCD
  * @retval None
  */
void DrawStatusLCD(void)
{
  uint8_t *usb_state_logo = 0;
  uint8_t state = gUsb_state;
  extern uint8_t ok_icon[];
  extern uint8_t nok_icon[];
  extern uint8_t speaker_icon[];

  switch (state)
  {
    case USB_STATE_DISCONNECTED:
      /* Draw NOK icon */
      usb_state_logo = &nok_icon[0];
      break;

    case USB_STATE_CONNECTED:
      /* Draw OK icon */
      usb_state_logo = &ok_icon[0];
      break;

    case USB_STATE_STREAMING:
      /* Draw Speaker icon */
      usb_state_logo = &speaker_icon[0];
      break;
  }

  /* Only update if state has changed */
  if ((usb_state_logo != 0) && (gUsb_displayed_state != state))
  {
    BSP_SPI3_Init();
    LCD_DrawBitmapArray(75 , 40, 16, 16, usb_state_logo);
    gUsb_displayed_state = state;
    BSP_SPI3_DeInit();
  }
}

/**
  * @brief Set current USB state to display it on the LCD
  * @param state: Current USB state
  * @retval None
  */
void Set_USB_State(uint8_t state)
{
  gUsb_state = state;
}
#endif  /* CFG_LCD_SUPPORTED */


/* USER CODE END 1 */
