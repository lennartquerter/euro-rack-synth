/*
* Datasheet: https://ww1.microchip.com/downloads/aemDocuments/documents/OTH/ProductDocuments/DataSheets/20002249B.pdf
 *
 */

#include "app/mcp4822.h"

static MCP4822_channel_config* get_chan_config(MCP4822_config* handle, MCP4822_DAC_SELECT dac_channel);

void mcp4822_init(MCP4822_config* config, GPIO_TypeDef* port, uint16_t cs_pin, SPI_HandleTypeDef* SPIHandler)
{
    config->port = port;
    config->cs_pin = cs_pin;
    config->spi_handler = SPIHandler;

    config->chan_configs.chan_A_config.gain = MCP4822_GAIN_2X;
    config->chan_configs.chan_A_config.output_mode = MCP4822_ACTIVE_MODE;

    config->chan_configs.chan_B_config.gain = MCP4822_GAIN_2X;
    config->chan_configs.chan_B_config.output_mode = MCP4822_ACTIVE_MODE;
}

MCP4822_STATUS mcp4822_write_value(MCP4822_config* config, uint16_t value, MCP4822_DAC_SELECT dac_channel)
{
    //Limit value to the max input for MCP4822
    if (value > MCP4822_DAC_MAX)
    {
        return MCP4822_ERROR_INVALID_ARG;
    }

    //Receive the correct DAC channel configuration
    MCP4822_channel_config* curr_chan_config = get_chan_config(config, dac_channel);

    uint8_t tx_data[2];

    // DAC_CHANNEL (A: 0/B: 1)
    uint8_t firstByte = (uint8_t)(dac_channel & MCP4822_FIRST_BIT_MASK) << MCP4822_SHIFT_7 |
        // GAIN setting (2X: 0/1X: 1)
        (uint8_t)(curr_chan_config->gain & MCP4822_FIRST_BIT_MASK) << MCP4822_SHIFT_5 |
        // MODE (ACTIVE: 1/SHUTDOWN: 0)
        (uint8_t)(curr_chan_config->output_mode & MCP4822_FIRST_BIT_MASK) << MCP4822_SHIFT_4 |
        // MSB
        (uint8_t)(value >> MCP4822_SHIFT_8 & MCP4822_LOW_HALF_BYTE_MASK);

    // LSB
    uint8_t secondByte = value & MCP4822_FIRST_BYTE_MASK;

    // split for debugging mainly
    tx_data[0] = firstByte;
    tx_data[1] = secondByte;

    // Meaning <CH></na>  </gain>  <MODE>
    // Value    0(A)   0   0(2x)   1(ACTIVE) 1111 1111 1111

    HAL_GPIO_WritePin(config->port, config->cs_pin, GPIO_PIN_RESET);
    HAL_StatusTypeDef spi_status = HAL_SPI_Transmit(config->spi_handler, tx_data, sizeof(tx_data), MCP4822_SPI_TIMEOUT);
    HAL_GPIO_WritePin(config->port, config->cs_pin, GPIO_PIN_SET);

    if (spi_status != HAL_OK)
    {
        return MCP4822_ERROR_SPI;
    }


    return MCP4822_OK;
}

static MCP4822_channel_config* get_chan_config(MCP4822_config* handle, MCP4822_DAC_SELECT dac_channel)
{
    // Assign pointer to the dac_channel configuration
    MCP4822_channel_config* chan_config;
    if (dac_channel == MCP4822_CHANNEL_A)
    {
        chan_config = &(handle->chan_configs.chan_A_config);
    }
    else
    {
        chan_config = &(handle->chan_configs.chan_B_config);
    }

    return chan_config;
}
