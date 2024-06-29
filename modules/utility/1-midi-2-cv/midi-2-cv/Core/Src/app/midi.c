/*
 * Midi implementation of midi.h
 *
 * Contains logic to drive the handling of the Midi Commands
 * Inspired by CureSynth
 */

#include "app/midi.h"
#include "app/buffer.h"

// ***********
// Private variable
// ***********

MidiAnalysisStatus analyzed_status;
ExtendCCEvent rpn_event;
Buffer rx_buffer;
uint8_t midi_buffer;

// ***********
// Private functions
// ***********

bool midi_is_event_generated(MIDI_event *midi_event);

// ***********
// Implementation
// ***********

bool midi_init() {
    // I guess this means that the ring buffer is not initialized well
    if (!buffer_init(&rx_buffer, MIDI_BUFFER_LENGTH)) {
        return false;
    }

    // Initialize variables for RPN
    rpn_event.flag = 0x00;

    rpn_event.dataentry_lsb
            = rpn_event.dataentry_msb
            = rpn_event.param_lsb
            = rpn_event.param_msb
            = 0x00;

    return true;
}

bool midi_run(MIDI_event *midi_event) {
    while (1 == midi_pop_buffer()) {
        // will keep returning false when not all events are read
        if (midi_is_event_generated(midi_event)) {
            return true;
        }
    }
    return false;
}

bool midi_push_buffer(uint8_t *input) {
    return buffer_push(&rx_buffer, input) == BUFFER_SUCCESS;
}

bool midi_pop_buffer() {
    return buffer_pop(&rx_buffer, &midi_buffer) == BUFFER_SUCCESS;
}

bool midi_is_event_generated(MIDI_event *midi_event) {
    uint8_t upper_half_byte = (midi_buffer) & 0xF0;
    uint8_t lower_half_byte = (midi_buffer) & 0x0F;

    // status byte.
    if (upper_half_byte & 0x80) {
        // MIDI System Message
        if (0xF0 == upper_half_byte) {
            switch (lower_half_byte) {
                case 0x00://SysEx Start
                    midi_event->type = analyzed_status.type = MSG_SYSEX;
                    analyzed_status.data_idx = 0;
                    analyzed_status.stat = WAIT_SYSTEM_DATA;
                    break;

                case 0x07://SysEx End
                    analyzed_status.stat = END_ANALYSIS;
            }
            // MIDI Channel Message.
        } else {
            switch (upper_half_byte) {

                case 0x90: //Note On Message.
                    midi_event->type = analyzed_status.type = MSG_NOTE_ON;
                    analyzed_status.stat = WAIT_DATA1;
                    midi_event->channel = lower_half_byte;
                    analyzed_status.channel = lower_half_byte;
                    break;

                case 0x80: //Note Off Message.
                    midi_event->type = analyzed_status.type = MSG_NOTE_OFF;
                    analyzed_status.stat = WAIT_DATA1;
                    midi_event->channel = lower_half_byte;
                    analyzed_status.channel = lower_half_byte;
                    break;

                case 0xE0: //Pitch Bend.
                    midi_event->type = analyzed_status.type = MSG_PITCH;
                    analyzed_status.stat = WAIT_DATA1;
                    midi_event->channel = lower_half_byte;
                    analyzed_status.channel = lower_half_byte;
                    break;

                case 0xB0: //Control Change
                    midi_event->type = analyzed_status.type = MSG_CC;
                    analyzed_status.stat = WAIT_DATA1;
                    midi_event->channel = lower_half_byte;
                    analyzed_status.channel = lower_half_byte;
                    break;

                case 0xC0: //Program Change
                    midi_event->type = analyzed_status.type = MSG_PROG;
                    analyzed_status.stat = WAIT_DATA1;
                    midi_event->channel = lower_half_byte;
                    analyzed_status.channel = lower_half_byte;
                    break;

                default:
                    midi_event->type = analyzed_status.type = MSG_NOTHING;
                    analyzed_status.stat = START_ANALYSIS;
                    break;
            }
        }
        //data byte
    } else {
        switch (analyzed_status.stat) {

            case WAIT_DATA1:
                midi_event->data_byte[0] = (midi_buffer);
                if (MSG_NOTE_ON == analyzed_status.type || MSG_NOTE_OFF == analyzed_status.type ||
                    MSG_PITCH == analyzed_status.type || MSG_CC == analyzed_status.type) {
                    analyzed_status.stat = WAIT_DATA2;
                } else if (MSG_PROG == analyzed_status.type) {
                    analyzed_status.stat = END_ANALYSIS;
                } else {
                    analyzed_status.stat = START_ANALYSIS;
                }
                break;

            case WAIT_DATA2:
                midi_event->data_byte[1] = (midi_buffer);
                analyzed_status.stat = END_ANALYSIS;
                break;

            case WAIT_SYSTEM_DATA:
                midi_event->data_byte[analyzed_status.data_idx++] = (midi_buffer);

                if (analyzed_status.data_idx > (MIDI_DATABYTE_MAX - 1)) {
                    analyzed_status.stat = END_ANALYSIS;
                }
                break;

            case END_ANALYSIS:
                midi_event->data_byte[0] = (midi_buffer);
                analyzed_status.stat = WAIT_DATA2;
                break;

            case START_ANALYSIS:
            default:
                break;
        }
    }

    if (END_ANALYSIS == analyzed_status.stat) {
        return true;
    } else {
        return false;
    }

}
