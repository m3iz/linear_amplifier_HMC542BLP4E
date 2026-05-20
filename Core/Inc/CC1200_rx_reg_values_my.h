//Это конфигурация Алексея с заменёнными регистрами настройки синхропосылки и преамбулы. Значения для замены взял из конфигурации Евгения

// Address Config = No address check 
// Bit Rate = 5
// Carrier Frequency = 152.000000
// Deviation = 3.986359
// Device Address = 0 
// Manchester Enable = false 
// Modulation Format = 2-GFSK
// Packet Bit Length = 0 
// Packet Length = 255
// Packet Length Mode = Variable
// RX Filter BW = 19.841270
// Symbol rate = 5
// Whitening = false

#include "CC1200_REGISTERS.h"

typedef struct
{
	uint16_t  regAddr;
	uint8_t   value;
}registerSetting_t;

static const registerSetting_t preferredSettings[]=
{
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

static const registerSetting_t txSettings[] = { { CC1200_IOCFG0,         0x06 },    // GPIO0 IO Pin Configuration
		  { CC1200_IOCFG2,         0x0B },    // GPIO2 IO Pin Configuration
		  { CC1200_IOCFG3,         0x0C },    // GPIO3 IO Pin Configuration
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

		  { CC1200_FS_CFG,         0x1A },    // Frequency Synthesizer Configuration
		  { CC1200_PKT_CFG2,       0x00 },    // Packet Configuration Reg. 2
		  { CC1200_PKT_CFG1,       0x00 },    // Packet Configuration Reg. 1
		  { CC1200_PKT_CFG0,       0x00 },    // Packet Configuration Reg. 0
		  { CC1200_RFEND_CFG1,     0x3F },    // RFEND Configuration Reg. 1
		  { CC1200_PKT_LEN,        0x10 },    // Packet Length Configuration
		  { CC1200_IF_MIX_CFG,     0x1C },    // IF Mix Configuration
		  { CC1200_TOC_CFG,        0x03 },    // Timing Offset Correction Configuration
		  { CC1200_MDMCFG2,        0x02 },    // General Modem Parameter Configuration Reg. 2
		  { CC1200_FREQ2,          0x58 },    // Frequency Configuration [23:16]
		  { CC1200_FREQ1,          0x80 },    // Frequency Configuration [15:8]
		  { CC1200_FREQ0,          0x00 },    // Frequency Configuration [7:0]
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
