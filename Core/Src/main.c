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
#include <CC1200_rx_reg_values_my.h>
#include <stdio.h>
#include <stdlib.h>
#include "CC1200_commands.h"
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

#define CC_CS_ON()  HAL_GPIO_WritePin(CC_CS_GPIO_Port, CC_CS_Pin, GPIO_PIN_RESET) //включение
#define CC_CS_OFF() HAL_GPIO_WritePin(CC_CS_GPIO_Port, CC_CS_Pin, GPIO_PIN_SET) //выключение

#define MCP_CS_ON()  HAL_GPIO_WritePin(MCP_CS_GPIO_Port, MCP_CS_Pin, GPIO_PIN_RESET) //включение
#define MCP_CS_OFF() HAL_GPIO_WritePin(MCP_CS_GPIO_Port, MCP_CS_Pin, GPIO_PIN_SET) //выключение

uint8_t spiByte; //for SPI work
uint8_t spiStatusByte; //for SPI work
const uint8_t dummyByte=0xFF; //for SPI read
uint8_t cc_received_byte;

uint8_t received_bytes_arr[10000]={0};
int received_bytes_arr_ptr=0;


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

///CC1200

void CC1200_rx_write_reg(uint16_t regAddr, uint8_t value) {

	CC_CS_ON();

	uint8_t addr_h = (uint8_t)(regAddr >> 8);
	uint8_t addr_l = (uint8_t)(regAddr & 0x00FF);

	if (addr_h) //если 2-байтный адрес
	{
		uint8_t command = CC1200_WRITE | addr_h;
		HAL_SPI_TransmitReceive(&hspi1, &command, &spiByte, 1, HAL_MAX_DELAY);
		HAL_SPI_TransmitReceive(&hspi1, &addr_l, &spiByte, 1, HAL_MAX_DELAY);
	}
	else //если 1-байтный адрес
	{
		uint8_t command = CC1200_WRITE | addr_l;
		HAL_SPI_TransmitReceive(&hspi1, &command, &spiByte, 1, HAL_MAX_DELAY);
	}
	HAL_SPI_Transmit(&hspi1, &value, 1, HAL_MAX_DELAY);//значение регистра

	CC_CS_OFF();
}

uint8_t CC1200_rx_read_reg(uint16_t regAddr) {

	CC_CS_ON();

	uint8_t addr_h = (uint8_t)(regAddr >> 8);
	uint8_t addr_l = (uint8_t)(regAddr & 0x00FF);

	if (addr_h) //если 2-байтный адрес
	{
		uint8_t command = CC1200_READ | addr_h;
		HAL_SPI_TransmitReceive(&hspi1, &command, &spiByte, 1, HAL_MAX_DELAY);
		HAL_SPI_TransmitReceive(&hspi1, &addr_l, &spiByte, 1, HAL_MAX_DELAY);
	}
	else //если 1-байтный адрес
	{
		uint8_t command = CC1200_WRITE | addr_l;
		HAL_SPI_TransmitReceive(&hspi1, &command, &spiByte, 1, HAL_MAX_DELAY);
	}
	HAL_SPI_TransmitReceive(&hspi1, (uint8_t*)&dummyByte, &spiByte, 1, HAL_MAX_DELAY);//send dummy byte for read register

	CC_CS_OFF();

	return spiByte;//значение регистра
}

void CC1200_rx_send_command(uint8_t value) {

	CC_CS_ON();

	HAL_SPI_TransmitReceive(&hspi1, &value, &spiStatusByte,1, HAL_MAX_DELAY); //команда //устанавливает StatusByte

	CC_CS_OFF();
}


uint8_t get_cc_state(){
	CC1200_rx_send_command(CC1200_SNOP);
	return ( spiStatusByte & CC1200_STATUS_BYTE_STATE_MASK ) >> CC1200_STATUS_BYTE_STATE_SHIFT;
}


void  CC1200_rx_init(){

	CC_CS_OFF();
	HAL_GPIO_WritePin(RES_CC_GPIO_Port, RES_CC_Pin, 0);
	HAL_Delay(100);
	HAL_GPIO_WritePin(RES_CC_GPIO_Port, RES_CC_Pin, 1);

	CC_CS_ON(); //включение приёма по SPI
	HAL_Delay(50);

	for (int i = 0; i < sizeof(preferredSettings) / sizeof(registerSetting_t); ++i) {
		CC1200_rx_write_reg(preferredSettings[i].regAddr, preferredSettings[i].value);
	}

	CC1200_rx_send_command(CC1200_SIDLE);
	CC1200_rx_send_command(CC1200_SFRX);
	CC1200_rx_send_command(CC1200_SRX);
	HAL_Delay(50);
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


	//CC1200_init();
	CC1200_rx_init();
	//HMC_SetAttenuation(15.5f, 0b01011100);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	//HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);

	uint32_t trigger_dac0 = 0;
	uint32_t trigger_dac1 = 0;

	while (1)
		{
			//это временный вариант - мигает светодиодом, если есть байты в буфере FIFO
			//HAL_Delay(1);
			const int toggle_led_freq_divider=10;
			static int toggle_led_counter=0;

			uint8_t cc_state=get_cc_state();
			switch(cc_state)
			{
			case 0b001: //rx mode
				int num_of_bytes = CC1200_rx_read_reg ( CC1200_NUM_RXBYTES );
				if ( num_of_bytes > 0)
				{
					cc_received_byte=CC1200_rx_read_reg(CC1200_FIFO);

	//				received_bytes_arr[received_bytes_arr_ptr]=cc_received_byte;	//использовать только для отладки!!!
	//				received_bytes_arr_ptr++; 										//использовать только для отладки!!!
	//				if(received_bytes_arr_ptr>500){									//использовать только для отладки!!!
	//					received_bytes_arr_ptr=0;									//использовать только для отладки!!!
	//				}																//использовать только для отладки!!!

					toggle_led_counter++;
					if(toggle_led_counter>toggle_led_freq_divider)
					{
						HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
						toggle_led_counter=0;
					}
				}else
				{
					HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_RESET);
				}
				break;

			case 0b000: //idle
				CC1200_rx_send_command(CC1200_SRX);
				break;

			case 0b110: //rx fifo error
				CC1200_rx_send_command(CC1200_SFRX);
				CC1200_rx_send_command(CC1200_SRX);
				break;

			case 0b111: //tx fifo error
				CC1200_rx_send_command(CC1200_SFTX);
				CC1200_rx_send_command(CC1200_SRX);
				break;
			}

	    /* USER CODE END WHILE */

	    /* USER CODE BEGIN 3 */
		}







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
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

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
