//
// Created by Lennart Querter on 02.02.25.
//

#ifndef LED_DRIVER_H
#define LED_DRIVER_H

#include "main.h"

typedef struct
{
    GPIO_TypeDef* port;
    uint16_t cs_pin;
    SPI_HandleTypeDef* spi_handler;
} LED_DRIVER_config;

// LED colors (8 bits total for 4 dual-color LEDs)
typedef enum {
    LED_OFF =     0b00,
    LED_RED =     0b10,
    LED_GREEN =   0b01,
    LED_YELLOW =  0b11
} LED_Color;

// LED positions
#define LED1 0  // Bits 0-1
#define LED2 1  // Bits 2-3
#define LED3 2  // Bits 4-5
#define LED4 3  // Bits 6-7

void LED_DRIVER_init(LED_DRIVER_config* config, GPIO_TypeDef* port, uint16_t cs_pin, SPI_HandleTypeDef* spi_handler);

void LED_DRIVER_set_color(LED_DRIVER_config* config, uint8_t led_number, LED_Color color);

void LED_DRIVER_set_all(LED_DRIVER_config* config, LED_Color color);

#endif //LED_DRIVER_H
