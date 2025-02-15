//
// Created by Lennart Querter on 14.02.25.
//

#ifndef MIDI_HANDLER_H
#define MIDI_HANDLER_H

#define MIDI_BUFFER_LENGTH 1024
#define MIDI_DATABYTE_MAX 32

#include "buffer.h"
#include <stdint.h>
#include <stdbool.h>

struct MIDI_HANDLER_config {
    Buffer *buffer;
};

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

typedef enum {
    CC_ALL_NOTES_OFF = 123,
} MIDI_cc_message_type;

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

uint32_t MIDI_HANDLER_init(const struct MIDI_HANDLER_config* cfg);
uint32_t MIDI_HANDLER_get_event(MIDI_event *midi_event);
bool MIDI_HANDLER_push_buffer(uint8_t *input);

#endif //MIDI_HANDLER_H
