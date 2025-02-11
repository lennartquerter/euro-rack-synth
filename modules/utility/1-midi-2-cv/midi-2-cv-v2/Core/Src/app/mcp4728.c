/*
 * Datasheet: https://ww1.microchip.com/downloads/aemDocuments/documents/OTH/ProductDocuments/DataSheets/22187E.pdf
 *
 */

#include "app/mcp4728.h"

#include <stdio.h>

#define MCP4728_ADDR (0x60 << 1)  // Shift for HAL's 8-bit address format

// Command bytes
#define MCP4728_CMD_FAST_WRITE    0x00
#define MCP4728_CMD_MULTI_WRITE   0x40
#define MCP4728_CMD_SINGLE_WRITE  0x58
#define MCP4728_CMD_SEQ_WRITE     0x50

// Initialize DAC
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
HAL_StatusTypeDef MCP4728_FastWrite(I2C_HandleTypeDef* I2CHandler, uint16_t ch_a, uint16_t ch_b, uint16_t ch_c,
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

// Example function to set voltage (assuming VREF = 3.3V)
float MCP4728_SetVoltage(I2C_HandleTypeDef* I2CHandler, uint8_t channel, float voltage)
{
    // Convert voltage to 12-bit value
    uint16_t digital_value = (uint16_t)((voltage * 4096.0f) / 3.3f);
    if (digital_value > 4095) digital_value = 4095; // Limit to 12-bit

    return MCP4728_SingleWrite(I2CHandler, channel, digital_value, 1, 0, 0);
    // VREF = internal, Gain = 1x, Power = normal
}

HAL_StatusTypeDef MCP4728_Reset(I2C_HandleTypeDef* I2CHandler)
{
    // General Call Reset
    uint8_t reset_cmd = 0x06;
    return HAL_I2C_Master_Transmit(I2CHandler, 0x00, &reset_cmd, 1, HAL_MAX_DELAY);
}

HAL_StatusTypeDef MCP4728_WakeUp(I2C_HandleTypeDef* I2CHandler)
{
    // General Call Wake-Up
    uint8_t wakeup_cmd = 0x09;
    return HAL_I2C_Master_Transmit(I2CHandler, 0x00, &wakeup_cmd, 1, HAL_MAX_DELAY);
}

HAL_StatusTypeDef I2C_Reset(I2C_HandleTypeDef I2CHandler)
{
    // Disable I2C
    I2CHandler.Instance->CR1 &= ~I2C_CR1_PE;

    // Configure GPIO pins as GPIO output
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // Toggle SCL 9 times
    for(int i = 0; i < 9; i++) {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
        HAL_Delay(1);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
        HAL_Delay(1);
    }

    // Generate STOP condition
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);

    // Reconfigure I2C pins
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // Re-enable I2C
    return HAL_I2C_Init(&I2CHandler);
}


HAL_StatusTypeDef I2C_Check_Error(I2C_HandleTypeDef I2CHandler)
{
    // Check I2C errors
    if ((I2CHandler.Instance->SR1 & I2C_SR1_BERR) == I2C_SR1_BERR)
    {
        printf("Bus Error\n");
    }
    if ((I2CHandler.Instance->SR1 & I2C_SR1_ARLO) == I2C_SR1_ARLO)
    {
        printf("Arbitration Lost\n");
    }
    if ((I2CHandler.Instance->SR1 & I2C_SR1_AF) == I2C_SR1_AF)
    {
        printf("Acknowledge Failure\n");
    }
    if ((I2CHandler.Instance->SR1 & I2C_SR1_OVR) == I2C_SR1_OVR)
    {
        printf("Overrun/Underrun\n");
    }
    if ((I2CHandler.Instance->SR1 & I2C_SR1_TIMEOUT) == I2C_SR1_TIMEOUT)
    {
        printf("Timeout Error\n");
    }

    // Check if busy
    if ((I2CHandler.Instance->SR2 & I2C_SR2_BUSY) == I2C_SR2_BUSY)
    {
        printf("Bus is busy\n");
    }

    return HAL_OK;
}
