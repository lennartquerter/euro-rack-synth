//
// Created by Lennart Querter on 02.02.25.
//

#include "app/led_driver.h"

void LED_DRIVER_init(LED_DRIVER_config* config, GPIO_TypeDef* port, uint16_t cs_pin, SPI_HandleTypeDef* spi_handler) {
  config->port = port;
  config->cs_pin = cs_pin;

  config->spi_handler = spi_handler;

  HAL_GPIO_WritePin(config->port, config->cs_pin, GPIO_PIN_SET);  // ENABLE CHIP
}

void LED_DRIVER_write(LED_DRIVER_config* config, uint8_t data)
{
  // Disable output while shifting
  HAL_GPIO_WritePin(config->port, config->cs_pin, GPIO_PIN_SET);  // ENABLE high

  // Send data via SPI
  HAL_SPI_Transmit(config->spi_handler, &data, 1, HAL_MAX_DELAY);

  // Wait for transmission complete
  while(HAL_SPI_GetState(config->spi_handler) != HAL_SPI_STATE_READY)
  {
    // Wait for SPI to be ready
  }

  // Enable output
  HAL_GPIO_WritePin(config->port, config->cs_pin, GPIO_PIN_RESET);  // ENABLE low
}

void LED_DRIVER_set_color(LED_DRIVER_config* config, uint8_t led_number, LED_Color color)
{
  static uint8_t led_state = 0;
  uint8_t shift = led_number * 2;  // Each LED uses 2 bits

  // Clear the old state for this LED
  led_state &= ~(0b11 << shift);
  // Set the new color
  led_state |= (color << shift);

  // Update shift register via SPI
  LED_DRIVER_write(config, led_state);
}

void LED_DRIVER_set_all(LED_DRIVER_config* config, LED_Color color)
{
  uint8_t led_state = 0;

  // Set all LEDs to the same color
  for(int i = 0; i < 4; i++)
  {
    led_state |= (color << (i * 2));
  }

  LED_DRIVER_write(config, led_state);
}

