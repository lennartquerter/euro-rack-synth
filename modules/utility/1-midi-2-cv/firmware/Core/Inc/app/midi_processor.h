//
// Created by Lennart Querter on 14.02.25.
//

#ifndef MIDI_PROCESSOR_H
#define MIDI_PROCESSOR_H

#include <stdint.h>
#include "midi_handler.h"
#include "stm32f4xx_hal.h"

#define NUMBER_OF_CHANNELS                  4
#define NUMBER_OF_NOTES_PER_CHANNEL         4
#define MIDI_NOTES_LENGTH                   127
#define DEFAULT_DAC_MAX_VALUE               2000
#define DEFAULT_PITCH_BEND                  250

#define PITCH_CENTER                        8192
#define PITCH_RANGE                         8192

typedef enum
{
    MIDI_MODE_CHANNEL,
    MIDI_MODE_POLY,
    MIDI_MODE_SEQUENCE,
} MIDI_PROCESSOR_mode;

struct MIDI_PROCESSOR_config
{
    I2C_HandleTypeDef* cv_dac1; // will be used in MH_SINGLE_DAC mode
    I2C_HandleTypeDef* vel_dac2;
    I2C_HandleTypeDef* mod_dac3;

    uint8_t available_channels;
    // 4 bits as BIT_MASK (LSB) to denote which channels are active and can be used --> 0b0000ABCD;
    MIDI_PROCESSOR_mode mode;
};

typedef struct
{
    bool is_on;
    uint8_t number;
    uint8_t note_value;

    uint32_t cv;
    uint32_t velocity;
    uint32_t mod;
} MIDI_PROCESSOR_note;

typedef struct
{
    uint8_t channel;

    MIDI_PROCESSOR_note notes[4];
} MIDI_PROCESSOR_channel;

uint8_t MIDI_PROCESSOR_init(const struct MIDI_PROCESSOR_config* cfg);
void MIDI_PROCESSOR_handle_event(MIDI_event* midi_event);
void MIDI_PROCESSOR_mode_changed();

#endif //MIDI_PROCESSOR_H
