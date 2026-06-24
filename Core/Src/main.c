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
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
const uint8_t scrambler_tbl[] = { 0x00, 0xa5, 0xd2, 0x69, 0xb4, 0xda, 0xed,
		0x76, 0xbb, 0x5d, 0xae, 0xd7, 0x6b, 0xb5, 0x5a, 0xad, 0x56, 0xab, 0xd5,
		0x6a, 0x35, 0x1a, 0x8d, 0x46, 0x23, 0x11, 0x88, 0x44, 0x22, 0x91, 0xc8,
		0x64, 0x32, 0x19, 0x0c, 0x86, 0x43, 0x21, 0x10, 0x08, 0x04, 0x02, 0x81,
		0x40, 0xa0, 0xd0, 0x68, 0x34, 0x9a, 0xcd, 0x66, 0x33, 0x99, 0x4c, 0xa6,
		0x53, 0xa9, 0xd4, 0xea, 0x75, 0x3a, 0x9d, 0xce, 0xe7, 0xf3, 0xf9, 0x7c,
		0x3e };

typedef struct {
	int8_t power_dbm;
	uint8_t pa_cfg1_value;
} TxPowerEntry;

const TxPowerEntry txPowerTable[] = { { 14, 0x7F }, { 13, 0x7C }, { 12, 0x7A },
		{ 11, 0x78 }, { 10, 0x76 }, { 9, 0x73 }, { 8, 0x71 }, { 7, 0x6E }, { 6,
				0x6C }, { 5, 0x6A }, { 4, 0x68 }, { 3, 0x66 }, { 2, 0x63 }, { 1,
				0x61 }, { 0, 0x5F }, { -3, 0x58 }, { -6, 0x51 }, { -11, 0x46 },
		{ -12, 0x44 }, { -24, 0x42 }, { -40, 0x41 }, };

typedef struct {
	char registerValue;
	float value_in_db_from_datsheet;
	float value_in_db_real;
	int value_in_db_to_display;
} attValues_t;

const attValues_t attValue[] = {

{ 0b11111100, 0, 1.2, 1 }, { 0b11110100, 1, 2.3, 2 }, { 0b11101100, 2, 3.3, 3 },
		{ 0b11100100, 3, 4.3, 4 }, { 0b11011100, 4, 5.3, 5 }, { 0b11010100, 5,
				5.3, 6 }, { 0b11001100, 6, 7.3, 7 }, { 0b11000100, 7, 8.3, 8 },
		{ 0b10111100, 8, 9.2, 9 }, { 0b10110100, 9, 10.2, 10 }, { 0b10101100,
				10, 11.2, 11 }, { 0b10100100, 11, 12.2, 12 }, { 0b10011100, 12,
				13.2, 13 }, { 0b10010100, 13, 14.2, 14 }, { 0b10001100, 14,
				15.2, 15 }, { 0b10000100, 15, 16.2, 16 }, { 0b01111100, 16,
				17.1, 17 }, { 0b01110100, 17, 18.1, 18 }, { 0b01101100, 18,
				19.1, 19 }, { 0b01100100, 19, 20.1, 20 }, { 0b01011100, 20,
				21.1, 21 }, { 0b01010100, 21, 22.1, 22 }, { 0b01001100, 22,
				23.1, 23 }, { 0b01000100, 23, 24.1, 24 }, { 0b00111100, 24,
				25.1, 25 }, { 0b00110100, 25, 26.1, 26 }, { 0b00101100, 26,
				27.1, 27 }, { 0b00100100, 27, 28.1, 28 }, { 0b00011100, 28,
				29.1, 29 }, { 0b00010100, 29, 30.1, 30 }, { 0b00001100, 30,
				31.1, 31 }, { 0b00000100, 31, 32.1, 32 },

};
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

#define CC_CS_ON()  HAL_GPIO_WritePin(CC_CS_GPIO_Port, CC_CS_Pin, GPIO_PIN_RESET)
#define CC_CS_OFF() HAL_GPIO_WritePin(CC_CS_GPIO_Port, CC_CS_Pin, GPIO_PIN_SET)

#define MCP_CS_ON()  HAL_GPIO_WritePin(MCP_CS_GPIO_Port, MCP_CS_Pin, GPIO_PIN_RESET)
#define MCP_CS_OFF() HAL_GPIO_WritePin(MCP_CS_GPIO_Port, MCP_CS_Pin, GPIO_PIN_SET)

uint8_t spiByte; //for SPI work
uint8_t spiStatusByte; //for SPI work
const uint8_t dummyByte = 0xFF; //for SPI read
uint8_t cc_received_byte;

uint8_t received_bytes_arr[10000] = { 0 };
int received_bytes_arr_ptr = 0;

int ary = 0;
int rssi = 0;

int ary2 = 0;
int rssi2 = 0;

typedef struct {
	uint16_t dac_value;
	int8_t rssi_level;
} DAC_RSSI;

DAC_RSSI dac_rssi_table[] = { { 0, -17 }, { 20, -16 }, { 50, -15 },
		{ 100, -14 }, { 170, -13 }, { 270, -12 }, { 400, -11 }, { 550, -10 }, {
				800, -9 }, { 1200, -8 }, { 1700, -7 }, { 2200, -6 },
		{ 4095, -5 }, };

struct amp_settings {
	unsigned gain :3;
	unsigned bias1 :2;
	unsigned bias2 :2;
	unsigned vgain :1;
	unsigned vbias :3;
	unsigned preamble :2;
	unsigned video_ena :1;
	unsigned diag_ena :1;
} amp_settings1;

#define TABLE_SIZE (sizeof(dac_rssi_table) / sizeof(DAC_RSSI))

static void delay_ns(uint32_t ns) {
	for (volatile uint32_t i = 0; i < (ns / 10 + 1); i++) {
		__NOP();
	}
}

void MCP4922_Write(uint8_t channel, uint16_t value) {
	uint16_t frame = 0;
	frame |= ((channel & 0x1) << 15);  // D15 — канал: 0=A, 1=B
	frame |= (0 << 14);                // D14 — unbuffered (0)
	frame |= (1 << 13);                // D13 — gain = 1×
	frame |= (1 << 12);                // D12 — shutdown = active (1)
	frame |= (value & 0x0FFF);         // D11…D0 — 12-бит данные

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
	HAL_GPIO_WritePin(Chip_GPIO_Port, Chip_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi1, &control, 1, HAL_MAX_DELAY);
	HAL_SPI_Transmit(&hspi1, &data, 1, HAL_MAX_DELAY);
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

	uint8_t addr_h = (uint8_t) (regAddr >> 8);
	uint8_t addr_l = (uint8_t) (regAddr & 0x00FF);

	if (addr_h) //если 2-байтный адрес
	{
		uint8_t command = CC1200_WRITE | addr_h;
		HAL_SPI_TransmitReceive(&hspi1, &command, &spiByte, 1, HAL_MAX_DELAY);
		HAL_SPI_TransmitReceive(&hspi1, &addr_l, &spiByte, 1, HAL_MAX_DELAY);
	} else //если 1-байтный адрес
	{
		uint8_t command = CC1200_WRITE | addr_l;
		HAL_SPI_TransmitReceive(&hspi1, &command, &spiByte, 1, HAL_MAX_DELAY);
	}
	HAL_SPI_Transmit(&hspi1, &value, 1, HAL_MAX_DELAY); //значение регистра

	CC_CS_OFF();
}

uint8_t CC1200_rx_read_reg(uint16_t regAddr) {

	CC_CS_ON();

	uint8_t addr_h = (uint8_t) (regAddr >> 8);
	uint8_t addr_l = (uint8_t) (regAddr & 0x00FF);

	if (addr_h) //если 2-байтный адрес
	{
		uint8_t command = CC1200_READ | addr_h;
		HAL_SPI_TransmitReceive(&hspi1, &command, &spiByte, 1, HAL_MAX_DELAY);
		HAL_SPI_TransmitReceive(&hspi1, &addr_l, &spiByte, 1, HAL_MAX_DELAY);
	} else //если 1-байтный адрес
	{
		uint8_t command = CC1200_WRITE | addr_l;
		HAL_SPI_TransmitReceive(&hspi1, &command, &spiByte, 1, HAL_MAX_DELAY);
	}
	HAL_SPI_TransmitReceive(&hspi1, (uint8_t*) &dummyByte, &spiByte, 1,
	HAL_MAX_DELAY); //send dummy byte for read register

	CC_CS_OFF();

	return spiByte; //значение регистра
}

void CC1200_rx_send_command(uint8_t value) {

	CC_CS_ON();

	HAL_SPI_TransmitReceive(&hspi1, &value, &spiStatusByte, 1, HAL_MAX_DELAY); //команда //устанавливает StatusByte

	CC_CS_OFF();
}

uint8_t get_cc_state() {
	CC1200_rx_send_command(CC1200_SNOP);
	return (spiStatusByte & CC1200_STATUS_BYTE_STATE_MASK)
			>> CC1200_STATUS_BYTE_STATE_SHIFT;
}

void CC1200_tx_init(void) {
	CC_CS_OFF();
	HAL_GPIO_WritePin(RES_CC_GPIO_Port, RES_CC_Pin, 0);
	HAL_Delay(100);
	HAL_GPIO_WritePin(RES_CC_GPIO_Port, RES_CC_Pin, 1);
	CC_CS_ON();
	HAL_Delay(50);
	for (int i = 0; i < sizeof(txSettings) / sizeof(txSettings[0]); i++) {
		CC1200_rx_write_reg(txSettings[i].regAddr, txSettings[i].value);
	}
	CC1200_rx_send_command(CC1200_SIDLE);
	CC1200_rx_send_command(CC1200_SFTX);
	CC_CS_OFF();
}

void CC1200_send_packet(const uint8_t *data, uint8_t len) {
	CC_CS_ON();
	uint8_t burstCmd = CC1200_WRITE | CC1200_BURST | 0x3F;
	HAL_SPI_Transmit(&hspi1, &burstCmd, 1, HAL_MAX_DELAY);
	//HAL_SPI_Transmit(&hspi1, &len, 1, HAL_MAX_DELAY);
	// НЕ отправляем len отдельно! data уже содержит полный пакет
	HAL_SPI_Transmit(&hspi1, (uint8_t*) data, len, HAL_MAX_DELAY);
	CC_CS_OFF();
	CC1200_rx_send_command(CC1200_STX);
	uint32_t timeout = 5000;
	while (timeout--) {
		if (get_cc_state() == 0b000)
			break;
		HAL_Delay(1);
	}
	if (timeout == 0)
		CC1200_rx_send_command(CC1200_SIDLE);
}

void CC1200_rx_init() {

	CC_CS_OFF();
	HAL_GPIO_WritePin(RES_CC_GPIO_Port, RES_CC_Pin, 0);
	HAL_Delay(100);
	HAL_GPIO_WritePin(RES_CC_GPIO_Port, RES_CC_Pin, 1);

	CC_CS_ON(); //включение приёма по SPI
	HAL_Delay(50);

	for (int i = 0; i < sizeof(preferredSettings) / sizeof(registerSetting_t);
			++i) {
		CC1200_rx_write_reg(preferredSettings[i].regAddr,
				preferredSettings[i].value);
	}

	CC1200_rx_send_command(CC1200_SIDLE);
	CC1200_rx_send_command(CC1200_SFRX);
	CC1200_rx_send_command(CC1200_SRX);
	HAL_Delay(50);
}

uint16_t crc16_modbus(const uint8_t *data, uint8_t len) {
	uint16_t crc = 0xFFFF;
	for (uint8_t i = 0; i < len; i++) {
		crc ^= data[i];
		for (uint8_t j = 0; j < 8; j++) {
			if (crc & 0x0001)
				crc = (crc >> 1) ^ 0xA001;
			else
				crc >>= 1;
		}
	}
	return crc;
}

uint8_t diag_mk_len(uint8_t n) {
	// n – количество байт в payload (без заголовка длины)
	if (n % 4 || n > 64)
		return 0;
	n >>= 2;
	n--;
	return ((~n & 15) << 4) | n;
}

uint16_t diag_build_packet(uint8_t *buf, uint16_t addr, uint8_t req,
		uint8_t opt, const uint8_t *data, uint8_t data_len) {
	uint8_t payload_len = 4 + data_len; // addr(2) + req(1) + opt(1) + data
	uint8_t enc_len = diag_mk_len(payload_len);
	if (!enc_len)
		return 0;

	buf[0] = enc_len;
	buf[1] = addr >> 8;
	buf[2] = addr & 0xFF;
	buf[3] = req;
	buf[4] = opt;
	if (data_len && data) {
		memcpy(&buf[5], data, data_len);
	}

	uint16_t crc = crc16_modbus(buf, payload_len + 1);
	uint8_t *p = buf + payload_len + 1;
	*p++ = crc & 0xFF;
	*p++ = crc >> 8;

	*p++ = 0xAA;
	*p++ = 0xAA;
	*p++ = 0xAA;

	uint8_t total_len = payload_len + 1 + 2 + 3; // enc_len + addr+req+opt+data + crc + 3*AA

	for (uint8_t i = 0; i < total_len; i++) {
		buf[i] ^= scrambler_tbl[i % sizeof(scrambler_tbl)];
	}
	return total_len;
}

uint16_t pack_amp_settings(struct amp_settings *a) {
	return ((a->gain & 0x07) << 13) | ((a->bias1 & 0x03) << 11)
			| ((a->bias2 & 0x03) << 9) | ((a->vgain & 0x01) << 8)
			| ((a->vbias & 0x07) << 5) | ((a->preamble & 0x03) << 3)
			| ((a->video_ena & 0x01) << 2) | ((a->diag_ena & 0x01) << 1);
}

int diag_descramble(uint8_t *data, uint8_t len) {
	if (len < 3)
		return 0;
	// Скремблированы все байты, кроме последних трёх (0xAA)
	for (uint8_t i = 0; i < len - 3; i++) {
		data[i] ^= scrambler_tbl[i % sizeof(scrambler_tbl)];
	}
	// Проверка CRC (CRC считается по data[0] до len-5, т.к. последние 5 байт: CRC(2) + AA(3))
	if (len < 5)
		return 0;
	uint16_t crc_received = (data[len - 5] << 8) | data[len - 6];
	uint16_t crc_calc = crc16_modbus(data, len - 5);
	//if (crc_received != crc_calc)
	//	return 0;
	// Проверка, что последние три байта = 0xAA
	//if (data[len - 3] != 0xAA || data[len - 2] != 0xAA || data[len - 1] != 0xAA)
	//	return 0;
	return 1;
}

uint16_t diag_build_reply(uint8_t *buf, uint16_t addr, uint8_t req,
		uint8_t rssi, uint8_t adcvid, uint8_t adcul, uint8_t adcdl,
		uint8_t adcvin, uint8_t lsbs, uint16_t amp) {
	// Размер полезной нагрузки без sz = 2+1+1+1+1+1+1+2+1 = 11 байт?
	// По документации sz = 12 (включая само поле sz). Значит payload_len = 11.
	uint8_t payload_len = 12; // addr(2)+rssi(1)+adcvid(1)+adcul(1)+adcdl(1)+adcvin(1)+lsbs(1)+amp(2)+pad(1)
	uint8_t enc_len = diag_mk_len(payload_len);
	if (!enc_len)
		return 0;

	buf[0] = enc_len;
	buf[1] = addr >> 8;
	buf[2] = addr & 0xFF;
	buf[3] = req;
	buf[4] = rssi;
	buf[5] = adcvid;
	buf[6] = adcul;
	buf[7] = adcdl;
	buf[8] = adcvin;
	buf[9] = lsbs;
	buf[10] = amp >> 8;
	buf[11] = amp & 0xFF;
	buf[12] = 0; // pad

	uint16_t crc = crc16_modbus(buf, payload_len + 1);
	uint8_t *p = buf + payload_len + 1;
	*p++ = crc & 0xFF;
	*p++ = crc >> 8;
	*p++ = 0xAA;
	*p++ = 0xAA;
	*p++ = 0xAA;

	uint8_t total_len = payload_len + 1 + 2 + 3; // enc_len + payload + crc + AA*3
	for (uint8_t i = 0; i < total_len - 3; i++) {
		buf[i] ^= scrambler_tbl[i % sizeof(scrambler_tbl)];
	}
	return total_len;
}

void send_telemetry_reply(uint16_t addr, uint8_t req) {
	//get rssi and ect
	uint8_t rssi = 1;
	uint8_t adcvid = 2, adcul = 3, adcdl = 4, adcvin = 5, lsbs = 6;
	uint16_t amp = pack_amp_settings(&amp_settings1); // актуальные настройки усилителя
	uint8_t buf[128];
	uint16_t addr1 = 0x1594;
	uint16_t len = diag_build_reply(buf, addr1, req, rssi, adcvid, adcul, adcdl,
			adcvin, lsbs, amp);
	if (len)
		CC1200_send_packet(buf, len);
}

void CC1200_rx_read_fifo_burst(uint8_t *buffer, uint8_t len) {
	CC_CS_ON();
	uint8_t cmd = CC1200_READ | CC1200_BURST | 0x3F; // 0xFF
	HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
	for (uint8_t i = 0; i < len; i++) {
		HAL_SPI_Receive(&hspi1, &buffer[i], 1, HAL_MAX_DELAY);
	}
	CC_CS_OFF();
}

void unpack_amp_settings(uint16_t amp_word, struct amp_settings *a) {
	a->gain = (amp_word >> 13) & 0x07;
	a->bias1 = (amp_word >> 11) & 0x03;
	a->bias2 = (amp_word >> 9) & 0x03;
	a->vgain = (amp_word >> 8) & 0x01;
	a->vbias = (amp_word >> 5) & 0x07;
	a->preamble = (amp_word >> 3) & 0x03;
	a->video_ena = (amp_word >> 2) & 0x01;
	a->diag_ena = (amp_word >> 1) & 0x01;
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
	HMC_SetAttenuation2(15.5f, 0b10111100);
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

	while (1) {
		//это временный вариант - мигает светодиодом, если есть байты в буфере FIFO
		//HAL_Delay(1);
		const int toggle_led_freq_divider = 10;
		static int toggle_led_counter = 0;

		uint8_t cc_state = get_cc_state();
		switch (cc_state) {
		case 0b001: //rx mode
			//int num_of_bytes = CC1200_rx_read_reg ( CC1200_NUM_RXBYTES );
			uint8_t num_bytes = CC1200_rx_read_reg(CC1200_NUM_RXBYTES);
			if (num_bytes > 0) {
				uint8_t rx_packet[128];
 				CC1200_rx_read_fifo_burst(rx_packet, num_bytes);
				uint8_t start = 0;
				    while (start < num_bytes && rx_packet[start] == 0xAA) start++;
				    if (start >= num_bytes) {
				        CC1200_rx_send_command(CC1200_SFRX);
				        break;
				    }
				    uint8_t packet_len = num_bytes - start;
				        memmove(rx_packet, &rx_packet[start], packet_len);
				        num_bytes = packet_len;

				if (!diag_descramble(rx_packet, num_bytes)) {
					// пакет невалиден – игнорируем
					uint16_t src_addr = (rx_packet[1] << 8) | rx_packet[2];
					//CC1200_tx_init();
					CC1200_tx_init();
					send_telemetry_reply(src_addr, 2);
					CC1200_rx_send_command(CC1200_SFRX);
					CC1200_rx_init();
					break;
				}
				// теперь rx_packet[0] – кодированная длина, [1]..[2] – addr, [3] – req, [4] – opt, [5].. – data
				uint16_t src_addr = (rx_packet[1] << 8) | rx_packet[2];
				uint8_t req = rx_packet[3];
				uint8_t opt = rx_packet[4];
				// data_len = (enc_len & 0x0F) * 4? Но проще вычислить из общего размера
				// payload_len = (enc_len & 0x0F) * 4 + 4? Лучше по размеру полученного пакета

				uint8_t enc_len = rx_packet[0];
				uint8_t payload_len = ((enc_len & 0x0F) + 1) * 4; // обратное преобразование diag_mk_len
				uint8_t data_len = payload_len - 4;
				uint8_t *data_ptr = &rx_packet[5];

				if (req == 0) { // PCK_REQ_GENERAL
					send_telemetry_reply(src_addr, req);
				} else if (req == 2) { // PCK_SET_PARAMS
					if (data_len >= 2) {
						uint16_t amp_word = (data_ptr[1] << 8) | data_ptr[0]; // little-endian: младший байт первым
						unpack_amp_settings(amp_word, &amp_settings1); // сохраняем в глобальную структуру

						// Применяем настройки к усилителю.
						// Например, используем поле gain для выбора аттенюации (map 0..7 -> индекс в attValue)
						// Можно отобразить gain * 4 (макс 28, но у нас таблица до 31)
						uint8_t idx = amp_settings1.gain * 4;
						if (idx >= sizeof(attValue) / sizeof(attValue[0]))
							idx = sizeof(attValue) / sizeof(attValue[0]) - 1;
						// Устанавливаем аттенюацию для обоих каналов одинаково (или по-другому)
						HMC_SetAttenuation(15.5f, attValue[idx].registerValue);
						HMC_SetAttenuation2(15.5f, attValue[idx].registerValue);
						// Здесь также можно применить другие поля (bias, vgain и т.д.) к другим узлам.
					}
					send_telemetry_reply(src_addr, req);
				} else if (req == 3) { // PCK_START_TEST
					if (data_len >= 1) {
						uint8_t timeout = data_ptr[0]; // время теста
						// Запустить тестовый режим (непрерывная передача)
					}
					send_telemetry_reply(src_addr, req);
				} else if (req == 12) {
					if (data_len >= 2) {
						uint8_t code_down = data_ptr[0]; // код для канала 1 (вниз)
						uint8_t code_up = data_ptr[1]; // код для канала 2 (вверх)
						HMC_SetAttenuation(0, code_down);
						HMC_SetAttenuation2(0, code_up);
					} else if (data_len == 1) {
						// если передан один байт – применить к обоим
						uint8_t code = data_ptr[0];
						HMC_SetAttenuation(0, code);
						HMC_SetAttenuation2(0, code);
					}
					send_telemetry_reply(src_addr, req);
				}
				CC1200_rx_send_command(CC1200_SFRX);
			} else {
				//HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_RESET);
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

			if (HAL_GPIO_ReadPin(RSII_Q1_EX_GPIO_Port, RSII_Q1_EX_Pin)
					== GPIO_PIN_SET) {

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
						HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin,
								GPIO_PIN_RESET);
					}
				}
			}

			/* -------- CHANNEL 2 -------- */

			if (HAL_GPIO_ReadPin(RSII_Q2_EX_GPIO_Port, RSII_Q2_EX_Pin)
					== GPIO_PIN_SET) {

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
						HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin,
								GPIO_PIN_RESET);
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

				break;
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
