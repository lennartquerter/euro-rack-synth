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

void MCP4728_Init(I2C_HandleTypeDef *I2CHandler);
void MCP4728_Write_Voltage(I2C_HandleTypeDef *I2CHandler, uint16_t channel, uint16_t output);

#define MCP4728_BASE_ADDR			0x60<<1

#define MCP4728_CHANNEL_A			0x0
#define MCP4728_CHANNEL_B			0x1
#define MCP4728_CHANNEL_C			0x2
#define MCP4728_CHANNEL_D			0x3

#define MCP4728_CMD_DACWRITE_SEQ	0x50
#define MCP4728_CMD_DACWRITE_SINGLE	0x58

#define MCP4728_GENERAL_RESET		0x06
#define MCP4728_GENERAL_WAKEUP		0x09
#define MCP4728_GENERAL_SWUPDATE	0x08
#define MCP4728_GENERAL_READ_ADDR	0x0C

#endif