/*
******************************************************************************
* @file           : switches.h
* @author         : Lennart Querter
* @brief          : Header for switches.c file.
*                   All panel inputs: 12 note switches via 2x 74HC165 on
*                   SPI2 (PB13 = ~PL load pulse), plus the direct-GPIO
*                   buttons A-D (PC6-PC9) and the encoder switch (PA5).
*                   Debounced at the 1 kHz tick, edges become events.
******************************************************************************
*/

#ifndef __SWITCHES_H
#define __SWITCHES_H

#include "stm32f4xx_hal.h"

// logical switch indices
#define SW_NOTE_BASE 0   // 0..11 = note buttons C..B
#define SW_CHAIN_SPARE 12 // 12..15 = unused 74HC165 inputs
#define SW_BTN_A 16
#define SW_BTN_B 17
#define SW_BTN_C 18
#define SW_BTN_D 19
#define SW_ENC 20
#define SW_COUNT 21

// switches short to GND against pull-ups; flip these if bring-up shows otherwise
#define SWITCHES_CHAIN_ACTIVE_LOW 1
#define SWITCHES_GPIO_ACTIVE_LOW 1

// consecutive 1 kHz samples a level must hold to count as a press/release
#define SWITCHES_DEBOUNCE_TICKS 5

struct SWITCHES_event
{
    uint8_t index;   // SW_* logical index
    uint8_t pressed; // 1 = pressed, 0 = released
};

void SWITCHES_init(SPI_HandleTypeDef* hspi);

// sample + debounce, call at 1 kHz
void SWITCHES_scan(void);

// pop the next debounced edge; returns 1 while events are pending
uint8_t SWITCHES_get_event(struct SWITCHES_event* event);

// debounced state of one logical switch (1 = pressed)
uint8_t SWITCHES_state(uint8_t index);

// last raw (pre-debounce) sample, bit = logical index — for the CLI
uint32_t SWITCHES_raw(void);

#endif
