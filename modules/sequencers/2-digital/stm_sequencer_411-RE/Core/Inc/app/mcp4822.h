/*
******************************************************************************
* @file           : mcp4822.h
* @author         : Lennart Querter
* @brief          : Header for mcp4822.c file.
*                   This file contains the defines for the mcp4822 driver
******************************************************************************
*/

#ifndef __MCP4822_H
#define __MCP4822_H

#include "stm32f4xx_hal.h"

/** Bit Manipulation Macros */
#define MCP4822_SHIFT_4    			            4
#define MCP4822_SHIFT_5    			            5
#define MCP4822_SHIFT_7    			            7
#define MCP4822_SHIFT_8    			            8

#define MCP4822_FIRST_BIT_MASK  			    0x01
#define MCP4822_LOW_HALF_BYTE_MASK			    0x000F
#define MCP4822_FIRST_BYTE_MASK 			    0x00FF

#define MCP4822_RES      	   		   12
#define MCP4822_DAC_MAX				   4095
#define MCP4822_VREF				   2.048f
#define MCP4822_SPI_TIMEOUT			   1 // (in ms)


typedef enum
{
    MCP4822_OK = 0,
    MCP4822_ERROR_INVALID_ARG = -1,
    MCP4822_ERROR_SPI = -2
} MCP4822_STATUS;

typedef enum
{
    MCP4822_CHANNEL_A = 0,
    MCP4822_CHANNEL_B = 1
} MCP4822_DAC_SELECT;

typedef enum
{
    MCP4822_GAIN_2X = 0,
    MCP4822_GAIN_1X = 1
} MCP4822_OUTPUT_GAIN;

typedef enum
{
    MCP4822_SHUTDOWN_MODE = 0,
    MCP4822_ACTIVE_MODE = 1
} MCP4822_OUTPUT_MODE;

typedef struct
{
    MCP4822_OUTPUT_GAIN gain;
    MCP4822_OUTPUT_MODE output_mode;
} MCP4822_channel_config;

typedef struct
{
    MCP4822_channel_config chan_A_config;
    MCP4822_channel_config chan_B_config;
} MCP4822_channels_config;

typedef struct
{
    MCP4822_channels_config chan_configs;
    GPIO_TypeDef* port;
    uint16_t cs_pin;
    SPI_HandleTypeDef* spi_handler;
} MCP4822_config;


void mcp4822_init(MCP4822_config* config, GPIO_TypeDef* port, uint16_t pin, SPI_HandleTypeDef* SPIHandler);

MCP4822_STATUS mcp4822_write_value(MCP4822_config* config, uint16_t value, MCP4822_DAC_SELECT dac_channel);

#endif // __MCP4822_H
