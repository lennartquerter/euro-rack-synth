/*
******************************************************************************
* @file           : settings.h
* @author         : Lennart Querter
* @brief          : Header for settings.c file.
*                   Persistent settings (note masks, root, transpose,
*                   calibration) in flash sector 7 (last 128 KB), stored as
*                   an append-only record chain with CRC32. Load on boot,
*                   append on save, erase only when the sector is full.
******************************************************************************
*/

#ifndef __SETTINGS_H
#define __SETTINGS_H

#include "stm32f4xx_hal.h"

// per-channel calibration, all linear two-point fits
struct SETTINGS_cal
{
    float adc_offset_v;    // input volts at ADC code 0 (nominal 5 x 1.815V)
    float adc_v_per_lsb;   // input volts per ADC LSB (negative: inverting stage)
    float dac_offset_code; // DAC code at 0V out
    float dac_code_per_v;  // DAC codes per volt out (nominal 4095/10V)
};

// personality override: auto follows the PB2 MODE strap
#define SETTINGS_MODE_AUTO 0
#define SETTINGS_MODE_QUANTIZER 1
#define SETTINGS_MODE_SEQUENCER 2

struct SETTINGS_data
{
    uint16_t note_mask[2];
    int8_t root[2];
    int8_t transpose[2];
    struct SETTINGS_cal cal[2];
    uint8_t mode_override; // SETTINGS_MODE_*
    uint8_t i2c_instance;  // conductor bus instance 0-3
    uint16_t seq_loaded[2]; // per-lane sequence index
    uint16_t seq_bpm;       // sequencer internal clock
    uint8_t reserved[8];    // zero-filled, room before the next version bump
};

void SETTINGS_init(void);
struct SETTINGS_data* SETTINGS_get(void);
void SETTINGS_load_defaults(void);

// 1 when the boot-time load found a valid record in flash
uint8_t SETTINGS_loaded_from_flash(void);

// call after any edit; the next SETTINGS_poll_autosave picks it up
void SETTINGS_mark_dirty(void);
uint8_t SETTINGS_is_dirty(void);

// immediate save (append record, erase sector first if full/corrupt)
HAL_StatusTypeDef SETTINGS_save(void);

/*
 * Debounced autosave: saves once the settings have been dirty for
 * debounce_ms without further edits. Call from the 1 kHz tick.
 * Returns 1 when a save just happened (so the caller can log it).
 */
uint8_t SETTINGS_poll_autosave(uint32_t debounce_ms);

#endif
