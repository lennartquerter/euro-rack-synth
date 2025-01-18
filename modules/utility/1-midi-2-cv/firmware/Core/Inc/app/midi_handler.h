#ifndef _MIDI_HANDLER_H_
#define _MIDI_HANDLER_H_

#include "midi.h"

typedef enum {
    MH_SINGLE_DAC,      // Sets the system up to write to 3 channels on a single DAC, only 1 midi channel is supported
    MH_MULTI_DAC,       // Sets the system up to write to 3 different DACs, 4 midi channels are supported
} MIDI_HANDLER_DAC_MODE;

typedef enum {
    MH_CHANNEL,         // Every output lane will be assigned to 1 MIDI channel (Mono)
    MH_SEQUENCE,        // Every note will be set to the next available channel (Mono, sequence)
    MH_POLYPHONIC,      // Only MIDI channel 1 is used, but up to 4 notes are assigned to each lane (POLY)
} MIDI_HANDLER_ASSIGNMENT_MODE;

typedef enum {
    MH_TRIGGER_ON,         // Will restart the gate on EVERY keypress
    MH_TRIGGER_OFF,        // Will not restart the gate on subsequent key press
} MIDI_HANDLER_NOTE_TRIGGER;

struct midi_handler_config {
    MIDI_HANDLER_DAC_MODE mode;
    MIDI_HANDLER_ASSIGNMENT_MODE assignment_mode;
    MIDI_HANDLER_NOTE_TRIGGER trigger_mode;

    I2C_HandleTypeDef *cv_dac1;     // will be used in MH_SINGLE_DAC mode
    I2C_HandleTypeDef *vel_dac2;
    I2C_HandleTypeDef *mod_dac3;

    uint8_t available_channels;    // 4 bits as BIT_MASK (LSB) to denote which channels are active and can be used --> 0b0000ABCD;
};

uint32_t midi_handler_init(struct midi_handler_config *config);

void midi_handler_run(MIDI_event *midi_event);

void midi_handler_set_available_channels(uint8_t available_channels);

#endif /* _MIDI_HANDLER_H_ */