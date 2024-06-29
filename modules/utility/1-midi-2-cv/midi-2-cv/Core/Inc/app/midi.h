/*
 * Midi functionality
 * Used to analyze RX messages coming from UART
 * Push messages into the midi_buffer during UART interrupt
 * Read messages during main loop, analyze and execute the midi command
 */

#ifndef _M2C_MIDI_H_
#define _M2C_MIDI_H_

#define MIDI_BUFFER_LENGTH (1024)
#define MIDI_DATABYTE_MAX (32)

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "stm32f4xx_hal.h"

typedef enum {
    START_ANALYSIS,    // Initial Status, including exception.
    WAIT_DATA1,        // Waiting data byte(1st byte)
    WAIT_DATA2,        // Waiting data byte(2nd byte)
    WAIT_SYSTEM_DATA,  // Waiting data byte(system exclusive)
    END_ANALYSIS       // Analysis is ended.
} MIDI_analysis_status;

typedef enum {
    EXCC_START,
    EXCC_PARAM,
    EXCC_DAT
} MIDI_extend_cc_status;

typedef enum {
    MSG_NOTHING,    // Exception(can't resolved, missing data, etc.)
    MSG_NOTE_ON,    // Note-on message
    MSG_NOTE_OFF,   // Note-off message
    MSG_PITCH,      // PitchBend message
    MSG_SYSEX,      // System Exclusive message
    MSG_CC,         // Control Change message
    MSG_PROG,       // Program Change message
} MIDI_message_type;

typedef struct {
    MIDI_message_type type;
    uint8_t channel;
    uint8_t data_byte[MIDI_DATABYTE_MAX]; //data_byte[0]=MSB, [1]=LSB, [2]=OTHER...(e.g. sysEx, Control Change...)
} MIDI_event;

typedef struct {
    MIDI_analysis_status stat;
    MIDI_message_type type;
    uint8_t channel;
    uint8_t data_idx;
} MidiAnalysisStatus;

typedef struct {
    MIDI_extend_cc_status stat;
    uint8_t flag; // RPN Received flag. half lower byte means :param_msb, param_lsb, dataentry_msb, dataentry_lsb (e.g. 0b00000010: dataentry_msb is received)
    uint8_t param_msb, param_lsb;
    uint8_t dataentry_msb, dataentry_lsb;
} ExtendCCEvent;

extern uint8_t midi_buffer;

bool midi_run(MIDI_event *midi_event);

bool midi_init();

bool midi_push_buffer(uint8_t *input);

bool midi_pop_buffer();

#endif /* _M2C_MIDI_H_ */