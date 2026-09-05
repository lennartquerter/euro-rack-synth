#include <stdio.h>

#include "app/mcp4728.h"

// Sets Vref and gain in the volatile input registers. Fast Write carries neither, so without this
// the module runs on whatever the chip loaded from its EEPROM at power-up and every output voltage
// silently depends on that. Done on every boot; costs two bytes and no EEPROM wear.
HAL_StatusTypeDef MCP4728_SetConfig(I2C_HandleTypeDef* I2CHandler)
{
    // All four channels: internal 2.048V reference
    uint8_t vref_cmd = MCP4728_CMD_WRITE_VREF | 0x0F;
    if (HAL_I2C_Master_Transmit(I2CHandler, MCP4728_ADDR, &vref_cmd, 1, HAL_MAX_DELAY) != HAL_OK)
    {
        return HAL_ERROR;
    }

    // All four channels: gain x1
    uint8_t gain_cmd = MCP4728_CMD_WRITE_GAIN | 0x00;
    if (HAL_I2C_Master_Transmit(I2CHandler, MCP4728_ADDR, &gain_cmd, 1, HAL_MAX_DELAY) != HAL_OK)
    {
        return HAL_ERROR;
    }

    return HAL_OK;
}

// Persists the same configuration to EEPROM so the chip powers up correct even before firmware
// runs. Only called when a read-back shows the stored config differs, so this does not wear the
// EEPROM on every boot.
HAL_StatusTypeDef MCP4728_StoreConfigEEPROM(I2C_HandleTypeDef* I2CHandler)
{
    // Sequential write starting at channel A: command byte, then 2 bytes per channel
    uint8_t data[9];
    data[0] = MCP4728_CMD_SEQ_WRITE; // starting channel A, UDAC = 0

    for (int channel = 0; channel < 4; channel++)
    {
        data[1 + channel * 2] = MCP4728_CONFIG_BYTE; // Vref/PD/gain, upper data nibble zero
        data[2 + channel * 2] = 0x00;                // power up at 0V
    }

    if (HAL_I2C_Master_Transmit(I2CHandler, MCP4728_ADDR, data, sizeof(data), HAL_MAX_DELAY) != HAL_OK)
    {
        return HAL_ERROR;
    }

    // The chip is busy for the duration of the EEPROM write cycle and will NAK until it finishes
    HAL_Delay(MCP4728_EEPROM_WRITE_MS);

    return HAL_OK;
}

HAL_StatusTypeDef MCP4728_Init(I2C_HandleTypeDef* I2CHandler)
{
    // Check if device is responding
    if (HAL_I2C_IsDeviceReady(I2CHandler, MCP4728_ADDR, 3, HAL_MAX_DELAY) != HAL_OK)
    {
        return HAL_ERROR;
    }

    // Make the running configuration correct first -- this is what the outputs actually depend on
    if (MCP4728_SetConfig(I2CHandler) != HAL_OK)
    {
        return HAL_ERROR;
    }

    // Then check what the chip will load at its next power-up, and only rewrite EEPROM if it is
    // wrong. A read returns 6 bytes per channel: 3 for the DAC register, then 3 for the EEPROM.
    uint8_t registers[MCP4728_READ_LENGTH];
    if (HAL_I2C_Master_Receive(I2CHandler, MCP4728_ADDR, registers, sizeof(registers), HAL_MAX_DELAY) != HAL_OK)
    {
        return HAL_ERROR;
    }

    for (int channel = 0; channel < 4; channel++)
    {
        // Byte 1 of the EEPROM triple holds Vref, the power-down bits and the gain bit
        const uint8_t stored_config = registers[channel * 6 + 4] & 0xF0;

        if (stored_config != MCP4728_CONFIG_BYTE)
        {
            return MCP4728_StoreConfigEEPROM(I2CHandler);
        }
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
