/*
******************************************************************************
* @file           : dac.h
* @author         : Lennart Querter
* @brief          : Header for dac.c file.
*                   MCP4922 dual 12-bit DAC driver over SPI1 with software
*                   ~CS on PB4 (TO_FIX #1: ~LDAC is tied to GND, the write
*                   latches on the ~CS rising edge).
* @Datasheet.     : https://ww1.microchip.com/downloads/en/DeviceDoc/22250A.pdf
******************************************************************************
*/

#ifndef __DAC_H
#define __DAC_H

#include "stm32f4xx_hal.h"

#define DAC_CHANNEL_A 0
#define DAC_CHANNEL_B 1

#define DAC_CODE_MAX 4095

void DAC_init(SPI_HandleTypeDef* hspi);

// write a raw 12-bit code to channel A/B; the output updates on ~CS rising edge
HAL_StatusTypeDef DAC_write_code(uint8_t channel, uint16_t code);

// last code written per channel (for CLI/status)
uint16_t DAC_get_code(uint8_t channel);

#endif
