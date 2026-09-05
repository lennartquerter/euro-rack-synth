/*
******************************************************************************
* @file           : mcp4728.h
* @author         : Lennart Querter
* @brief          : Header for mcp4728.c file.
*                   This file contains the defines for the mcp4728 driver
* @Datasheet.     : https://ww1.microchip.com/downloads/aemDocuments/documents/OTH/ProductDocuments/DataSheets/22187E.pdf
******************************************************************************
*/

#ifndef __MCP4728_H
#define __MCP4728_H

#include "stm32f4xx_hal.h"

#define MCP4728_ADDR (0x60 << 1)  // Shift for HAL's 8-bit address format

// Command bytes
#define MCP4728_CMD_SINGLE_WRITE  0x58
#define MCP4728_CMD_SEQ_WRITE     0x50  // writes the DAC input registers AND their EEPROM
#define MCP4728_CMD_WRITE_VREF    0x80  // volatile: 0x80 | (VrefA<<3 | VrefB<<2 | VrefC<<1 | VrefD)
#define MCP4728_CMD_WRITE_GAIN    0xC0  // volatile: 0xC0 | (GxA<<3  | GxB<<2  | GxC<<1  | GxD)
#define MCP4728_CMD_WRITE_PD      0xA0  // volatile: two bytes, 2 power-down bits per channel

// Config bits as they appear in the high nibble of a channel's config/data byte
#define MCP4728_VREF_INTERNAL     0x80  // 2.048V internal reference (0x00 would select VDD)
#define MCP4728_GAIN_X1           0x00  // 0x10 would select x2
#define MCP4728_PD_NORMAL         0x00

// The configuration this module depends on: internal 2.048V reference at gain 1, powered up.
// Fast Write carries no Vref/gain bits, so this must already be in the input registers.
#define MCP4728_CONFIG_BYTE       (MCP4728_VREF_INTERNAL | MCP4728_PD_NORMAL | MCP4728_GAIN_X1)

#define MCP4728_READ_LENGTH       24    // 6 bytes per channel: 3 DAC register, 3 EEPROM
#define MCP4728_EEPROM_WRITE_MS   60    // datasheet: 50ms typical per EEPROM write cycle

#define MCP4728_CHANNEL_A               0
#define MCP4728_CHANNEL_B               1
#define MCP4728_CHANNEL_C               2
#define MCP4728_CHANNEL_D               3
#define MCP4728_NO_CHANNEL              0x4

#define MCP4728_GENERAL_RESET           0x06
#define MCP4728_GENERAL_WAKEUP          0x09
#define MCP4728_GENERAL_SWUPDATE        0x08

HAL_StatusTypeDef MCP4728_Init(I2C_HandleTypeDef* I2CHandler);
HAL_StatusTypeDef MCP4728_SetConfig(I2C_HandleTypeDef* I2CHandler);
HAL_StatusTypeDef MCP4728_StoreConfigEEPROM(I2C_HandleTypeDef* I2CHandler);
HAL_StatusTypeDef I2C_Reset(I2C_HandleTypeDef I2CHandler);
HAL_StatusTypeDef I2C_Check_Error(I2C_HandleTypeDef I2CHandler);

HAL_StatusTypeDef MCP4728_FastWrite(I2C_HandleTypeDef* I2CHandler, uint16_t ch_a, uint16_t ch_b, uint16_t ch_c,
                                    uint16_t ch_d);
HAL_StatusTypeDef MCP4728_SingleWrite(I2C_HandleTypeDef* I2CHandler, uint8_t channel, uint16_t value, uint8_t vref,
                                      uint8_t gain, uint8_t pd);


#endif
