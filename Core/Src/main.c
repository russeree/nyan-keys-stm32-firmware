/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "main.h"
#include "dma.h"
#include "i2c.h"
#include "rng.h"
#include "spi.h"
#include "tim.h"
#include "usb_otg.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usb_device.h"
#include "usbd_cdc_acm_if.h"
#include "24xx_eeprom.h"
#include "iceuncompr.h"
#include "lattice_ice_hx.h"
#include "nyan_os.h"
#include "nyan_leds.h"
#include "nyan_strings.h"
#include "nyan_keys.h"

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
double system_status_led_angle;

volatile NyanOS nos;
Iceuncompr ice_uncompr;
Eeprom24xx nos_eeprom;
LatticeIceHX nos_fpga;
NyanKeys nyan_keys;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  system_status_led_angle = 0;
  EepromInit(&nos_eeprom, true, true);
  NyanOsInit(&nos, &nos_eeprom);
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SPI1_Init();
  MX_SPI2_Init();
  MX_SPI4_Init();
  MX_I2C1_Init();
  MX_TIM7_Init();
  MX_TIM6_Init();
  MX_TIM1_Init();
  MX_USB_OTG_HS_PCD_Init();
  MX_RNG_Init();
  MX_TIM8_Init();
  /* USER CODE BEGIN 2 */
  // Activate the STM32F7 timer interrupts
  HAL_TIM_Base_Start_IT(&htim1);
  HAL_TIM_Base_Start_IT(&htim7);
  HAL_TIM_Base_Start_IT(&htim6);
  HAL_TIM_OC_Start_IT(&htim1, TIM_CHANNEL_1);
  HAL_TIM_OC_Start_IT(&htim1, TIM_CHANNEL_2);
  HAL_TIM_OC_Start_IT(&htim8, TIM_CHANNEL_1);
  // USB composite device creation
  MX_USB_DEVICE_Init();
  // FPGA Bitstream Loading
  FPGAInit(&nos_fpga);
  // Load up the fast cat IP for access to your keys; happy typing.
  NyanKeysInit(&nyan_keys);
  // Deinit the SPI4 interface until we need it again and replace it with GPIO inputs
  HAL_SPI_DeInit(&hspi4);
  NYAN_SPI4_GPIO_Init();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    if(nos_fpga.configured)
      NyanGetKeys(&nyan_keys);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 432;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
  nyan_keys.key_read_inflight = false;
}

void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *I2cHandle)
{
  nos_eeprom.tx_inflight = false;
}

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *I2cHandle)
{
  nos_eeprom.rx_inflight = false;
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *I2cHandle)
{
  uint8_t error = HAL_I2C_GetError(I2cHandle);
  switch (error)
  {
    case HAL_I2C_ERROR_AF :
      nos_eeprom.tx_failed = true;
      break;
    default:
      Error_Handler();
  }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM1) {
    HAL_GPIO_WritePin(GPIOD, Nyan_Keys_LED4_Pin, GPIO_PIN_SET);
    nos_fpga.configured = HAL_GPIO_ReadPin(Nyan_FPGA_Config_Done_GPIO_Port, Nyan_FPGA_Config_Done_Pin);
    // FPGA configuration done indicator
    if(nos_fpga.configured)
      HAL_GPIO_WritePin(Nyan_Keys_LED0_GPIO_Port, Nyan_Keys_LED0_Pin, GPIO_PIN_SET);
    else
      HAL_GPIO_WritePin(Nyan_Keys_LED0_GPIO_Port, Nyan_Keys_LED0_Pin, GPIO_PIN_RESET);
    // Nyan Keys FPGA IP running indicator
    if(HAL_GPIO_ReadPin(NYAN_SPI_GPIO_1_Port, NYAN_SPI_GPIO_1_Pin))
      HAL_GPIO_WritePin(Nyan_Keys_LED0_GPIO_Port, Nyan_Keys_LED1_Pin, GPIO_PIN_SET);
    else
      HAL_GPIO_WritePin(Nyan_Keys_LED0_GPIO_Port, Nyan_Keys_LED1_Pin, GPIO_PIN_RESET);
  }
  if (htim->Instance == TIM6) {
    // Increment the power on pulsing LED angle [sin^2(x) + cos^2(x) = 1]
    system_status_led_angle += SYSTEM_STATUS_DEGREE_INCREMENT;
  }
  if (htim->Instance == TIM7) {
    if(nos.send_welcome_screen_guard > 0) {
      if(++nos.send_welcome_screen_guard > _NYAN_WELCOME_GUARD_TIME) {
        nos.send_welcome_screen_guard = 0;
      }
    }
  }
}

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM1) {
    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
      // Pulse the SystemStatus LED off
      HAL_GPIO_WritePin(Nyan_Keys_LED4_GPIO_Port, Nyan_Keys_LED4_Pin, GPIO_PIN_RESET);
      // Now lets set the new capture compare register value.
      unsigned char cc_val = getSystemStatusOCRValue(system_status_led_angle);
      __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, (unsigned int)cc_val);
      __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_2, (unsigned int)cc_val);
    }
    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2) {
      HAL_GPIO_WritePin(Nyan_Keys_LED0_GPIO_Port, Nyan_Keys_LED0_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(Nyan_Keys_LED1_GPIO_Port, Nyan_Keys_LED1_Pin, GPIO_PIN_RESET);
    }
  }
  if (htim->Instance == TIM8) {
    // Clear the CDC TX buffer with Nyan large print support
    NyanCdcTX(&nos);
    // Every 200ms check to see if the welcome display needs to be presented
    if(nos.exe == NYAN_EXE_IDLE) {
      NyanWelcomeDisplay(&nos);
    }
    // Program Execution - Must be idle with no TXs inflight since we are modifying the ptr
    if(nos.exe != NYAN_EXE_IDLE && nos.tx_inflight == 0 && nos.exe_in_progress == 0) {
      NyanExecute(&nos);
    }
    // Turn off the RX CDC LED
    HAL_GPIO_WritePin(Nyan_Keys_LED3_GPIO_Port, Nyan_Keys_LED3_Pin, GPIO_PIN_RESET);
  }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
    // Turn on all LEDs on hard error
    HAL_GPIO_WritePin(GPIOD, Nyan_Keys_LED0_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOD, Nyan_Keys_LED1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOD, Nyan_Keys_LED2_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOD, Nyan_Keys_LED3_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOD, Nyan_Keys_LED4_Pin, GPIO_PIN_SET);
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     example: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
