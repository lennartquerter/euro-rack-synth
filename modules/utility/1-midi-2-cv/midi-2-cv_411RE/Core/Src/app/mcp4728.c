/*
 * Datasheet: https://ww1.microchip.com/downloads/aemDocuments/documents/OTH/ProductDocuments/DataSheets/22187E.pdf
 *
 */

#include "app/mcp4728.h"

void MCP4728_Write_GeneralCall(I2C_HandleTypeDef *I2CHandler, uint8_t command) {
    HAL_I2C_Master_Transmit(I2CHandler, 0x00, &command, 1, HAL_MAX_DELAY);
}

void mcp4728_write_voltage(I2C_HandleTypeDef *I2CHandler, uint16_t channel, uint16_t output) {
    uint8_t buf[3];
    uint8_t command = MCP4728_CMD_DACWRITE_SINGLE;

    // MASK only the 8 LSB
    uint8_t lowByte = output & 0xff;
    // SHIFT RIGHT (12 bit shift --> 4 MSB)
    uint8_t highByte = (output >> 8);

    buf[0] = command | (channel << 1);
    buf[1] = (0 << 7) | (channel << 5) | highByte;
    buf[2] = lowByte;

    HAL_I2C_Master_Transmit(I2CHandler, MCP4728_BASE_ADDR, buf, sizeof(buf), HAL_MAX_DELAY);

    MCP4728_Write_GeneralCall(I2CHandler, MCP4728_GENERAL_SWUPDATE);
}


void mcp4728_init(I2C_HandleTypeDef *I2CHandler) {
    MCP4728_Write_GeneralCall(I2CHandler, MCP4728_GENERAL_RESET);
    MCP4728_Write_GeneralCall(I2CHandler, MCP4728_GENERAL_WAKEUP);

    uint8_t buf[9];

    buf[0] = MCP4728_CMD_DACWRITE_SEQ;

    for (uint8_t i = 1; i <= 4; i++) {
        buf[(i * 2) + 1] = 0x00;
        buf[(i * 2)] = (0 << 7) | ((i - 1) << 4) | 0x0;
    }

    HAL_I2C_Master_Transmit(I2CHandler, MCP4728_BASE_ADDR, buf, sizeof(buf), HAL_MAX_DELAY);
    MCP4728_Write_GeneralCall(I2CHandler, MCP4728_GENERAL_SWUPDATE);
}