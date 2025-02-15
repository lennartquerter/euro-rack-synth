#include <stdio.h>

#include "app/mcp4728.h"

HAL_StatusTypeDef MCP4728_Init(I2C_HandleTypeDef* I2CHandler)
{
    // Check if device is responding
    if (HAL_I2C_IsDeviceReady(I2CHandler, MCP4728_ADDR, 3, HAL_MAX_DELAY) != HAL_OK)
    {
        return HAL_ERROR;
    }
    return HAL_OK;
}

// Fast Write - Update all channels quickly
HAL_StatusTypeDef MCP4728_FastWrite(I2C_HandleTypeDef* I2CHandler,
                                    uint16_t ch_a,
                                    uint16_t ch_b,
                                    uint16_t ch_c,
                                    uint16_t ch_d)
{
    uint8_t data[8];

    // Format data for each channel (12-bit values)
    data[0] = (ch_a >> 8) & 0x0F; // Upper 4 bits
    data[1] = ch_a & 0xFF; // Lower 8 bits
    data[2] = (ch_b >> 8) & 0x0F;
    data[3] = ch_b & 0xFF;
    data[4] = (ch_c >> 8) & 0x0F;
    data[5] = ch_c & 0xFF;
    data[6] = (ch_d >> 8) & 0x0F;
    data[7] = ch_d & 0xFF;

    return HAL_I2C_Master_Transmit(I2CHandler, MCP4728_ADDR, data, 8, HAL_MAX_DELAY);
}

// Single Channel Write with configuration
HAL_StatusTypeDef MCP4728_SingleWrite(I2C_HandleTypeDef* I2CHandler, uint8_t channel, uint16_t value, uint8_t vref,
                                      uint8_t gain, uint8_t pd)
{
    uint8_t data[3];

    data[0] = MCP4728_CMD_SINGLE_WRITE | (channel << 1);
    data[1] = (vref << 7) | (pd << 5) | (gain << 4) | ((value >> 8) & 0x0F);
    data[2] = value & 0xFF;

    return HAL_I2C_Master_Transmit(I2CHandler, MCP4728_ADDR, data, 3, HAL_MAX_DELAY);
}

HAL_StatusTypeDef MCP4728_Reset(I2C_HandleTypeDef* I2CHandler)
{
    // General Call Reset
    uint8_t reset_cmd = MCP4728_GENERAL_RESET;
    return HAL_I2C_Master_Transmit(I2CHandler, 0x00, &reset_cmd, 1, HAL_MAX_DELAY);
}

HAL_StatusTypeDef MCP4728_WakeUp(I2C_HandleTypeDef* I2CHandler)
{
    // General Call Wake-Up
    uint8_t wakeup_cmd = MCP4728_GENERAL_WAKEUP;
    return HAL_I2C_Master_Transmit(I2CHandler, 0x00, &wakeup_cmd, 1, HAL_MAX_DELAY);
}
HAL_StatusTypeDef MCP4728_SwUpdate(I2C_HandleTypeDef* I2CHandler)
{
    // General Call Wake-Up
    uint8_t sw_update_cmd = MCP4728_GENERAL_SWUPDATE;
    return HAL_I2C_Master_Transmit(I2CHandler, 0x00, &sw_update_cmd, 1, HAL_MAX_DELAY);
}
