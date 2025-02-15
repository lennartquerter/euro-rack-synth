#include <stddef.h>

#include "app/midi_handler.h"
#include "app/buffer.h"

// STATE
struct midi_handler_state
{
    struct MIDI_HANDLER_config cfg;
};

static struct midi_handler_state state;

// PRIVATE VARIABLES

MidiAnalysisStatus analyzed_status;
uint8_t midi_buffer;

// PRIVATE FUNCTIONS

bool midi_is_event_generated(MIDI_event* midi_event);
bool pop_buffer();

// PUBLIC FUNCTIONS

uint32_t MIDI_HANDLER_init(const struct MIDI_HANDLER_config* cfg)
{
    if (!buffer_init(cfg->buffer, MIDI_BUFFER_LENGTH))
    {
        return -1;
    }
    state.cfg = *cfg;

    return 0;
}

uint32_t MIDI_HANDLER_get_event(MIDI_event* midi_event)
{
    size_t iteration_count = 0;

    while (1 == pop_buffer())
    {
        if (++iteration_count >= MIDI_BUFFER_LENGTH)
        {
            // Something's wrong, we're stuck
            analyzed_status.stat = START_ANALYSIS;
            return -1; // Error code
        }

        // will keep returning false when not all events are read
        if (midi_is_event_generated(midi_event))
        {
            // we reset the state and return that a midi event was succesfully generated!
            analyzed_status.stat = START_ANALYSIS;
            return 1;
        }
    }

    return 0;
}

bool MIDI_HANDLER_push_buffer(uint8_t* input)
{
    return buffer_push(state.cfg.buffer, input) == BUFFER_SUCCESS;
}

bool pop_buffer()
{
    return buffer_pop(state.cfg.buffer, &midi_buffer) == BUFFER_SUCCESS;
}

bool midi_is_event_generated(MIDI_event* midi_event)
{
    if (analyzed_status.stat == START_ANALYSIS)
    {
        const uint8_t upper_half_byte = midi_buffer & 0xF0;
        const uint8_t lower_half_byte = midi_buffer & 0x0F;

        switch (upper_half_byte)
        {
        case 144: // Note On Message (0x90)
            midi_event->type = MSG_NOTE_ON;
            analyzed_status.type = MSG_NOTE_ON;
            analyzed_status.stat = WAIT_DATA1;
            midi_event->channel = lower_half_byte;
            analyzed_status.channel = lower_half_byte;
            break;
        case 128: // Note Off Message (0x80)
            midi_event->type = MSG_NOTE_OFF;
            analyzed_status.type = MSG_NOTE_OFF;
            analyzed_status.stat = WAIT_DATA1;
            midi_event->channel = lower_half_byte;
            analyzed_status.channel = lower_half_byte;
            break;
        case 176: // Control Change (0xB)
            midi_event->type = MSG_CC;
            analyzed_status.type = MSG_CC;
            analyzed_status.stat = WAIT_DATA1;
            midi_event->channel = lower_half_byte;
            analyzed_status.channel = lower_half_byte;
            break;
        default:
            // reset the state if we don't understand what is happening
            analyzed_status.stat = START_ANALYSIS;
            return false;
        }
    }
    else if (analyzed_status.stat == WAIT_DATA1)
    {
        // push data into the first data_byte
        midi_event->data_byte[0] = (midi_buffer);

        if (analyzed_status.type == MSG_NOTE_ON ||
            analyzed_status.type == MSG_NOTE_OFF ||
            analyzed_status.type == MSG_PITCH ||
            analyzed_status.type == MSG_CC)
        {
            analyzed_status.stat = WAIT_DATA2;
        }
        else
        {
            return true;
        }
    }
    else if (analyzed_status.stat == WAIT_DATA2)
    {
        midi_event->data_byte[1] = (midi_buffer);
        return true;
    }
    else if (analyzed_status.stat == END_ANALYSIS)
    {
        return true;
    }

    return false;
}
