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

// Running status: the last channel voice status byte received. Senders are allowed to omit the
// status byte on consecutive messages of the same type, and most of them do so for note streams,
// so an unheralded data byte belongs to whatever is recorded here.
static MIDI_message_type running_status_type = MSG_NOTHING;
static uint8_t running_status_channel;

// Data bytes of the message currently being assembled. A message normally spans several calls and
// the caller hands us a different MIDI_event each time, so partial results cannot be parked in the
// caller's struct -- they have to live here until the message is complete.
static uint8_t pending_data[2];

// PRIVATE FUNCTIONS

bool midi_is_event_generated(MIDI_event* midi_event);
static void start_new_message(void);
bool pop_buffer();

// PUBLIC FUNCTIONS

int32_t MIDI_HANDLER_init(const struct MIDI_HANDLER_config* cfg)
{
    if (!buffer_init(cfg->buffer, MIDI_BUFFER_LENGTH))
    {
        return -1;
    }
    state.cfg = *cfg;

    return 0;
}

int32_t MIDI_HANDLER_get_event(MIDI_event* midi_event)
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

// Handles a status byte. A status byte always begins a new message, abandoning any partially
// received one, and never completes a message by itself.
static void start_new_message(void)
{
    const uint8_t upper_half_byte = midi_buffer & 0xF0;
    const uint8_t lower_half_byte = midi_buffer & 0x0F;

    MIDI_message_type type;

    switch (upper_half_byte)
    {
    case MIDI_STATUS_NOTE_ON:
        type = MSG_NOTE_ON;
        break;
    case MIDI_STATUS_NOTE_OFF:
        type = MSG_NOTE_OFF;
        break;
    case MIDI_STATUS_CC:
        type = MSG_CC;
        break;
    case MIDI_STATUS_PITCH_BEND:
        type = MSG_PITCH;
        break;
    default:
        // System Common (0xF0-0xF7) and the channel messages we do not decode. These cancel
        // running status, so the data bytes that follow are discarded one by one instead of being
        // mistaken for notes. That is what safely skips over a System Exclusive dump.
        analyzed_status.stat = START_ANALYSIS;
        running_status_type = MSG_NOTHING;
        return;
    }

    analyzed_status.type = type;
    analyzed_status.channel = lower_half_byte;
    analyzed_status.stat = WAIT_DATA1;

    // Remember it: the sender may drop the status byte on the messages that follow
    running_status_type = type;
    running_status_channel = lower_half_byte;
}

bool midi_is_event_generated(MIDI_event* midi_event)
{
    // System Real-Time is a single byte that may legally appear between any two bytes of another
    // message -- clock arrives 24 times per beat and active sensing roughly every 300ms. It must
    // not disturb the message being assembled, and it does not cancel running status.
    if (midi_buffer >= MIDI_REALTIME_FIRST)
    {
        return false;
    }

    if (midi_buffer & MIDI_STATUS_BIT)
    {
        start_new_message();
        return false;
    }

    // Past this point the byte is a data byte: bit 7 is known clear, so the value is 0-127
    if (analyzed_status.stat == START_ANALYSIS)
    {
        if (running_status_type == MSG_NOTHING)
        {
            // Nothing to repeat -- a stray data byte, or the payload of a message we skipped
            return false;
        }

        analyzed_status.type = running_status_type;
        analyzed_status.channel = running_status_channel;
        analyzed_status.stat = WAIT_DATA1;
    }

    if (analyzed_status.stat == WAIT_DATA1)
    {
        pending_data[0] = midi_buffer;

        // Every message this parser decodes carries two data bytes
        analyzed_status.stat = WAIT_DATA2;
        return false;
    }

    if (analyzed_status.stat == WAIT_DATA2)
    {
        pending_data[1] = midi_buffer;

        // The message is complete, so the caller's event is filled in one go from parser state.
        // Nothing is written into it before this point: a message spans several calls and the
        // caller supplies a different (uninitialised) event each time.
        midi_event->type = analyzed_status.type;
        midi_event->channel = analyzed_status.channel;
        midi_event->data_byte[0] = pending_data[0];
        midi_event->data_byte[1] = pending_data[1];

        // A Note On with velocity 0 is how most keyboards and DAWs release a note. Without this it
        // runs the note-on path with zero velocity and the gate never falls.
        if (midi_event->type == MSG_NOTE_ON && midi_event->data_byte[1] == 0)
        {
            midi_event->type = MSG_NOTE_OFF;
        }

        return true;
    }

    return false;
}
