/*
******************************************************************************
* @file           : display.h
* @author         : Lennart Querter
* @brief          : Header for display.c file.
*                   This file contains the functionality for the display
*                   8 status leds per channel, driven by HC595A & HC595B
*                   4 sequence_counter, driven by HC595C (up to 64 steps!)
*                   3 control mode leds, driven by HC595C
*                   1 additional LED, driven by HC595C
******************************************************************************
*/

#ifndef __DISPLAY_H
#define __DISPLAY_H

#include "stm32f4xx_hal.h"

typedef struct {
    // Chip Select PIN (mode LEDS & SEQ_COUNTER)
    GPIO_TypeDef* mode_port;
    uint16_t mode_pin;

    // Chip Select PIN (A)
    GPIO_TypeDef* channel_a_port;
    uint16_t channel_a_pin;

    // Chip Select PIN (B)
    GPIO_TypeDef* channel_b_port;
    uint16_t channel_b_pin;

    // Will handle in a BUS configuration for all chips
    SPI_HandleTypeDef* spi_handler;
} DISPLAY_config;

#endif // __DISPLAY_H
