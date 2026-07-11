/*
******************************************************************************
* @file           : leds.h
* @author         : Lennart Querter
* @brief          : Header for leds.c file.
*                   20 LEDs on 3x chained 74HC595 via SPI3, latched with
*                   RCLK on PC11. Frame buffer with per-LED modes and
*                   one-shot pulses, pushed at the 1 kHz tick when dirty.
******************************************************************************
*/

#ifndef __LEDS_H
#define __LEDS_H

#include "stm32f4xx_hal.h"

// logical LED indices
#define LED_NOTE_BASE 0 // 0..11 = note button LEDs C..B
#define LED_BTN_A 12
#define LED_BTN_B 13
#define LED_BTN_C 14
#define LED_BTN_D 15
#define LED_STATUS_BASE 16 // 16..19 = status LEDs D5..D8
#define LEDS_COUNT 20

// 595 output high lights the LED; flip if bring-up shows otherwise
#define LEDS_ACTIVE_HIGH 1

enum LEDS_mode
{
    LEDS_OFF = 0,
    LEDS_ON,
    LEDS_BLINK_SLOW, // ~2 Hz
    LEDS_BLINK_FAST, // ~8 Hz
};

void LEDS_init(SPI_HandleTypeDef* hspi);
void LEDS_set(uint8_t index, enum LEDS_mode mode);
enum LEDS_mode LEDS_get(uint8_t index);

// force the LED on for a number of milliseconds on top of its mode
void LEDS_pulse(uint8_t index, uint16_t ms);

// blink phases + transmit-on-change, call at 1 kHz
void LEDS_tick(void);

#endif
