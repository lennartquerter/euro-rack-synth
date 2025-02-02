/*
******************************************************************************
* @file           : cs4270.h
* @author         : Lennart Querter
* @brief          : Header for cs4270.c file.
*                   This file contains the defines for the cs4270 driver
******************************************************************************
*/

#ifndef __CS4270_H
#define __CS4270_H

#include <stdbool.h>

#include "main.h"

#define CS4270_ADDR             0x48  // I2C address (0x48 when AD0 is grounded)

// Register addresses
#define CS4270_CHIP_ID          0x01
#define CS4270_POWER_CTRL       0x02
#define CS4270_MODE_CTRL        0x03
#define CS4270_ADC_DAC_CTRL     0x04
#define CS4270_TRANSITION       0x05
#define CS4270_MUTE             0x06
#define CS4270_DAC_VOL          0x07
#define CS4270_ADC_VOL          0x08


typedef struct
{
    GPIO_TypeDef* reset_port;
    uint16_t reset_pin;
    I2C_HandleTypeDef* i2c_handler;
} CS4270_config;

void cs4270_init(CS4270_config* config, GPIO_TypeDef* reset_port, uint16_t reset_pin, I2C_HandleTypeDef* I2CHandler);

HAL_StatusTypeDef cs4270_configure(CS4270_config* config);

HAL_StatusTypeDef cs4270_set_volume(CS4270_config* config, uint8_t volume);

HAL_StatusTypeDef cs4270_mute(CS4270_config* config, bool mute);


#endif // __CS4270_H
