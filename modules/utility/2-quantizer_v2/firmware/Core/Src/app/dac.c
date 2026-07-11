/*
******************************************************************************
* @file           : dac.c
* @author         : Lennart Querter
* @brief          : MCP4922 dual 12-bit DAC driver (SPI1, software ~CS PB4)
******************************************************************************
*/

#include "app/dac.h"

#include "main.h"

// command bits: bit15 = channel, bit14 = BUF (buffered VREF),
// bit13 = ~GA (1 = gain x1), bit12 = ~SHDN (1 = active)
#define DAC_CMD_CHANNEL_B 0x8000u
#define DAC_CMD_BUF       0x4000u
#define DAC_CMD_GAIN_1X   0x2000u
#define DAC_CMD_ACTIVE    0x1000u

static SPI_HandleTypeDef* dac_spi;
static uint16_t dac_code[2];

void DAC_init(SPI_HandleTypeDef* hspi)
{
    dac_spi = hspi;
    dac_code[DAC_CHANNEL_A] = 0;
    dac_code[DAC_CHANNEL_B] = 0;

    HAL_GPIO_WritePin(DAC_CS_GPIO_Port, DAC_CS_Pin, GPIO_PIN_SET);
}

HAL_StatusTypeDef DAC_write_code(uint8_t channel, uint16_t code)
{
    if (code > DAC_CODE_MAX)
    {
        code = DAC_CODE_MAX;
    }

    uint16_t frame = DAC_CMD_BUF | DAC_CMD_GAIN_1X | DAC_CMD_ACTIVE | code;
    if (channel == DAC_CHANNEL_B)
    {
        frame |= DAC_CMD_CHANNEL_B;
    }

    uint8_t bytes[2] = {(uint8_t)(frame >> 8), (uint8_t)(frame & 0xFF)};

    HAL_GPIO_WritePin(DAC_CS_GPIO_Port, DAC_CS_Pin, GPIO_PIN_RESET);
    HAL_StatusTypeDef status = HAL_SPI_Transmit(dac_spi, bytes, 2, 10);
    // rising edge executes the write; ~LDAC is grounded so the output updates now
    HAL_GPIO_WritePin(DAC_CS_GPIO_Port, DAC_CS_Pin, GPIO_PIN_SET);

    if (status == HAL_OK)
    {
        dac_code[channel & 1] = code;
    }
    return status;
}

uint16_t DAC_get_code(uint8_t channel)
{
    return dac_code[channel & 1];
}
