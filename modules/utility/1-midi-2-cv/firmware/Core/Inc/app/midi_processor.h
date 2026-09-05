//
// Created by Lennart Querter on 14.02.25.
//

#ifndef MIDI_PROCESSOR_H
#define MIDI_PROCESSOR_H

#include <stdint.h>
#include "midi_handler.h"
#include "stm32f4xx_hal.h"

#define NUMBER_OF_CHANNELS                  4

// In Poly and Sequence mode the module listens to a single MIDI channel, which drives all four
// outputs. 0 is the channel nibble for MIDI channel 1.
#define MIDI_INPUT_CHANNEL                  0
#define NUMBER_OF_NOTES_PER_CHANNEL         4
#define MIDI_NOTES_LENGTH                   127

// The MCP4728 is driven with Fast Write, which carries no Vref/gain bits, so the chip uses its
// EEPROM-stored configuration. That is the factory default: internal 2.048V reference at gain 1,
// i.e. 4096 codes across 2.048V => 0.5mV per code. The output stage amplifies by 4x, so the
// module produces 500 DAC codes per volt at the jack.
#define DAC_MAX_CODE                        4095    // 2.048V at the DAC => 8.19V at the output
#define DAC_CODES_PER_OUTPUT_VOLT           500

// Full-scale span for the VEL/MOD outputs: 2.000V at the DAC => 8.00V at the output.
#define DEFAULT_DAC_MAX_VALUE               4000

// 1V/oct scaling for the pitch CV. Volts per octave is a fixed physical constant, not a fraction
// of the DAC range, so the pitch CV must not be scaled like VEL/MOD. CV_LOWEST_NOTE is the MIDI
// note placed at 0V; C0 (12) puts C4 (60) at exactly 4.000V. Notes below it clamp to 0V; the
// highest note that still tracks is D8 (110, 8.166V), above which the DAC clamps at 8.19V.
#define SEMITONES_PER_OCTAVE                12
#define CV_LOWEST_NOTE                      12

// Pitch bend range in semitones either side of centre. Bend is applied to the pitch CV, so this is
// a musical interval, not a DAC span: +/-2 semitones is +/-83.3 codes (+/-0.167V at the jack).
#define DEFAULT_PITCH_BEND_SEMITONES        2

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

    // Current pitch bend for this channel, as a signed DAC code offset added to every note's CV.
    // Held separately from the notes so a bend can be re-applied without losing the played note.
    int16_t pitch_bend_codes;

    MIDI_PROCESSOR_note notes[4];
} MIDI_PROCESSOR_channel;

uint8_t MIDI_PROCESSOR_init(const struct MIDI_PROCESSOR_config* cfg);
void MIDI_PROCESSOR_handle_event(MIDI_event* midi_event);
void MIDI_PROCESSOR_mode_changed();

#endif //MIDI_PROCESSOR_H
