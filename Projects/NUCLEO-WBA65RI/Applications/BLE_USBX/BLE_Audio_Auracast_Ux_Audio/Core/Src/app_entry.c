/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_entry.c
  * @author  MCD Application Team
  * @brief   Entry point of the application
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
#include "app_common.h"
#include "log_module.h"
#include "app_conf.h"
#include "main.h"
#include "app_entry.h"
#include "stm32_rtos.h"
#if (CFG_LPM_LEVEL != 0)
#include "stm32_lpm.h"
#endif /* (CFG_LPM_LEVEL != 0) */
#include "stm32_timer.h"
#include "advanced_memory_manager.h"
#include "stm32_mm.h"
#if (CFG_LOG_SUPPORTED != 0)
#include "stm32_adv_trace.h"
#include "serial_cmd_interpreter.h"
#endif /* CFG_LOG_SUPPORTED */
#include "app_ble.h"
#include "ll_sys.h"
#include "ll_sys_if.h"
#include "app_sys.h"
#include "otp.h"
#include "scm.h"
#include "pka_ctrl.h"
#include "flash_driver.h"
#include "flash_manager.h"
#include "simple_nvm_arbiter.h"
#include "app_debug.h"
#if(CFG_RT_DEBUG_DTB == 1)
#include "RTDebug_dtb.h"
#endif /* CFG_RT_DEBUG_DTB */
#include "stm32_lpm_if.h"

/* Private includes -----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "app_bsp.h"
#include "codec_mngr.h"
#include "codec_if.h"
#include "stm32_lpm.h"
#include "pbp_app.h"
#if(CFG_RT_DEBUG_DTB == 1)
#include "RTDebug_dtb.h"
#endif /* CFG_RT_DEBUG_DTB */
#include "ux_device_audio_play.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/

/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private defines -----------------------------------------------------------*/
#define AMM_POOL_SIZE ( DIVC(CFG_MM_POOL_SIZE, sizeof (uint32_t)) +\
                      (AMM_VIRTUAL_INFO_ELEMENT_SIZE * CFG_AMM_VIRTUAL_MEMORY_NUMBER) )
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macros ------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private constants ---------------------------------------------------------*/
/* USER CODE BEGIN PC */

/* USER CODE END PC */

/* Private variables ---------------------------------------------------------*/
#if ( CFG_LPM_LEVEL != 0)
static bool system_startup_done = FALSE;
#endif /* ( CFG_LPM_LEVEL != 0) */

#if (CFG_LOG_SUPPORTED != 0)
/* Log configuration
 * .verbose_level can be any value of the Log_Verbose_Level_t enum.
 * .region_mask can either be :
 * - LOG_REGION_ALL_REGIONS to enable all regions
 * or
 * - One or several specific regions (any value except LOG_REGION_ALL_REGIONS)
 *   from the Log_Region_t enum and matching the mask value.
 *
 *   For example, to enable both LOG_REGION_BLE and LOG_REGION_APP,
 *   the value assigned to the define is :
 *   (1U << LOG_REGION_BLE | 1U << LOG_REGION_APP)
 */
static Log_Module_t Log_Module_Config = { .verbose_level = APPLI_CONFIG_LOG_LEVEL, .region_mask = APPLI_CONFIG_LOG_REGION };
#endif /* (CFG_LOG_SUPPORTED != 0) */

/* AMM configuration */
static uint32_t AMM_Pool[AMM_POOL_SIZE];
static AMM_VirtualMemoryConfig_t vmConfig[CFG_AMM_VIRTUAL_MEMORY_NUMBER] =
{
  /* Virtual Memory #1 */
  {
    .Id = CFG_AMM_VIRTUAL_BLE_TIMERS,
    .BufferSize = CFG_AMM_VIRTUAL_BLE_TIMERS_BUFFER_SIZE
  },
  /* Virtual Memory #2 */
  {
    .Id = CFG_AMM_VIRTUAL_BLE_EVENTS,
    .BufferSize = CFG_AMM_VIRTUAL_BLE_EVENTS_BUFFER_SIZE
  },
};

static AMM_InitParameters_t ammInitConfig =
{
  .p_PoolAddr = AMM_Pool,
  .PoolSize = AMM_POOL_SIZE,
  .VirtualMemoryNumber = CFG_AMM_VIRTUAL_MEMORY_NUMBER,
  .p_VirtualMemoryConfigList = vmConfig
};

static uint8_t PkaMut = 0u;
static uint8_t EndOfProcSem = 0u;

static uint8_t FlashMut = 0u;

/* USER CODE BEGIN PV */
#if (CFG_LED_SUPPORTED == 1)
static uint8_t gAuracast_state = AURACAST_STATE_OFF;
static uint8_t gUsb_state = USB_STATE_DISCONNECTED;
#endif /* (CFG_LED_SUPPORTED == 1) */
static uint32_t PLL_Target_Clock_Freq = 0;
/* USER CODE END PV */

/* Global variables ----------------------------------------------------------*/
/* USER CODE BEGIN GV */
uint32_t gDefault_Exec_Time = 0; /* effective default exec time */

/* USER CODE END GV */

/* Private functions prototypes-----------------------------------------------*/
static void System_Init( void );
static void SystemPower_Config( void );
static void Config_HSE(void);
static void APPE_RNG_Init( void );

static void APPE_AMM_Init(void);
static void AMM_WrapperInit(uint32_t * const p_PoolAddr, const uint32_t PoolSize);
static uint32_t * AMM_WrapperAllocate(const uint32_t BufferSize);
static void AMM_WrapperFree(uint32_t * const p_BufferAddr);

static void APPE_FLASH_MANAGER_Init( void );
FM_Cmd_Status_t FM_MutexTake(void);
FM_Cmd_Status_t FM_MutexRelease(void);

static void APPE_PKACTRL_Init( void );
int PKACTRL_MutexTake(void);
int PKACTRL_MutexRelease(void);
int PKACTRL_TakeSemEndOfOperation(void);
int PKACTRL_ReleaseSemEndOfOperation(void);

/* USER CODE BEGIN PFP */
#if (CFG_LED_SUPPORTED == 1)
static void LED_Init(void);
static void UpdateLEDStatusTask(void);
static void UpdateLEDStatus(void);
#endif /* CFG_LED_SUPPORTED */
static void PLL_Ready_Task(void);
/* USER CODE END PFP */

/* External variables --------------------------------------------------------*/

/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Functions Definition ------------------------------------------------------*/
/**
 * @brief   Wireless Private Area Network configuration.
 */
void MX_APPE_Config(void)
{
  /* Configure HSE Tuning */
  Config_HSE();
}

/**
 * @brief   Wireless Private Area Network initialisation.
 */
uint32_t MX_APPE_Init(void *p_param)
{
  APP_DEBUG_SIGNAL_SET(APP_APPE_INIT);

  UNUSED(p_param);

  /* System initialization */
  System_Init();

  /* Configure the system Power Mode */
  SystemPower_Config();

  /* Initialize the Advance Memory Manager module */
  APPE_AMM_Init();

  /* Initialize the Random Number Generator module */
  APPE_RNG_Init();

  /* Initialize the Flash Manager module */
  APPE_FLASH_MANAGER_Init();

  /* USER CODE BEGIN APPE_Init_1 */
  APP_BSP_Init();

#if (CFG_LED_SUPPORTED == 1)
  LED_Init();
  UTIL_SEQ_RegTask(1U << CFG_TASK_DRAW_STATE_LCD_ID, UTIL_SEQ_RFU, UpdateLEDStatusTask);
#endif /* CFG_LED_SUPPORTED */

  UTIL_SEQ_RegTask(1U << CFG_TASK_PLL_READY_ID, UTIL_SEQ_RFU, PLL_Ready_Task);

  /* USER CODE END APPE_Init_1 */

  /* Initialize the Public Key Accelerator module */
  APPE_PKACTRL_Init();

  /* Initialize the Simple Non Volatile Memory Arbiter */
  if( SNVMA_Init((uint32_t *)(FLASH_BASE + (FLASH_PAGE_SIZE * (CFG_SNVMA_START_SECTOR_ID)))) != SNVMA_ERROR_OK )
  {
    Error_Handler();
  }

  APP_BLE_Init();

  /* Disable RFTS Bypass for flash operation - Since LL has not started yet */
  FD_SetStatus (FD_FLASHACCESS_RFTS_BYPASS, LL_FLASH_DISABLE);

  /* USER CODE BEGIN APPE_Init_2 */

  /* USER CODE END APPE_Init_2 */

  APP_DEBUG_SIGNAL_RESET(APP_APPE_INIT);

  return WPAN_SUCCESS;
}

void MX_APPE_Process(void)
{
  /* USER CODE BEGIN MX_APPE_Process_1 */

  /* USER CODE END MX_APPE_Process_1 */
  UTIL_SEQ_Run(UTIL_SEQ_DEFAULT);
  /* USER CODE BEGIN MX_APPE_Process_2 */

  /* USER CODE END MX_APPE_Process_2 */
}

/* USER CODE BEGIN FD */

/* USER CODE END FD */

/*************************************************************
 *
 * LOCAL FUNCTIONS
 *
 *************************************************************/

/**
 * @brief Configure HSE by read this Tuning from OTP
 *
 */
static void Config_HSE(void)
{
  OTP_Data_s* otp_ptr = NULL;

  /* Read HSE_Tuning from OTP */
  if (OTP_Read(DEFAULT_OTP_IDX, &otp_ptr) != HAL_OK)
  {
    /* OTP no present in flash, apply default gain */
    HAL_RCCEx_HSESetTrimming(0x0C);
  }
  else
  {
    HAL_RCCEx_HSESetTrimming(otp_ptr->hsetune);
  }
}

/**
 *
 */
static void System_Init( void )
{
  /* Clear RCC RESET flag */
  LL_RCC_ClearResetFlags();

  /* Initialize the Timer Server */
  UTIL_TIMER_Init();

  /* USER CODE BEGIN System_Init_1 */

  /* USER CODE END System_Init_1 */

  /* Enable wakeup out of standby from RTC ( UTIL_TIMER )*/
  HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN7_HIGH_3);

#if (CFG_LOG_SUPPORTED != 0)
  /* Initialize the UART used for logs */
  MX_USART1_UART_Init();
  /* Initialize the logs ( using the USART ) */
  Log_Module_Init( Log_Module_Config );

  /* Initialize the Command Interpreter */
  Serial_CMD_Interpreter_Init();
#endif  /* (CFG_LOG_SUPPORTED != 0) */

#if(CFG_RT_DEBUG_DTB == 1)
  /* DTB initialization and configuration */
  RT_DEBUG_DTBInit();
  RT_DEBUG_DTBConfig();
#endif /* CFG_RT_DEBUG_DTB */
#if(CFG_RT_DEBUG_GPIO_MODULE == 1)
  /* RT DEBUG GPIO_Init */
  RT_DEBUG_GPIO_Init();
#endif /* (CFG_RT_DEBUG_GPIO_MODULE == 1) */

#if ( CFG_LPM_LEVEL != 0)
  system_startup_done = TRUE;
  UNUSED(system_startup_done);
#endif /* ( CFG_LPM_LEVEL != 0) */

  return;
}

/**
 * @brief  Configure the system for power optimization
 *
 * @note  This API configures the system to be ready for low power mode
 *
 * @param  None
 * @retval None
 */
static void SystemPower_Config(void)
{
#if (CFG_SCM_SUPPORTED == 1)
  /* Initialize System Clock Manager */
  scm_init();
#else
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }
#endif /* CFG_SCM_SUPPORTED */

#if (CFG_DEBUGGER_LEVEL == 0)
  /* Setup GPIOA 13, 14, 15 in Analog no pull */
  if(__HAL_RCC_GPIOA_IS_CLK_ENABLED() == 0)
  {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIOA->PUPDR &= ~0xFC000000;
    GPIOA->MODER |= 0xFC000000;
    __HAL_RCC_GPIOA_CLK_DISABLE();
  }
  else
  {
    GPIOA->PUPDR &= ~0xFC000000;
    GPIOA->MODER |= 0xFC000000;
  }

  /* Setup GPIOB 3, 4 in Analog no pull */
  if(__HAL_RCC_GPIOB_IS_CLK_ENABLED() == 0)
  {
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIOB->PUPDR &= ~0x3C0;
    GPIOB->MODER |= 0x3C0;
    __HAL_RCC_GPIOB_CLK_DISABLE();
  }
  else
  {
    GPIOB->PUPDR &= ~0x3C0;
    GPIOB->MODER |= 0x3C0;
  }
#endif /* CFG_DEBUGGER_LEVEL */

#if (CFG_DEBUGGER_LEVEL > 1)
  LL_DBGMCU_EnableDBGStandbyMode();
  LL_DBGMCU_EnableDBGStopMode();
#endif /* CFG_DEBUGGER_LEVEL */

#if (CFG_LPM_LEVEL != 0)
  /* Initialize low Power Manager. By default enabled */
  UTIL_LPM_Init();

#if ((CFG_LPM_STANDBY_SUPPORTED == 1) || (CFG_LPM_STOP2_SUPPORTED == 1))
  /* Enable SRAM1, SRAM2 and RADIO retention*/
  LL_PWR_SetSRAM1SBRetention(LL_PWR_SRAM1_SB_FULL_RETENTION);
  LL_PWR_SetSRAM2SBRetention(LL_PWR_SRAM2_SB_FULL_RETENTION);
  LL_PWR_SetRadioSBRetention(LL_PWR_RADIO_SB_FULL_RETENTION); /* Retain sleep timer configuration */

#else /* (CFG_LPM_STANDBY_SUPPORTED == 1) || (CFG_LPM_STOP2_SUPPORTED == 1) */
  UTIL_LPM_SetMaxMode(1U << CFG_LPM_APP, UTIL_LPM_MAX_MODE);
#endif /* (CFG_LPM_STANDBY_SUPPORTED == 1) || (CFG_LPM_STOP2_SUPPORTED == 1) */
#endif /* (CFG_LPM_LEVEL != 0)  */

  /* USER CODE BEGIN SystemPower_Config */
  /* USER CODE END SystemPower_Config */
}

/**
 * @brief Initialize Random Number Generator module
 */
static void APPE_RNG_Init(void)
{
  HW_RNG_SetPoolThreshold(CFG_HW_RNG_POOL_THRESHOLD);
  HW_RNG_Start();
  /* Register Random Number Generator task */
  UTIL_SEQ_RegTask(1U << CFG_TASK_HW_RNG, UTIL_SEQ_RFU, (void (*)(void))HW_RNG_Process);
}

/**
 * @brief Initialize Flash Manager module
 */
static void APPE_FLASH_MANAGER_Init(void)
{
  /* Init the Flash Manager module */
  FM_Init();

  /* Register Flash Manager task */
  UTIL_SEQ_RegTask(1U << CFG_TASK_FLASH_MANAGER, UTIL_SEQ_RFU, FM_BackgroundProcess);

  /* Disable flash before any use - RFTS */
  FD_SetStatus (FD_FLASHACCESS_RFTS, LL_FLASH_DISABLE);
  /* Enable RFTS Bypass for flash operation - Since LL has not started yet */
  FD_SetStatus (FD_FLASHACCESS_RFTS_BYPASS, LL_FLASH_ENABLE);
  /* Enable flash system flag */
  FD_SetStatus (FD_FLASHACCESS_SYSTEM, LL_FLASH_ENABLE);

  return;
}

/**
 * @brief Initialize Public Key Accelerator module
 */
static void APPE_PKACTRL_Init(void)
{
  /* Register Public Key Accelerator task */
  UTIL_SEQ_RegTask(1U << CFG_TASK_PKACTRL, UTIL_SEQ_RFU, PKACTRL_BG_Process);
}

static void APPE_AMM_Init(void)
{
  /* Initialize the Advance Memory Manager */
  if( AMM_Init(&ammInitConfig) != AMM_ERROR_OK )
  {
    Error_Handler();
  }

  /* Register Advance Memory Manager task */
  UTIL_SEQ_RegTask(1U << CFG_TASK_AMM, UTIL_SEQ_RFU, AMM_BackgroundProcess);
}

/* USER CODE BEGIN FD_LOCAL_FUNCTIONS */

#if (CFG_LED_SUPPORTED == 1)
static void LED_Init( void )
{
}

static void UpdateLEDStatusTask(void)
{
  UpdateLEDStatus();
}

uint8_t countLed = 0;

static void UpdateLEDStatus(void)
{
  switch (gAuracast_state)
  {
    case AURACAST_STATE_OFF:
      BSP_LED_Off(LED_BLUE);
      break;

    case AURACAST_STATE_CONFIGURED:
      BSP_LED_On(LED_BLUE);
      break;

    case AURACAST_STATE_STREAMING:
      if (countLed == 0)
      {
        BSP_LED_Toggle(LED_BLUE);
      }
      break;
  }

  switch (gUsb_state)
  {
    case USB_STATE_DISCONNECTED:
      BSP_LED_Off(LED_GREEN);
      break;

    case USB_STATE_CONNECTED:
      BSP_LED_On(LED_GREEN);
      break;

    case USB_STATE_STREAMING:
      if (countLed == 0)
      {
        BSP_LED_Toggle(LED_GREEN);
      }
      break;
  }

  if (countLed == 0)
  {
    countLed = 25;

  }
  else
  {
    countLed--;
  }
}
#endif  /* CFG_LED_SUPPORTED */

/**
  * @brief Configure PLL for the audio frequency, PLL output is used for the SAI peripheral and as the core clock
  * @retval None
  */
void AudioClock_Init(uint32_t audio_frequency_type)
{
  uint32_t target_freq;
  if (audio_frequency_type == SAMPLE_FREQ_44100_HZ)
  {
    target_freq = 90316800;
  }
  else
  {
    /* valid for 8k, 16k, 24k, 32k or 48k */
    target_freq = 98304000;
  }

  if ( target_freq !=  PLL_Target_Clock_Freq )
  {
    scm_pll_config_t pll_config;
    LOG_INFO_APP("Audio Clock Initialization\n");

    pll_config.pll_mode = PLL_FRACTIONAL_MODE;
    pll_config.PLLM = 6;
    pll_config.PLLP = 4;
    pll_config.PLLQ = 4;
    pll_config.PLLR = 4;

    /* VCO frequency should be in range 128 to 544 MHz */
    /* PLL fractionnal should allows to cover +/-550ppm around the targeted frequency*/
    if (target_freq == 90316800)
    {
      pll_config.PLLN = 67;  /* VCO = HSE/M * N * (1 + Nfrac / 8192) < 32 / 6 * (67 + 1) = 362.7 MHz */
      pll_config.PLLFractional = 6042;
      pll_config.AHB5_PLL1_CLKDivider = LL_RCC_AHB5_DIV_3;
      /* PLLSYS = 90.31673 MHz */
    }
    else
    {
      pll_config.PLLN = 73;  /* VCO = HSE/M * N * (1 + Nfrac / 8192) < 32 / 6 * (73 + 1) = 394.7 MHz */
      pll_config.PLLFractional = 5964;
      pll_config.AHB5_PLL1_CLKDivider = LL_RCC_AHB5_DIV_4;
      /* PLLSYS = 98.30396 MHz */
    }

    UTIL_LPM_SetMaxMode(1 << CFG_LPM_AUDIO, UTIL_LPM_SLEEP_MODE);

    scm_pll_setconfig(&pll_config);

    scm_setsystemclock(SCM_USER_APP, SYS_PLL);

    /* enable P output used for the audio timer */
    LL_RCC_PLL1_EnableDomain_PLL1P();

    PLL_Target_Clock_Freq = target_freq;
  }
  else
  {
    LOG_INFO_APP("Audio Clock already configured\n");
  }
}

void PLL_Ready_ProcessIT(void)
{
  UTIL_SEQ_SetTask(1U << CFG_TASK_PLL_READY_ID, CFG_SEQ_PRIO_0);
}

void PLL_Ready_Task(void)
{
  /* set Link Layer audio timings */
  uint32_t iso_exec_time;
  Evnt_timing_t event_time;
  event_time.drift_time    = ISO_PLL_DRIFT_TIME;
  event_time.exec_time     = ISO_PLL_EXEC_TIME;
  event_time.schdling_time = ISO_PLL_SCHDL_TIME;

#if defined(__GNUC__) && defined(DEBUG)
  event_time.drift_time += ISO_PLL_DRIFT_TIME_EXTRA_GCC_DEBUG;
  event_time.exec_time += ISO_PLL_EXEC_TIME_EXTRA_GCC_DEBUG;
#endif

  ll_intf_config_schdling_time(&event_time, &iso_exec_time);

  CODEC_CLK_Init();

  AUDIO_PLLConfig_t pll_config;
  pll_config.PLLTargetFreq = PLL_Target_Clock_Freq;
  pll_config.VCOInputFreq = (32000000.0f / 6.0f); /* HSE / PLL_M */
  pll_config.PLLOutputDiv = 4; /* PLL_P */

  AUDIO_CorrectorConfig_t corrector_config;
  corrector_config.initialMinSampling = 500;
  corrector_config.minSampling = 4000;
  corrector_config.mode = 1; /* this mode is in early release in V1.10 codec manager lib and allows to compensate long term drift using the hardware timestamping */
  AUDIO_InitializeClockCorrector(&pll_config, &corrector_config);
}

void PLL_Exit(void)
{
  if (PLL_Target_Clock_Freq != 0)
  {
    scm_setsystemclock(SCM_USER_APP, NO_CLOCK_CONFIG);

    PLL_Target_Clock_Freq = 0;

    UTIL_LPM_SetMaxMode(1 << CFG_LPM_AUDIO, UTIL_LPM_MAX_MODE);
  }
}

int32_t Start_TxAudio(void)
{
  LOG_INFO_APP("START AUDIO SINK (output)\n");
  return 0;
}

void Stop_TxAudio(void)
{
  /* Nothing to do */
}

extern AUDIO_BufferTypeDef  BufferCtl;
int32_t Start_RxAudio(void)
{
   /* Start TIM17 timer to start sending packets to codec */
  /* Clear the update flag */
  LL_TIM_ClearFlag_UPDATE(TIM17);

  /* Enable the update interrupt */
  LL_TIM_EnableIT_UPDATE(TIM17);

  /* Enable counter */
  LL_TIM_EnableCounter(TIM17);

  LOG_INFO_APP("START AUDIO SOURCE (input)\n");

#if (CFG_LED_SUPPORTED == 1)
  Set_Auracast_State(AURACAST_STATE_CONFIGURED);
#endif /* (CFG_LED_SUPPORTED == 1) */
  return 0;
}

void Stop_RxAudio(void)
{
  /* Nothing to do */
}


#if (CFG_LED_SUPPORTED == 1)
/**
  * @brief Set current USB state to display it on the LCD and run LCD Draw task
  * @param state: Current USB state
  * @retval None
  */
void Set_USB_State(uint8_t state)
{
  if (state != gUsb_state)
  {
    gUsb_state = state;
    UTIL_SEQ_SetTask( 1U << CFG_TASK_DRAW_STATE_LCD_ID, CFG_SEQ_PRIO_0);
  }
}

/**
  * @brief Set current Auracast state to display it on the LCD and run LCD Draw task
  * @param state: Current USB state
  * @retval None
  */
void Set_Auracast_State(uint8_t state)
{
  gAuracast_state = state;
  UTIL_SEQ_SetTask( 1U << CFG_TASK_DRAW_STATE_LCD_ID, CFG_SEQ_PRIO_0);

}
#endif /* (CFG_LED_SUPPORTED == 1) */
/* USER CODE END FD_LOCAL_FUNCTIONS */

/*************************************************************
 *
 * WRAP FUNCTIONS
 *
 *************************************************************/

void UTIL_SEQ_Idle( void )
{
#if ( CFG_LPM_LEVEL != 0)
  HAL_SuspendTick();
#if (CFG_SCM_SUPPORTED == 1)
  /* SCM HSE BEGIN */
  SCM_HSE_StopStabilizationTimer();
  /* SCM HSE END */
#endif /* CFG_SCM_SUPPORTED */
  UTIL_LPM_Enter(0);
  HAL_ResumeTick();
#endif /* CFG_LPM_LEVEL */
  return;
}

void UTIL_SEQ_PreIdle( void )
{
  /* USER CODE BEGIN UTIL_SEQ_PreIdle_1 */

  /* USER CODE END UTIL_SEQ_PreIdle_1 */
#if ( CFG_LPM_LEVEL != 0)
  LL_PWR_ClearFlag_STOP();

#if ((CFG_LPM_STANDBY_SUPPORTED == 1) || (CFG_LPM_STOP2_SUPPORTED == 1))
#if (CFG_LPM_STOP2_SUPPORTED == 1)
  if ( ( system_startup_done != FALSE ) && ( UTIL_LPM_GetMaxMode() >= UTIL_LPM_STOP2_MODE ) )
#else /* (CFG_LPM_STOP2_SUPPORTED == 1) */
#if (CFG_LPM_STANDBY_SUPPORTED == 1)
  if ( ( system_startup_done != FALSE ) && ( UTIL_LPM_GetMaxMode() >= UTIL_LPM_STANDBY_MODE ) )
#endif /* (CFG_LPM_STANDBY_SUPPORTED == 1) */
#endif /* (CFG_LPM_STOP2_SUPPORTED == 1) */
  {
    APP_SYS_EnterDeepSleep();
  }
#endif /* ((CFG_LPM_STANDBY_SUPPORTED == 1) || (CFG_LPM_STOP2_SUPPORTED == 1)) */
  LL_RCC_ClearResetFlags();

#if defined(STM32WBAXX_SI_CUT1_0)
  /* Wait until HSE is ready */
#if (CFG_SCM_SUPPORTED == 1)
  /* SCM HSE BEGIN */
  SCM_HSE_WaitUntilReady();
  /* SCM HSE END */
#else
  while (LL_RCC_HSE_IsReady() == 0);
#endif /* CFG_SCM_SUPPORTED */

  UTILS_ENTER_LIMITED_CRITICAL_SECTION(RCC_INTR_PRIO << 4U);
  scm_hserdy_isr();
  UTILS_EXIT_LIMITED_CRITICAL_SECTION();
#endif /* STM32WBAXX_SI_CUT1_0 */
#endif /* CFG_LPM_LEVEL */
  /* USER CODE BEGIN UTIL_SEQ_PreIdle_2 */

  /* USER CODE END UTIL_SEQ_PreIdle_2 */
  return;
}

void UTIL_SEQ_PostIdle( void )
{
  /* USER CODE BEGIN UTIL_SEQ_PostIdle_1 */

  /* USER CODE END UTIL_SEQ_PostIdle_1 */
#if ( CFG_LPM_LEVEL != 0)
  LL_AHB5_GRP1_EnableClock(LL_AHB5_GRP1_PERIPH_RADIO);
  (void)ll_sys_dp_slp_exit();
  UTIL_LPM_SetMaxMode(1U << CFG_LPM_LL_DEEPSLEEP, UTIL_LPM_MAX_MODE);
#endif /* CFG_LPM_LEVEL */
  /* USER CODE BEGIN UTIL_SEQ_PostIdle_2 */
#if ( (CFG_LPM_LEVEL != 0) && ( CFG_LPM_STANDBY_SUPPORTED != 0 ) )
  APP_BSP_PostIdle();
#endif /* ( CFG_LPM_LEVEL && CFG_LPM_STANDBY_SUPPORTED ) */

  /* USER CODE END UTIL_SEQ_PostIdle_2 */
  return;
}

void PKACTRL_CB_Process( void )
{
  UTIL_SEQ_SetTask(1U << CFG_TASK_PKACTRL, CFG_SEQ_PRIO_0);
}

/**
 * @brief Callback used by Random Number Generator to launch Task to generate Random Numbers
 */
void HWCB_RNG_Process( void )
{
  UTIL_SEQ_SetTask(1U << CFG_TASK_HW_RNG, TASK_PRIO_RNG);
}

void AMM_RegisterBasicMemoryManager (AMM_BasicMemoryManagerFunctions_t * const p_BasicMemoryManagerFunctions)
{
  /* Fulfill the function handle */
  p_BasicMemoryManagerFunctions->Init = AMM_WrapperInit;
  p_BasicMemoryManagerFunctions->Allocate = AMM_WrapperAllocate;
  p_BasicMemoryManagerFunctions->Free = AMM_WrapperFree;
}

void AMM_ProcessRequest(void)
{
  /* Trigger to call Advance Memory Manager process function */
  UTIL_SEQ_SetTask(1U << CFG_TASK_AMM, CFG_SEQ_PRIO_0);
}

static void AMM_WrapperInit(uint32_t * const p_PoolAddr, const uint32_t PoolSize)
{
  UTIL_MM_Init ((uint8_t *)p_PoolAddr, ((size_t)PoolSize * sizeof(uint32_t)));
}

static uint32_t * AMM_WrapperAllocate(const uint32_t BufferSize)
{
  return (uint32_t *)UTIL_MM_GetBuffer (((size_t)BufferSize * sizeof(uint32_t)));
}

static void AMM_WrapperFree (uint32_t * const p_BufferAddr)
{
  UTIL_MM_ReleaseBuffer ((void *)p_BufferAddr);
}

void FM_ProcessRequest(void)
{
  /* Trigger to call Flash Manager process function */
  UTIL_SEQ_SetTask(1U << CFG_TASK_FLASH_MANAGER, CFG_SEQ_PRIO_0);
}

#if ((CFG_LOG_SUPPORTED == 0) && (CFG_LPM_LEVEL != 0))
/* RNG module turn off HSI clock when traces are not used and low power used */
void RNG_KERNEL_CLK_OFF(void)
{
  /* USER CODE BEGIN RNG_KERNEL_CLK_OFF_1 */

  /* USER CODE END RNG_KERNEL_CLK_OFF_1 */
  LL_RCC_HSI_Disable();
  /* USER CODE BEGIN RNG_KERNEL_CLK_OFF_2 */

  /* USER CODE END RNG_KERNEL_CLK_OFF_2 */
}

#if (CFG_SCM_SUPPORTED == 1)
/* SCM module turn off HSI clock when traces are not used and low power used */
void SCM_HSI_CLK_OFF(void)
{
  /* USER CODE BEGIN SCM_HSI_CLK_OFF_1 */

  /* USER CODE END SCM_HSI_CLK_OFF_1 */
  LL_RCC_HSI_Disable();
  /* USER CODE BEGIN SCM_HSI_CLK_OFF_2 */

  /* USER CODE END SCM_HSI_CLK_OFF_2 */
}
#endif /* CFG_SCM_SUPPORTED */
#endif /* ((CFG_LOG_SUPPORTED == 0) && (CFG_LPM_LEVEL != 0)) */

#if (CFG_LOG_SUPPORTED != 0)
void UTIL_ADV_TRACE_PreSendHook(void)
{
#if (CFG_LPM_LEVEL != 0)
  /* Disable Stop mode before sending a LOG message over UART */
  UTIL_LPM_SetMaxMode(1U << CFG_LPM_LOG, UTIL_LPM_SLEEP_MODE);
#endif /* (CFG_LPM_LEVEL != 0) */
  /* USER CODE BEGIN UTIL_ADV_TRACE_PreSendHook */

  /* USER CODE END UTIL_ADV_TRACE_PreSendHook */
}

void UTIL_ADV_TRACE_PostSendHook(void)
{
#if (CFG_LPM_LEVEL != 0)
  /* Enable Stop mode after LOG message over UART sent */
  UTIL_LPM_SetMaxMode(1U << CFG_LPM_LOG, UTIL_LPM_MAX_MODE);
#endif /* (CFG_LPM_LEVEL != 0) */
  /* USER CODE BEGIN UTIL_ADV_TRACE_PostSendHook */

  /* USER CODE END UTIL_ADV_TRACE_PostSendHook */
}

/**
 * @brief  Treat Serial commands.
 *
 * @param  pRxBuffer      Pointer on received data from USART.
 * @param  iRxBufferSize  Number of received data.
 * @retval None
 */
void Serial_CMD_Interpreter_CmdExecute( uint8_t * pRxBuffer, uint16_t iRxBufferSize )
{
  /* USER CODE BEGIN Serial_CMD_Interpreter_CmdExecute_1 */

  /* USER CODE END Serial_CMD_Interpreter_CmdExecute_1 */
}

#endif /* (CFG_LOG_SUPPORTED != 0) */

int PKACTRL_MutexTake(void)
{
  int error = 0;

  /* Check if mutex is available */
  if (0u != PkaMut)
  {
    /* Clear flag */
    UTIL_SEQ_ClrEvt ((1u << CFG_PKA_MUTEX));

    /* Wait for flag to be raised */
    UTIL_SEQ_WaitEvt ((1u << CFG_PKA_MUTEX));
  }

  /* Increment mutex */
  PkaMut++;

  return error;
}

int PKACTRL_MutexRelease(void)
{
  int error = 0;

  if (0u != PkaMut)
  {
    PkaMut = 0u;

    /* Set the flag up */
    UTIL_SEQ_SetEvt ((1u << CFG_PKA_MUTEX));
  }

  return error;
}

int PKACTRL_TakeSemEndOfOperation(void)
{
  int error = 0;

  /* Check if semaphore is available */
  if (0u != EndOfProcSem)
  {
#if (CFG_LPM_LEVEL != 0)
  /* Avoid going in low power during computation */
  UTIL_LPM_SetMaxMode(1U << CFG_LPM_PKA_OVR_IT, UTIL_LPM_SLEEP_MODE);
#endif /* (CFG_LPM_LEVEL != 0) */

    /* Clear flag */
    UTIL_SEQ_ClrEvt ((1u << CFG_PKA_END_OF_PROCESS));

    /* Wait for flag to be raised */
    UTIL_SEQ_WaitEvt ((1u << CFG_PKA_END_OF_PROCESS));
  }

  /* Increment semaphore */
  EndOfProcSem++;

  return error;
}

int PKACTRL_ReleaseSemEndOfOperation(void)
{
  int error = 0;

  if (0u != EndOfProcSem)
  {
    EndOfProcSem = 0u;

    /* Set the flag up */
    UTIL_SEQ_SetEvt ((1u << CFG_PKA_END_OF_PROCESS));

#if (CFG_LPM_LEVEL != 0)
  /* Restore low power */
  UTIL_LPM_SetMaxMode(1U << CFG_LPM_PKA_OVR_IT, UTIL_LPM_MAX_MODE);
#endif /* (CFG_LPM_LEVEL != 0) */
  }

  return error;
}

FM_Cmd_Status_t FM_MutexTake(void)
{
  FM_Cmd_Status_t error = FM_OK;

  /* Check if mutex is available */
  if (0u != FlashMut)
  {
    /* Clear flag */
    UTIL_SEQ_ClrEvt ((1u << CFG_FLASH_MUTEX));

    /* Wait for flag to be raised */
    UTIL_SEQ_WaitEvt ((1u << CFG_FLASH_MUTEX));
  }

  /* Increment mutex */
  FlashMut++;

  return error;
}

FM_Cmd_Status_t FM_MutexRelease(void)
{
  FM_Cmd_Status_t error = FM_OK;

  if (0u != FlashMut)
  {
    FlashMut = 0u;

    /* Set the flag up */
    UTIL_SEQ_SetEvt ((1u << CFG_FLASH_MUTEX));
  }

  return error;
}

/* USER CODE BEGIN FD_WRAP_FUNCTIONS */
void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin)
{
  HAL_GPIO_EXTI_Rising_Callback(GPIO_Pin);
}

/* USER CODE END FD_WRAP_FUNCTIONS */
