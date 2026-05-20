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
  {CC1200_IOCFG2,            0x06},

  { CC1200_SYNC2,          0x6C },    // Sync Word Configuration [23:16]
  { CC1200_SYNC1,          0x94 },    // Sync Word Configuration [15:8]
  { CC1200_SYNC0,          0xA6 },    // Sync Word Configuration [7:0]
  { CC1200_SYNC_CFG1,      0x88 },    // Sync Word Detection Configuration Reg. 1
  { CC1200_SYNC_CFG0,      0x23 },    // Sync Word Detection Configuration Reg. 0

  {CC1200_DEVIATION_M,       0xD1},
  {CC1200_MODCFG_DEV_E,      0x08},
  {CC1200_DCFILT_CFG,        0x5D},

  { CC1200_PREAMBLE_CFG1,  0x08 },    // Preamble Length Configuration Reg. 1
  { CC1200_PREAMBLE_CFG0,  0x8A },    // Preamble Detection Configuration Reg. 0

  {CC1200_IQIC,              0xCB},
  {CC1200_CHAN_BW,           0x95},
  {CC1200_MDMCFG1,           0x40},
  {CC1200_MDMCFG0,           0x05},
  {CC1200_SYMBOL_RATE2,      0x60},
  {CC1200_SYMBOL_RATE1,      0x62},
  {CC1200_SYMBOL_RATE0,      0x4E},
  {CC1200_AGC_REF,           0x31},
  {CC1200_AGC_CS_THR,        0xEC},
  {CC1200_AGC_CFG1,          0x51},
  {CC1200_AGC_CFG0,          0x87},
  {CC1200_FIFO_CFG,          0x00},
  {CC1200_FS_CFG,            0x1B},
  {CC1200_PKT_CFG2,          0x00},
  {CC1200_PKT_CFG0,          0x20},
  {CC1200_PKT_LEN,           0xFF},
  {CC1200_IF_MIX_CFG,        0x1C},
  {CC1200_FREQOFF_CFG,       0x22},
  {CC1200_MDMCFG2,           0x0C},
  {CC1200_FREQ2,             0x5B},
  {CC1200_FREQ1,             0x33},
  {CC1200_FREQ0,             0x33},
  {CC1200_IF_ADC1,           0xEE},
  {CC1200_IF_ADC0,           0x10},
  {CC1200_FS_DIG1,           0x07},
  {CC1200_FS_DIG0,           0xAF},
  {CC1200_FS_CAL1,           0x40},
  {CC1200_FS_CAL0,           0x0E},
  {CC1200_FS_DIVTWO,         0x03},
  {CC1200_FS_DSM0,           0x33},
  {CC1200_FS_DVC0,           0x17},
  {CC1200_FS_PFD,            0x00},
  {CC1200_FS_PRE,            0x6E},
  {CC1200_FS_REG_DIV_CML,    0x1C},
  {CC1200_FS_SPARE,          0xAC},
  {CC1200_FS_VCO0,           0xB5},
  {CC1200_XOSC5,             0x0E},
  {CC1200_XOSC1,             0x03},
};

static const registerSetting_t txSettings[] = {
    { CC1200_IOCFG3,         0x0C },
    { CC1200_IOCFG2,         0x0B },
    { CC1200_IOCFG0,         0x06 },
    { CC1200_SYNC2,          0x6C },
    { CC1200_SYNC1,          0x94 },
    { CC1200_SYNC0,          0xA6 },
    { CC1200_SYNC_CFG1,      0x88 },
    { CC1200_SYNC_CFG0,      0x23 },
    { CC1200_DEVIATION_M,    0xD1 },
    { CC1200_MODCFG_DEV_E,   0x08 },
    { CC1200_DCFILT_CFG,     0x4B },
    { CC1200_PREAMBLE_CFG1,  0x08 },
    { CC1200_PREAMBLE_CFG0,  0x8A },
    { CC1200_IQIC,           0xD8 },
    { CC1200_CHAN_BW,        0x95 },
    { CC1200_MDMCFG1,        0x42 },
    { CC1200_MDMCFG0,        0x05 },
    { CC1200_SYMBOL_RATE2,   0x60 },
    { CC1200_SYMBOL_RATE1,   0x62 },
    { CC1200_SYMBOL_RATE0,   0x4E },
    { CC1200_AGC_REF,        0x1F },
    { CC1200_AGC_CS_THR,     0xF6 },
    { CC1200_AGC_CFG1,       0x12 },
    { CC1200_AGC_CFG0,       0x80 },
    { CC1200_FIFO_CFG,       0x60 },
    { CC1200_FS_CFG,         0x1A },
    { CC1200_PKT_CFG2,       0x00 },   // переменная длина
    { CC1200_PKT_CFG1,       0x00 },
    { CC1200_PKT_CFG0,       0x00 },
    { CC1200_RFEND_CFG1,     0x3F },
    { CC1200_PKT_LEN,        0xFF },
    { CC1200_PA_CFG1,        0x6A },   // мощность
    { CC1200_PA_CFG0,        0x6A },
    // Расширенные
    { CC1200_IF_MIX_CFG,     0x1C },
    { CC1200_TOC_CFG,        0x03 },
    { CC1200_MDMCFG2,        0x02 },
    { CC1200_FREQ2,          0x58 },   // 177 МГц
    { CC1200_FREQ1,          0x80 },
    { CC1200_FREQ0,          0x00 },
    { CC1200_IF_ADC1,        0xEE },
    { CC1200_IF_ADC0,        0x10 },
    { CC1200_FS_DIG1,        0x04 },
    { CC1200_FS_DIG0,        0x50 },
    { CC1200_FS_CAL1,        0x40 },
    { CC1200_FS_CAL0,        0x0E },
    { CC1200_FS_DIVTWO,      0x03 },
    { CC1200_FS_DSM0,        0x33 },
    { CC1200_FS_DVC1,        0xF7 },
    { CC1200_FS_DVC0,        0x0F },
    { CC1200_FS_PFD,         0x00 },
    { CC1200_FS_PRE,         0x6E },
    { CC1200_FS_REG_DIV_CML, 0x1C },
    { CC1200_FS_SPARE,       0xAC },
    { CC1200_FS_VCO0,        0xB5 },
    { CC1200_IFAMP,          0x09 },
    { CC1200_XOSC5,          0x0E },
    { CC1200_XOSC1,          0x03 },
};
