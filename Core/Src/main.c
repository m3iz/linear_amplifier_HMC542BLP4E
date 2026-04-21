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

#define CC1200_TXFIFO   0x3F
#define CC1200_RXFIFO   0x3F


/////CC1200
uint8_t CC1200_ReadReg(uint16_t addr)
{
    uint8_t tx[3] = {0};
    uint8_t rx[3] = {0};

    CC1200_CS_LOW();

    if (addr >= 0x80) {                     // Extended register
        tx[0] = 0xAF;                       // Read + Extended
        tx[1] = (uint8_t)addr;
        tx[2] = 0x00;
        HAL_SPI_TransmitReceive(&hspi1, tx, rx, 3, HAL_MAX_DELAY);
        CC1200_CS_HIGH();
        return rx[2];
    } else {
        tx[0] = (uint8_t)addr | 0x80;
        HAL_SPI_TransmitReceive(&hspi1, tx, rx, 1, HAL_MAX_DELAY);
        HAL_SPI_Receive(&hspi1, &rx[1], 1, HAL_MAX_DELAY);
        CC1200_CS_HIGH();
        return rx[1];
    }
}

// ====================== ПРАВИЛЬНАЯ ЗАПИСЬ ======================
void CC1200_WriteReg(uint16_t addr, uint8_t value)
{
    uint8_t tx[3];
    CC1200_CS_LOW();

    if (addr >= 0x80) {
        tx[0] = 0x2F;                       // Extended write
        tx[1] = (uint8_t)addr;
        tx[2] = value;
        HAL_SPI_Transmit(&hspi1, tx, 3, HAL_MAX_DELAY);
    } else {
        tx[0] = (uint8_t)addr;
        tx[1] = value;
        HAL_SPI_Transmit(&hspi1, tx, 2, HAL_MAX_DELAY);
    }
    CC1200_CS_HIGH();
}

void CC1200_Strobe(uint8_t cmd)
{
    CC1200_CS_LOW();
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    CC1200_CS_HIGH();
}

uint8_t CC1200_ReadStrobe(uint8_t cmd){
	CC1200_CS_LOW();
	uint8_t res = 0;
	HAL_SPI_TransmitReceive(&hspi1, &res, &cmd, sizeof(cmd), HAL_MAX_DELAY);
	return res;
}
typedef struct {
	uint16_t regAddr;
	uint8_t value;
} registerSetting_t;

uint8_t CC1200_ReceivePacket(uint8_t *buffer, uint8_t maxLen)
{
    // Читаем количество байт в RX FIFO
    uint8_t numBytes = CC1200_ReadReg(0x97);  // NUM_RXBYTES
    if (numBytes == 0) return 0;

    // Если есть данные, читаем их
    uint8_t len = 0;
    uint8_t cmd = 0xC0 | CC1200_RXFIFO;   // burst read

    CC1200_CS_LOW();
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    // Первый байт — это длина пакета (если включен формат с длиной)
    HAL_SPI_Receive(&hspi1, &len, 1, HAL_MAX_DELAY);
    if (len > maxLen) len = maxLen;
    if (len > 0) {
        HAL_SPI_Receive(&hspi1, buffer, len, HAL_MAX_DELAY);
    }
    CC1200_CS_HIGH();

    // Сбросить FIFO после чтения
    CC1200_Strobe(CC1200_SFRX);
    return len;
}

static const registerSetting_t preferredSettings[] = {
    { CC1200_IOCFG0,         0x06 },    // GPIO0 IO Pin Configuration
    { CC1200_IOCFG2,         0x00 },    // GPIO2 IO Pin Configuration
    { CC1200_IOCFG3,         0x02 },    // GPIO3 IO Pin Configuration
    { CC1200_SYNC2,          0x6C },    // Sync Word Configuration [23:16]
    { CC1200_SYNC1,          0x94 },    // Sync Word Configuration [15:8]
    { CC1200_SYNC0,          0xA6 },    // Sync Word Configuration [7:0]
    { CC1200_SYNC_CFG1,      0x88 },    // Sync Word Detection Configuration Reg. 1
    { CC1200_SYNC_CFG0,      0x23 },    // Sync Word Detection Configuration Reg. 0
    { CC1200_DEVIATION_M,    0xD1 },    // Frequency Deviation Configuration
    { CC1200_MODCFG_DEV_E,   0x08 },    // Modulation Format and Frequency Deviation Configur..
    { CC1200_DCFILT_CFG,     0x4B },    // Digital DC Removal Configuration
    { CC1200_PREAMBLE_CFG1,  0x08 },    // Preamble Length Configuration Reg. 1
    { CC1200_PREAMBLE_CFG0,  0x8A },    // Preamble Detection Configuration Reg. 0
    { CC1200_IQIC,           0xD8 },    // Digital Image Channel Compensation Configuration
    { CC1200_CHAN_BW,        0x95 },    // Channel Filter Configuration
    { CC1200_MDMCFG1,        0x42 },    // General Modem Parameter Configuration Reg. 1
    { CC1200_MDMCFG0,        0x05 },    // General Modem Parameter Configuration Reg. 0
    { CC1200_SYMBOL_RATE2,   0x60 },    // Symbol Rate Configuration Exponent and Mantissa [1..
    { CC1200_SYMBOL_RATE1,   0x62 },    // Symbol Rate Configuration Mantissa [15:8]
    { CC1200_SYMBOL_RATE0,   0x4E },    // Symbol Rate Configuration Mantissa [7:0]
    { CC1200_AGC_REF,        0x1F },    // AGC Reference Level Configuration
    { CC1200_AGC_CS_THR,     0xF6 },    // Carrier Sense Threshold Configuration
    { CC1200_AGC_CFG1,       0x12 },    // Automatic Gain Control Configuration Reg. 1
    { CC1200_AGC_CFG0,       0x80 },    // Automatic Gain Control Configuration Reg. 0
    { CC1200_FIFO_CFG,       0x60 },    // FIFO Configuration
    { CC1200_FS_CFG,         0x1B },    // Frequency Synthesizer Configuration
    { CC1200_PKT_CFG2,       0x00 },    // Packet Configuration Reg. 2
    { CC1200_PKT_CFG1,       0x00 },    // Packet Configuration Reg. 1
    { CC1200_PKT_CFG0,       0x00 },    // Packet Configuration Reg. 0
    { CC1200_RFEND_CFG1,     0x3F },    // RFEND Configuration Reg. 1
    { CC1200_PKT_LEN,        0x10 },    // Packet Length Configuration
    { CC1200_IF_MIX_CFG,     0x1C },    // IF Mix Configuration
    { CC1200_TOC_CFG,        0x03 },    // Timing Offset Correction Configuration
    { CC1200_MDMCFG2,        0x02 },    // General Modem Parameter Configuration Reg. 2
    { CC1200_FREQ2,          0x5B },    // Frequency Configuration [23:16]
    { CC1200_FREQ1,          0x33 },    // Frequency Configuration [15:8]
    { CC1200_FREQ0,          0x33 },    // Frequency Configuration [7:0]
    { CC1200_IF_ADC1,        0xEE },    // Analog to Digital Converter Configuration Reg. 1
    { CC1200_IF_ADC0,        0x10 },    // Analog to Digital Converter Configuration Reg. 0
    { CC1200_FS_DIG1,        0x04 },    // Frequency Synthesizer Digital Reg. 1
    { CC1200_FS_DIG0,        0x50 },    // Frequency Synthesizer Digital Reg. 0
    { CC1200_FS_CAL1,        0x40 },    // Frequency Synthesizer Calibration Reg. 1
    { CC1200_FS_CAL0,        0x0E },    // Frequency Synthesizer Calibration Reg. 0
    { CC1200_FS_DIVTWO,      0x03 },    // Frequency Synthesizer Divide by 2
    { CC1200_FS_DSM0,        0x33 },    // FS Digital Synthesizer Module Configuration Reg. 0
    { CC1200_FS_DVC1,        0xF7 },    // Frequency Synthesizer Divider Chain Configuration ..
    { CC1200_FS_DVC0,        0x0F },    // Frequency Synthesizer Divider Chain Configuration ..
    { CC1200_FS_PFD,         0x00 },    // Frequency Synthesizer Phase Frequency Detector Con..
    { CC1200_FS_PRE,         0x6E },    // Frequency Synthesizer Prescaler Configuration
    { CC1200_FS_REG_DIV_CML, 0x1C },    // Frequency Synthesizer Divider Regulator Configurat..
    { CC1200_FS_SPARE,       0xAC },    // Frequency Synthesizer Spare
    { CC1200_FS_VCO0,        0xB5 },    // FS Voltage Controlled Oscillator Configuration Reg..
    { CC1200_IFAMP,          0x09 },    // Intermediate Frequency Amplifier Configuration
    { CC1200_XOSC5,          0x0E },    // Crystal Oscillator Configuration Reg. 5
    { CC1200_XOSC1,          0x03 },    // Crystal Oscillator Configuration Reg. 1
};

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

void CC1200_init(void) {
    for (int i = 0; i < sizeof(preferredSettings) / sizeof(registerSetting_t); i++) {
        CC1200_WriteReg(preferredSettings[i].regAddr, preferredSettings[i].value);
    }
    // Калибровка частоты
    CC1200_Strobe(CC120X_SCAL);
}

void CC1200_EnterRxMode(void) {
    CC1200_Strobe(CC120X_SRX);
}

void halRfWriteFifo(const uint8_t *data, uint8_t length) {
	for (uint8_t i = 0; i < length; i++) {
		halRfWriteReg(CC120X_TXFIFO, data[i]);
	}
}

void transmitMessage(const uint8_t *message, uint8_t length) {
	// Переход в режим IDLE перед настройкой для отправки
	CC1200_Strobe(CC120X_SIDLE);

	// Очистка TX FIFO перед загрузкой данных
	CC1200_Strobe(CC120X_SFTX);

	// Запись сообщения в TX FIFO
	halRfWriteFifo(message, length);

	// Начало передачи
	CC1200_Strobe(CC120X_STX);
}

uint8_t readRegisterEx(uint8_t regAddr) {
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
int16_t CC1200_GetRssiDbm(void) {
    uint8_t rssi1Val = CC1200_ReadReg(0x2F71);
    uint8_t rssi0Val = CC1200_ReadReg(0x2F72);

    // Проверка валидности измерения (бит 7 в RSSI0)
    if (!(rssi0Val & (1 << 7))) {   // предположим, RSSI0_RSSI_VALID = 7
        return INT16_MIN;           // сигнал не валиден, вернуть минимальное значение
    }

    // Собираем 12-битное знаковое число
    uint16_t raw = (rssi1Val << 4) | ((rssi0Val >> 4) & 0x0F);

    // Преобразуем из дополнительного кода (12 бит) в int16_t
    int16_t rssi_signed;
    if (raw & 0x800) {          // если отрицательное
        rssi_signed = raw - 4096;
    } else {
        rssi_signed = raw;
    }

    // Переводим в dBm: значение * 0.0625 (шаг квантования)
    // Округляем до целого, чтобы вернуть int16_t
    float rssi_dbm_float = rssi_signed * 0.0625f;
    return (int16_t)(rssi_dbm_float + 0.5f);   // округление
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


	CC1200_CS_HIGH();
	HAL_GPIO_WritePin(RES_CC_GPIO_Port, RES_CC_Pin, 0);
	HAL_Delay(100);
	HAL_GPIO_WritePin(RES_CC_GPIO_Port, RES_CC_Pin, 1);
	CC1200_Strobe(CC1200_SRES);
	HAL_Delay(100);

	uint8_t partNum = CC1200_ReadReg(0x2F8F);
	//uint8_t partVer = CC1200_ReadReg(0x2F73);
	uint8_t stres = CC1200_ReadStrobe(0x3D);
	CC1200_init();
	uint8_t f2 = CC1200_ReadReg(0x2F0C);
	uint8_t f1 = CC1200_ReadReg(0x2F0D);
	uint8_t f0 = CC1200_ReadReg(0x2F0E);
	  HAL_Delay(10);
	  uint8_t state = CC1200_ReadReg(0x2F73);
	  CC1200_Strobe(CC1200_SIDLE);
	  Hal_Delay(10);
	  stres = CC1200_ReadStrobe(0x3D);
	  CC1200_Strobe(CC1200_SCAL);
	  HAL_Delay(1);
	  while ((CC1200_ReadReg(0x2F73) & 0x1F) != 0x01) {
	      HAL_Delay(1);
	  }
	    HAL_Delay(10);
	    CC1200_Strobe(CC1200_SFRX);
	    HAL_Delay(10);

	    //CC1200_Strobe(0x33);
	    //while ((CC1200_ReadReg(0x2F73) & 0x1F) != 0x01) {}
	    CC1200_Strobe(CC1200_SRX);
	    HAL_Delay(10);
	    stres = CC1200_ReadStrobe(0x3D);
	    uint8_t test = CC1200_ReadReg(0x2F73);
	    if ((CC1200_ReadReg(0x2F73) & 0x1F) == 0x10) {
	        printf("Успешно перешли в режим RX\n");
	    }
	//HMC_SetAttenuation(15.5f, 0b01011100);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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
