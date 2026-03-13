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
#define CC1201_PA_CFG1 0x2B
#define CC1201_PA_CFG0	0x2C
#define CC1201_FS_DSM1 0x2F1A

int ary = 0;
int rssi = 0;

int ary2 = 0;
int rssi2 = 0;

typedef struct {
	uint16_t dac_value;   // Значение ЦАП
	int8_t rssi_level;    // Уровень RSSI
} DAC_RSSI;

// Массив структур с данными из таблицы
DAC_RSSI dac_rssi_table[] = { { 0, -17 }, { 20, -16 }, { 50, -15 },
		{ 100, -14 }, { 170, -13 }, { 270, -12 }, { 400, -11 }, { 550, -10 }, {
				800, -9 }, // Предположим, что диапазон 1500-1600 представлен как 1500
		{ 1200, -8 }, { 1700, -7 }, { 2200, -6 }, { 4095, -5 }, };

// Размер массива
#define TABLE_SIZE (sizeof(dac_rssi_table) / sizeof(DAC_RSSI))

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

static void HMC_WriteBit2(uint8_t bit) {
	HAL_GPIO_WritePin(DATA2_GPIO_Port, DATA2_Pin,
			bit ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(SHIFT_CLK2_GPIO_Port, SHIFT_CLK2_Pin, GPIO_PIN_RESET);
	delay_ns(30);
	HAL_GPIO_WritePin(SHIFT_CLK2_GPIO_Port, SHIFT_CLK2_Pin, GPIO_PIN_SET);

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
void HMC_SetAttenuation2(float attenuation_db, uint8_t data) {
	if (attenuation_db < 0.0f)
		attenuation_db = 0.0f;
	if (attenuation_db > 31.5f)
		attenuation_db = 31.5f;

	uint8_t att_half_db = (uint8_t) (attenuation_db * 2.0f + 0.5f); // округление до ближайшего
	if (att_half_db > 63)
		att_half_db = 63;
	//0b10011100

	//data = 0b10110100; // данные по даташиту (6 бит сдвинуты на 2 влево)

	HAL_GPIO_WritePin(LE2_GPIO_Port, LE2_Pin, GPIO_PIN_RESET);

	// Отправка всех 8 бит MSB first
	for (int i = 7; i >= 0; i--) {
		HMC_WriteBit2((data >> i) & 0x01);
	}

	delay_ns(40); // Время установки LE
	HAL_GPIO_WritePin(LE2_GPIO_Port, LE2_Pin, GPIO_PIN_SET);
	delay_ns(40); // Удержание LE
	HAL_GPIO_WritePin(LE2_GPIO_Port, LE2_Pin, GPIO_PIN_RESET);
}

// Сброс устройства
void HMC_Reset(void) {
	HAL_GPIO_WritePin(RST_GPIO_Port, RST_Pin, GPIO_PIN_RESET);
	HAL_Delay(1);
	HAL_GPIO_WritePin(RST_GPIO_Port, RST_Pin, GPIO_PIN_SET);
}

void HMC2_Reset(void) {
	HAL_GPIO_WritePin(RST2_GPIO_Port, RST2_Pin, GPIO_PIN_RESET);
	HAL_Delay(1);
	HAL_GPIO_WritePin(RST2_GPIO_Port, RST2_Pin, GPIO_PIN_SET);
}

// Функция для установки значения ЦАП
void set_dac_value(uint16_t value, int num) {
	// Здесь должен быть код для установки значения ЦАП
	// Например, если используется HAL для STM32:
	if (num == 0) {
		MCP4922_Write(0, value);
	} else
		MCP4922_Write(1, value);
}

/////CC1200

typedef struct {
	uint16_t regAddr;
	uint8_t value;
} registerSetting_t;

static const registerSetting_t preferredSettings[] = { { CC1201_IOCFG2, 0x06 },
		{ CC1201_SYNC1, 0x6F }, { CC1201_SYNC0, 0x4E },
		{ CC1201_SYNC_CFG1, 0xE8 }, { CC1201_SYNC_CFG0, 0x13 }, {
				CC1201_DEVIATION_M, 0x99 }, { CC1201_MODCFG_DEV_E, 0x05 }, {
				CC1201_DCFILT_CFG, 0x26 }, { CC1201_PREAMBLE_CFG1, 0x15 }, {
				CC1201_PREAMBLE_CFG0, 0x8A }, { CC1201_IQIC, 0x00 }, {
				CC1201_CHAN_BW, 0x02 }, { CC1201_MDMCFG1, 0x42 }, {
				CC1201_MDMCFG0, 0x05 }, { CC1201_SYMBOL_RATE2, 0xC9 }, {
				CC1201_SYMBOL_RATE1, 0x99 }, { CC1201_SYMBOL_RATE0, 0x99 }, {
				CC1201_AGC_REF, 0x2F }, { CC1201_AGC_CS_THR, 0xEC }, {
				CC1201_AGC_CFG1, 0x16 }, { CC1201_AGC_CFG0, 0x84 }, {
				CC1201_FIFO_CFG, 0x00 }, { CC1201_FS_CFG, 0x1B }, {
				CC1201_PKT_CFG2, 0x20 }, { CC1201_PKT_CFG1, 0x83 }, {
				CC1201_PKT_CFG0, 0x20 }, { CC1201_PA_CFG1, 0x44 }, {
				CC1201_PKT_LEN, 0xFF }, { CC1201_IF_MIX_CFG, 0x18 }, {
				CC1201_TOC_CFG, 0x03 }, { CC1201_MDMCFG2, 0x00 }, {
				CC1201_FREQ2, 0x5B }, { CC1201_FREQ1, 0x80 }, { CC1201_IF_ADC1,
				0xEE }, { CC1201_IF_ADC0, 0x10 }, { CC1201_FS_DIG1, 0x04 }, {
				CC1201_FS_DIG0, 0x50 }, { CC1201_FS_CAL1, 0x40 }, {
				CC1201_FS_CAL0, 0x0E }, { CC1201_FS_DIVTWO, 0x03 }, {
				CC1201_FS_DSM0, 0x33 }, { CC1201_FS_DVC1, 0xF7 }, {
				CC1201_FS_DVC0, 0x0F }, { CC1201_FS_PFD, 0x00 }, {
				CC1201_FS_PRE, 0x6E }, { CC1201_FS_REG_DIV_CML, 0x1C }, {
				CC1201_FS_SPARE, 0xAC }, { CC1201_FS_VCO0, 0xB5 }, {
				CC1201_IFAMP, 0x0D }, { CC1201_XOSC5, 0x0E }, { CC1201_XOSC1,
				0x03 }, };
void halRfWriteReg(uint16_t regAddr, uint8_t value) {
	CC1200_CS_LOW(); // Активируем чип, устанавливая CS в низкое состояние

	uint8_t tempExt = (uint8_t) (regAddr >> 8);
	uint8_t tempAddr = (uint8_t) (regAddr & 0x00FF);
	uint8_t statusByte;

	if (tempExt) {
		uint8_t command = CC1200_WRITE | CC1200_EXT_ADDR;
		HAL_SPI_TransmitReceive(&hspi1, &command, &statusByte, 1,
		HAL_MAX_DELAY);
		HAL_SPI_Transmit(&hspi1, &tempAddr, 1, HAL_MAX_DELAY);
	} else {
		tempAddr = CC1200_WRITE | tempAddr;
		HAL_SPI_TransmitReceive(&hspi1, &tempAddr, &statusByte, 1,
		HAL_MAX_DELAY);
	}

	HAL_SPI_Transmit(&hspi1, &value, 1, HAL_MAX_DELAY);

	CC1200_CS_HIGH();
}

void CC1200_init() {
	for (int i = 0; i < sizeof(preferredSettings) / sizeof(registerSetting_t);
			++i) {
		halRfWriteReg(preferredSettings[i].regAddr, preferredSettings[i].value);
	}
}

void halRfWriteFifo(const uint8_t *data, uint8_t length) {
	for (uint8_t i = 0; i < length; i++) {
		halRfWriteReg(CC120X_TXFIFO, 1);
	}
}

void transmitMessage(const uint8_t *message, uint8_t length) {
	// Переход в режим IDLE перед настройкой для отправки
	halRfWriteReg(CC120X_SIDLE, CC120X_WRITE_SINGLE_BYTE);
	//while (halRfReadReg(CC120X_MARCSTATE) != MARCSTATE_IDLE);

	// Очистка TX FIFO перед загрузкой данных
	halRfWriteReg(CC120X_SFTX, CC120X_WRITE_SINGLE_BYTE);

	// Запись сообщения в TX FIFO
	halRfWriteFifo(message, length);

	// Начало передачи
	halRfWriteReg(CC120X_STX, CC120X_WRITE_SINGLE_BYTE);

	// Ожидание завершения передачи может потребовать дополнительной логики,
	// в зависимости от настройки прерываний или проверки статусных регистров.
}

uint8_t readRegisterEx(const uint8_t regAddr) {
	uint8_t readCommand = 0x80 | CC1200_EXT_ADDR;
	uint8_t statusByte = 0;
	uint8_t regValue = 0;

	CC1200_CS_LOW();

	HAL_SPI_TransmitReceive(&hspi1, &readCommand, &statusByte, 1,
	HAL_MAX_DELAY);
	HAL_SPI_TransmitReceive(&hspi1, &regAddr, &regValue, 1, HAL_MAX_DELAY);
	HAL_SPI_Receive(&hspi1, &regValue, 1, HAL_MAX_DELAY);

	CC1200_CS_HIGH();

	return regValue;
}
uint8_t readRegister(const uint8_t regAddr) {
	uint8_t readCommand = regAddr | 0x80 | 0x00;
	uint8_t statusByte = 0;
	uint8_t regValue = 0;

	CC1200_CS_LOW();

	HAL_SPI_TransmitReceive(&hspi1, &readCommand, &statusByte, 1,
	HAL_MAX_DELAY);
	//HAL_SPI_TransmitReceive(&hspi1, (uint8_t[]){0xFF}, &regValue, 1, HAL_MAX_DELAY);
	HAL_SPI_Receive(&hspi1, &regValue, 1, HAL_MAX_DELAY);

	CC1200_CS_HIGH();

	return regValue;
}
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

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
	/* USER CODE BEGIN 2 */
	// Инициализация начального состояния пинов
	HAL_GPIO_WritePin(LE_GPIO_Port, LE_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(SHIFT_CLK_GPIO_Port, SHIFT_CLK_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(DATA_GPIO_Port, DATA_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(RST_GPIO_Port, RST_Pin, GPIO_PIN_SET);

	HAL_GPIO_WritePin(LE2_GPIO_Port, LE2_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(SHIFT_CLK2_GPIO_Port, SHIFT_CLK2_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(DATA2_GPIO_Port, DATA2_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(RST2_GPIO_Port, RST2_Pin, GPIO_PIN_SET);

	// Сброс чипа
	HMC_Reset();
	HMC2_Reset();

	HMC_SetAttenuation(15.5f, 0b10111100); //0b10011100
	HMC_SetAttenuation2(15.5f, 0b11000100);
	HAL_GPIO_WritePin(SHDN_GPIO_Port, SHDN_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LDAC_GPIO_Port, LDAC_Pin, GPIO_PIN_RESET);
	HAL_Delay(100);
	MCP4922_Write(1, 4095); //1230 - 1В
	HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_RESET);
	//HAL_GPIO_WritePin(CC_RESET_GPIO_Port, CC_RESET_Pin, GPIO_PIN_RESET);

	CC1200_CS_HIGH();
	HAL_GPIO_WritePin(RES_CC_GPIO_Port, RES_CC_Pin, 0);
	HAL_Delay(100);
	HAL_GPIO_WritePin(RES_CC_GPIO_Port, RES_CC_Pin, 1);

	halRfWriteReg(0x1B, 0x22);
	uint8_t partNum = readRegister(0x1B);
	printf("Part Number: %d\n", partNum);

	uint8_t version = readRegister(VERSION_REG);
	printf("Version: %d\n", version);

	CC1200_init();
	//HMC_SetAttenuation(15.5f, 0b01011100);
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	//HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);
	uint8_t message[] = "Hello, RF World!";
	transmitMessage(message, sizeof(message) - 1);
	uint32_t trigger_dac0 = 0;
	uint32_t trigger_dac1 = 0;
	while (1) {

		int8_t found_rssi_level = 0;
		    int8_t found_rssi_level2 = 0;

		    uint8_t ch1_ready = 0;
		    uint8_t ch2_ready = 0;

		    for (uint8_t i = TABLE_SIZE - 1; i > 0; i--) {

		        if (ary2 == 0) {
		            MCP4922_Write(0, dac_rssi_table[i].dac_value);
		        }

		        if (ary == 0) {
		            MCP4922_Write(1, dac_rssi_table[i].dac_value);
		        }

		        HAL_Delay(10);

		        /* -------- CHANNEL 1 -------- */

		        if (HAL_GPIO_ReadPin(RSII_Q1_EX_GPIO_Port, RSII_Q1_EX_Pin) == GPIO_PIN_SET) {

		            if (ary2 == 0) {

		                found_rssi_level = dac_rssi_table[i].rssi_level;

		                if (found_rssi_level >= -8) {
		                    ch1_ready = 1;
		                    trigger_dac0 = dac_rssi_table[i].dac_value;
		                }
		            }

		        } else {

		            if (ary2 == 1) {

		               // HMC_SetAttenuation2(15.5f, 0b11000100);
		                //ary2 = 0;

		                if (ary == 0) {
		                    HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_RESET);
		                }
		            }
		        }

		        /* -------- CHANNEL 2 -------- */

		        if (HAL_GPIO_ReadPin(RSII_Q2_EX_GPIO_Port, RSII_Q2_EX_Pin) == GPIO_PIN_SET) {

		            if (ary == 0) {

		                found_rssi_level2 = dac_rssi_table[i].rssi_level;

		                if (found_rssi_level2 >= -8) { //-8
		                    ch2_ready = 1;
		                    trigger_dac1 = dac_rssi_table[i].dac_value;
		                }
		            }

		        } else {

		            if (ary == 1) {

		                HMC_SetAttenuation(15.5f, 0b10111100);
		                HMC_SetAttenuation2(15.5f, 0b11000100);
		                ary = 0;
		                ary2 = 0;
		                if (ary2 == 0) {
		                    HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_RESET);
		                }
		            }
		        }

		        /* -------- ВКЛЮЧЕНИЕ АРУ СРАЗУ НА ДВУХ -------- */

		        if (ch1_ready || ch2_ready && ary == 0 && ary2 == 0) {

		            HMC_SetAttenuation2(15.5f, 0b10001100);
		            HMC_SetAttenuation(15.5f, 0b10001100);

		            ary2 = 1;
		            ary = 1;

		            HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_SET);

		            HAL_Delay(100);

		            MCP4922_Write(0, 10); //160
		            MCP4922_Write(1, 10);

		            HAL_Delay(100);

		            break; // выходим из цикла таблицы
		        }
		    }

		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL12;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK) {
		Error_Handler();
	}
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
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
