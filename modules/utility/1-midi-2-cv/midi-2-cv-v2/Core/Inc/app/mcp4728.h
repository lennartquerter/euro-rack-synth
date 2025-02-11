/*
******************************************************************************
* @file           : mcp4728.h
* @author         : Lennart Querter
* @brief          : Header for mcp4728.c file.
*                   This file contains the defines for the mcp4728 driver
******************************************************************************
*/

#ifndef __MCP4728_H
#define __MCP4728_H

#include "stm32f4xx_hal.h"

HAL_StatusTypeDef MCP4728_Init(I2C_HandleTypeDef *I2CHandler);
HAL_StatusTypeDef I2C_Reset(I2C_HandleTypeDef I2CHandler);
HAL_StatusTypeDef I2C_Check_Error(I2C_HandleTypeDef I2CHandler);

HAL_StatusTypeDef MCP4728_FastWrite(I2C_HandleTypeDef *I2CHandler, uint16_t ch_a, uint16_t ch_b, uint16_t ch_c, uint16_t ch_d);
HAL_StatusTypeDef MCP4728_SingleWrite(I2C_HandleTypeDef *I2CHandler, uint8_t channel, uint16_t value, uint8_t vref, uint8_t gain, uint8_t pd);
float MCP4728_SetVoltage(I2C_HandleTypeDef *I2CHandler, uint8_t channel, float voltage);



#define MCP4728_BASE_ADDR               0x60<<1

#define MCP4728_CHANNEL_A               0x0
#define MCP4728_CHANNEL_B               0x1
#define MCP4728_CHANNEL_C               0x2
#define MCP4728_CHANNEL_D               0x3
#define MCP4728_NO_CHANNEL              0x4

#define MCP4728_CMD_DACWRITE_SEQ        0x50
#define MCP4728_CMD_DACWRITE_SINGLE     0x58

#define MCP4728_GENERAL_RESET           0x06
#define MCP4728_GENERAL_WAKEUP          0x09
#define MCP4728_GENERAL_SWUPDATE        0x08
#define MCP4728_GENERAL_READ_ADDR       0x0C

#endif