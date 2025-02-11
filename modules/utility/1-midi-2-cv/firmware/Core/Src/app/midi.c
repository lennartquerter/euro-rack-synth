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

int32_t midi_init() {
    // I guess this means that the ring buffer is not initialized well
    if (!buffer_init(&rx_buffer, MIDI_BUFFER_LENGTH)) {
        return MOD_ERR_ARG;
    }

    // Initialize variables for RPN
    rpn_event.flag = 0x00;

    rpn_event.dataentry_lsb
            = rpn_event.dataentry_msb
            = rpn_event.param_lsb
            = rpn_event.param_msb
            = 0x00;

    return 0;
}

int32_t midi_run(MIDI_event *midi_event) {

	 size_t iteration_count = 0;
	 const size_t MAX_ITERATIONS = MIDI_BUFFER_LENGTH;  // Safety limit

    while (1 == midi_pop_buffer()) {
    	 if (++iteration_count >= MAX_ITERATIONS) {
			// Something's wrong, we're stuck
			analyzed_status.stat = START_ANALYSIS;
			return -1;  // Error code
		}

        // will keep returning false when not all events are read
        if (midi_is_event_generated(midi_event)) {
            return 1;
        }
    }
    return 0;
}

bool midi_push_buffer(uint8_t *input) {
	if (!input){
		return false;
	}
    return buffer_push(&rx_buffer, input) == BUFFER_SUCCESS;
}

bool midi_pop_buffer() {
    return buffer_pop(&rx_buffer, &midi_buffer) == BUFFER_SUCCESS;
}

void midi_reset_buffer() {
    buffer_init(&rx_buffer, MIDI_BUFFER_LENGTH);
    analyzed_status.stat = START_ANALYSIS;
    analyzed_status.type = MSG_NOTHING;
    analyzed_status.data_idx = 0;
}

void buffer_debug_info(Buffer *buf) {
    if (!buf) {
        printf("Buffer is NULL\n");
        return;
    }

    printf("Buffer status:\n");
    printf("  Size: %u\n", buf->length);
    printf("  Used: %u\n", buffer_get_size(buf));
    printf("  Front idx: %u\n", buf->idx_front);
    printf("  Rear idx: %u\n", buf->idx_rear);
    printf("  Is empty: %s\n", buffer_is_empty(buf) ? "yes" : "no");
    printf("  Is full: %s\n", buffer_is_full(buf) ? "yes" : "no");
}

bool midi_is_event_generated(MIDI_event *midi_event) {
	static uint8_t last_status = 0;  // For running status

	uint8_t upper_half_byte = (midi_buffer) & 0xF0;
	uint8_t lower_half_byte = (midi_buffer) & 0x0F;

	buffer_debug_info(midi_buffer);

	// Handle running status
	if (!(midi_buffer & 0x80)) {  // If not a status byte
		if (last_status != 0) {   // And we have a previous status
			// Reuse the last status
			upper_half_byte = last_status & 0xF0;
			lower_half_byte = last_status & 0x0F;
		}
	} else {
		last_status = midi_buffer;  // Save new status
	}

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

                case 0x90: // Note On Message.
                    midi_event->type = analyzed_status.type = MSG_NOTE_ON;
                    analyzed_status.stat = WAIT_DATA1;
                    midi_event->channel = lower_half_byte;
                    analyzed_status.channel = lower_half_byte;
                    break;

                case 0x80: // Note Off Message.
                    midi_event->type = analyzed_status.type = MSG_NOTE_OFF;
                    analyzed_status.stat = WAIT_DATA1;
                    midi_event->channel = lower_half_byte;
                    analyzed_status.channel = lower_half_byte;
                    break;

                case 0xE0: // Pitch Bend.
                    midi_event->type = analyzed_status.type = MSG_PITCH;
                    analyzed_status.stat = WAIT_DATA1;
                    midi_event->channel = lower_half_byte;
                    analyzed_status.channel = lower_half_byte;
                    break;

                case 0xB0: // Control Change
                    midi_event->type = analyzed_status.type = MSG_CC;
                    analyzed_status.stat = WAIT_DATA1;
                    midi_event->channel = lower_half_byte;
                    analyzed_status.channel = lower_half_byte;
                    break;

                case 0xC0: // Program Change
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
				if (analyzed_status.type == MSG_NOTE_ON ||
					analyzed_status.type == MSG_NOTE_OFF ||
					analyzed_status.type == MSG_CC) {
					// Validate data range
					if (midi_event->data_byte[1] > 127) {
						analyzed_status.stat = START_ANALYSIS;
						return false;
					}
				}
				analyzed_status.stat = END_ANALYSIS;
                break;

            case WAIT_SYSTEM_DATA:
                midi_event->data_byte[analyzed_status.data_idx++] = (midi_buffer);

                if (analyzed_status.data_idx > (MIDI_DATABYTE_MAX - 1)) {
                    analyzed_status.stat = END_ANALYSIS;
                }
                break;

            case END_ANALYSIS:
            	// Don't modify data here, just reset state
            	analyzed_status.stat = START_ANALYSIS;
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
