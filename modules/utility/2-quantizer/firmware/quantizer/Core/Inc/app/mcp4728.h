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

#define MCP4728_CHANNEL_A               0
#define MCP4728_CHANNEL_B               1
#define MCP4728_CHANNEL_C               2
#define MCP4728_CHANNEL_D               3
#define MCP4728_NO_CHANNEL              0x4

#define MCP4728_GENERAL_RESET           0x06
#define MCP4728_GENERAL_WAKEUP          0x09
#define MCP4728_GENERAL_SWUPDATE        0x08

HAL_StatusTypeDef MCP4728_Init(I2C_HandleTypeDef* I2CHandler);
HAL_StatusTypeDef I2C_Reset(I2C_HandleTypeDef I2CHandler);
HAL_StatusTypeDef I2C_Check_Error(I2C_HandleTypeDef I2CHandler);

HAL_StatusTypeDef MCP4728_FastWrite(I2C_HandleTypeDef* I2CHandler, uint16_t ch_a, uint16_t ch_b, uint16_t ch_c,
                                    uint16_t ch_d);
HAL_StatusTypeDef MCP4728_SingleWrite(I2C_HandleTypeDef* I2CHandler, uint8_t channel, uint16_t value, uint8_t vref,
                                      uint8_t gain, uint8_t pd);


#endif
