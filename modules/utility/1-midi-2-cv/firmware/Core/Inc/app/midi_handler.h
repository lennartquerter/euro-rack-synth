//
// Created by Lennart Querter on 14.02.25.
//

#ifndef MIDI_HANDLER_H
#define MIDI_HANDLER_H

#define MIDI_BUFFER_LENGTH 1024
#define MIDI_DATABYTE_MAX 32

// Bit 7 separates status bytes from data bytes: data bytes are always 0-127
#define MIDI_STATUS_BIT       0x80

// System Real-Time messages are single bytes from 0xF8 upwards. They may appear between any two
// bytes of another message, so they are filtered out before the parser state machine sees them.
#define MIDI_REALTIME_FIRST   0xF8

// Channel voice status bytes, high nibble only (the low nibble is the channel)
#define MIDI_STATUS_NOTE_OFF    0x80
#define MIDI_STATUS_NOTE_ON     0x90
#define MIDI_STATUS_CC          0xB0
#define MIDI_STATUS_PITCH_BEND  0xE0

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
    CC_MOD_WHEEL = 1,
    CC_ALL_NOTES_OFF = 123,
} MIDI_cc_message_type;

typedef struct {
    MIDI_message_type type;
    uint8_t channel;
    // Data bytes in wire order: data_byte[0] is the first data byte after the status byte.
    // For pitch bend that makes [0] the LSB and [1] the MSB, not the other way around.
    uint8_t data_byte[MIDI_DATABYTE_MAX];
} MIDI_event;

typedef struct {
    MIDI_analysis_status stat;
    MIDI_message_type type;
    uint8_t channel;
    uint8_t data_idx;
} MidiAnalysisStatus;

// Both return a negative value on failure, so the return type must be signed: as uint32_t the -1
// error code came back as 4294967295 and read as success at every call site.
int32_t MIDI_HANDLER_init(const struct MIDI_HANDLER_config* cfg);

// Returns 1 when midi_event was filled, 0 when no complete message is available yet, -1 on error.
// Only a return of 1 means midi_event holds valid data.
int32_t MIDI_HANDLER_get_event(MIDI_event *midi_event);
bool MIDI_HANDLER_push_buffer(uint8_t *input);

#endif //MIDI_HANDLER_H
