/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
#include "main.h"
#include "adc.h"
#include "spi.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
// Определения пинов
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

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// Микрозадержка (можно изменить на более точную позже)
static void delay_ns(uint32_t ns) {
	// Простейшая заглушка — точную задержку можно потом сделать через DWT
	for (volatile uint32_t i = 0; i < (ns / 10 + 1); i++) {
		__NOP();
	}
}

void MCP4922_Write(uint8_t channel, uint16_t value) {
	// 1) Собираем слово кадра
	uint16_t frame = 0;
	frame |= ((channel & 0x1) << 15);  // D15 — канал: 0=A, 1=B
	frame |= (0 << 14);                // D14 — unbuffered (0)
	frame |= (1 << 13);                // D13 — gain = 1×
	frame |= (1 << 12);                // D12 — shutdown = active (1)
	frame |= (value & 0x0FFF);         // D11…D0 — 12-бит данные

	// разбиваем на байты
	uint8_t spiData[2] = { (uint8_t) (frame >> 8), (uint8_t) (frame & 0xFF) };

	// 2) SPI-передача с CS LOW→HIGH
	HAL_GPIO_WritePin(Chip_GPIO_Port, Chip_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi1, spiData, 2, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(Chip_GPIO_Port, Chip_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LDAC_GPIO_Port, LDAC_Pin, GPIO_PIN_RESET);
	delay_ns(100);
	HAL_GPIO_WritePin(LDAC_GPIO_Port, LDAC_Pin, GPIO_PIN_SET);

}

void MCP4922_Send(uint8_t control, uint8_t data) {
	// CS LOW
	HAL_GPIO_WritePin(Chip_GPIO_Port, Chip_Pin, GPIO_PIN_RESET);
	// Передаём сначала control, затем data
	HAL_SPI_Transmit(&hspi1, &control, 1, HAL_MAX_DELAY);
	HAL_SPI_Transmit(&hspi1, &data, 1, HAL_MAX_DELAY);
	// CS HIGH
	HAL_GPIO_WritePin(Chip_GPIO_Port, Chip_Pin, GPIO_PIN_SET);
}

static void HMC_WriteBit(uint8_t bit) {
	HAL_GPIO_WritePin(DATA_GPIO_Port, DATA_Pin,
			bit ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(SHIFT_CLK_GPIO_Port, SHIFT_CLK_Pin, GPIO_PIN_RESET);
	delay_ns(30);
	HAL_GPIO_WritePin(SHIFT_CLK_GPIO_Port, SHIFT_CLK_Pin, GPIO_PIN_SET);

	delay_ns(30);
}

// Установка затухания
void HMC_SetAttenuation(float attenuation_db, uint8_t data) {
	if (attenuation_db < 0.0f)
		attenuation_db = 0.0f;
	if (attenuation_db > 31.5f)
		attenuation_db = 31.5f;

	uint8_t att_half_db = (uint8_t) (attenuation_db * 2.0f + 0.5f); // округление до ближайшего
	if (att_half_db > 63)
		att_half_db = 63;
	//0b10011100

	//data = 0b10110100; // данные по даташиту (6 бит сдвинуты на 2 влево)

	HAL_GPIO_WritePin(LE_GPIO_Port, LE_Pin, GPIO_PIN_RESET);

	// Отправка всех 8 бит MSB first
	for (int i = 7; i >= 0; i--) {
		HMC_WriteBit((data >> i) & 0x01);
	}

	delay_ns(40); // Время установки LE
	HAL_GPIO_WritePin(LE_GPIO_Port, LE_Pin, GPIO_PIN_SET);
	delay_ns(40); // Удержание LE
	HAL_GPIO_WritePin(LE_GPIO_Port, LE_Pin, GPIO_PIN_RESET);
}

// Сброс устройства
void HMC_Reset(void) {
	HAL_GPIO_WritePin(RST_GPIO_Port, RST_Pin, GPIO_PIN_RESET);
	HAL_Delay(1);
	HAL_GPIO_WritePin(RST_GPIO_Port, RST_Pin, GPIO_PIN_SET);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM2_Init();
  MX_SPI1_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */
	// Инициализация начального состояния пинов
	HAL_GPIO_WritePin(LE_GPIO_Port, LE_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(SHIFT_CLK_GPIO_Port, SHIFT_CLK_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(DATA_GPIO_Port, DATA_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(RST_GPIO_Port, RST_Pin, GPIO_PIN_SET);

	// Сброс чипа
	HMC_Reset();

	HMC_SetAttenuation(15.5f, 0b10011100);
	HAL_GPIO_WritePin(SHDN_GPIO_Port, SHDN_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LDAC_GPIO_Port, LDAC_Pin, GPIO_PIN_RESET);
	HAL_Delay(100);
	MCP4922_Write(0, 0); //1230 - 1В

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	//HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);
	while (1) {
		if (HAL_GPIO_ReadPin(RSII_Q1_EX_GPIO_Port, RSII_Q1_EX_Pin)
				== GPIO_PIN_SET) {
			HMC_SetAttenuation(15.5f, 0b01011100);
			HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_SET);
		} else {
			HMC_SetAttenuation(15.5f, 0b10011100);
			HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_RESET);
		}
		//MCP4922_Write(0, 1028);  // Канал A, выдаёт примерно 3.0 В
		//MCP4922_Write(1, 1028);  // Канал A, выдаёт примерно 3.0 В
		//MCP4922_Send(controlA, dataA);
		//HAL_Delay(100);
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL12;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC12;
  PeriphClkInit.Adc12ClockSelection = RCC_ADC12PLLCLK_DIV1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

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
	while (1) {
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
