//
// Created by Lennart Querter on 02.02.25.
//

#include "app/cs4270.h"


void cs4270_init(CS4270_config* config, GPIO_TypeDef* reset_port, uint16_t reset_pin, I2C_HandleTypeDef* I2CHandler)
{
    config->reset_port = reset_port;
    config->reset_pin = reset_pin;
    config->i2c_handler = I2CHandler;
}


HAL_StatusTypeDef cs4270_configure(CS4270_config* config)
{
    uint8_t data;
    HAL_StatusTypeDef status;

    // ensure reset pin is pulled low
    HAL_GPIO_WritePin(config->reset_port, config->reset_pin, GPIO_PIN_RESET);

    // Power down chip initially
    data = 0x01;  // Power down
    status = HAL_I2C_Mem_Write(config->i2c_handler, CS4270_ADDR << 1, CS4270_POWER_CTRL,
                              I2C_MEMADD_SIZE_8BIT, &data, 1, 100);
    if(status != HAL_OK) {
        return status;
    }

    // Mode Control: I2S, 24-bit
    data = 0x30;  // I2S mode, 24-bit
    status = HAL_I2C_Mem_Write(config->i2c_handler, CS4270_ADDR << 1, CS4270_MODE_CTRL,
                              I2C_MEMADD_SIZE_8BIT, &data, 1, 100);
    if(status != HAL_OK) {
        return status;
    }

    // ADC/DAC Control
    data = 0x00;  // Normal operation
    status = HAL_I2C_Mem_Write(config->i2c_handler, CS4270_ADDR << 1, CS4270_ADC_DAC_CTRL,
                              I2C_MEMADD_SIZE_8BIT, &data, 1, 100);
    if(status != HAL_OK) {
        return status;
    }

    // Set DAC volume (0dB)
    data = 0x00;
    status = HAL_I2C_Mem_Write(config->i2c_handler, CS4270_ADDR << 1, CS4270_DAC_VOL,
                              I2C_MEMADD_SIZE_8BIT, &data, 1, 100);
    if(status != HAL_OK){
        return status;
    }

    // Set ADC volume (0dB)
    data = 0x00;
    status = HAL_I2C_Mem_Write(config->i2c_handler, CS4270_ADDR << 1, CS4270_ADC_VOL,
                              I2C_MEMADD_SIZE_8BIT, &data, 1, 100);
    if(status != HAL_OK){
        return status;
    }

    // Unmute
    data = 0x00;  // Unmute both DAC channels
    status = HAL_I2C_Mem_Write(config->i2c_handler, CS4270_ADDR << 1, CS4270_MUTE,
                              I2C_MEMADD_SIZE_8BIT, &data, 1, 100);
    if(status != HAL_OK){
        return status;
    }

    // Power up
    data = 0x00;  // Power up all systems
    status = HAL_I2C_Mem_Write(config->i2c_handler, CS4270_ADDR << 1, CS4270_POWER_CTRL,
                              I2C_MEMADD_SIZE_8BIT, &data, 1, 100);

    return status;
}

HAL_StatusTypeDef cs4270_set_volume(CS4270_config* config, uint8_t volume)
{
    // Volume is 0-255, where 0 is 0dB and each step is -0.5dB
    return HAL_I2C_Mem_Write(config->i2c_handler, CS4270_ADDR << 1, CS4270_DAC_VOL,
                            I2C_MEMADD_SIZE_8BIT, &volume, 1, 100);
}

HAL_StatusTypeDef cs4270_mute(CS4270_config* config, bool mute)
{
    uint8_t data = mute ? 0x03 : 0x00;  // 0x03 mutes both channels
    return HAL_I2C_Mem_Write(config->i2c_handler, CS4270_ADDR << 1, CS4270_MUTE,
                            I2C_MEMADD_SIZE_8BIT, &data, 1, 100);
}