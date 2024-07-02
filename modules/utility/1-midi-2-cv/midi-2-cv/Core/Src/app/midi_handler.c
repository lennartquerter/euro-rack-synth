#include <string.h>

#include "app/midi_handler.h"
#include "app/mcp4728.h"
#include "libs/notes.h"
#include "main.h"

//******************
// State & Configuration
//******************

struct midi_handler_state {
    struct midi_handler_config cfg;
    uint8_t current_channel;
};

static struct midi_handler_state state;

//******************
// Private Functions
//******************

typedef struct {
    GPIO_TypeDef *port;
    uint16_t pin;
    uint8_t channel;
} midi_handler_trigger_channel;

void midi_handle_note_on(MIDI_event *midi_event);

void midi_handle_cc(MIDI_event *midi_event);

void midi_handle_note_off(MIDI_event *midi_event);

void midi_handle_pitch(MIDI_event *midi_event);

void midi_channel_to_trigger_channel(uint8_t midi_channel, midi_handler_trigger_channel *trigger_channel);

uint8_t midi_get_note(uint8_t midi_note);

uint8_t midi_get_octave(uint8_t midi_note);

//******************
// Implementation
//******************

uint32_t midi_handler_init(struct midi_handler_config *cfg) {
    if (cfg == NULL) {
        return MOD_ERR_ARG;
    }

    memset(&state, 0, sizeof(state));
    state.cfg = *cfg;

    // used to track channel mode
    state.current_channel = 0x00;

    // CV output
    MCP4728_Init(state.cfg.cv_dac1);

    if (state.cfg.mode == MH_MULTI_DAC) {
        // VEL output
        MCP4728_Init(state.cfg.vel_dac2);

        // MOD output
        MCP4728_Init(state.cfg.mod_dac3);
    }

    return 0;
}

void midi_handler_run(MIDI_event *midi_event) {
    switch (midi_event->type) {
        case MSG_NOTE_OFF:
            midi_handle_note_off(midi_event);
            break;

        case MSG_NOTE_ON:
            midi_handle_note_on(midi_event);
            break;
        case MSG_PITCH:
            midi_handle_pitch(midi_event);
            break;
        case MSG_CC:
            midi_handle_cc(midi_event);
            break;

        default:
            break;
    }
}

void midi_handler_set_available_channels(uint8_t available_channels) {
    state.cfg.available_channels = available_channels;
}

// ********************
// Private functions
// ********************

void midi_handle_note_on(MIDI_event *midi_event) {
    // check velocity (probably in WAIT_DATA_1)
    // if === 0 --> send note_off
    uint8_t velocity = midi_event->data_byte[1];

    // early return for 0 velocity
    if (velocity == 0) {
        midi_handle_note_off(midi_event);
        return;
    }

    if (state.cfg.assignment_mode == MH_SEQUENCE) {
        // todo: check for next available channel
        // assign next channel
        state.current_channel = 0;
    }

    // get Note/Octave for midi note
    uint8_t midi_note = midi_event->data_byte[0];

    uint8_t note = midi_get_note(midi_note);
    uint8_t octave = midi_get_octave(midi_note);

    // calculate the CV value
    uint16_t value = (octave * OCT_VALUE) + (note * (OCT_VALUE / 12));

    if (state.cfg.mode == MH_SINGLE_DAC) {
        // only 1 channel is supported in MH_SINGLE_DAC mode
        if (midi_event->channel != 0) {
            return;
        }

        // Always write to DAC_1
        MCP4728_Write_Voltage(state.cfg.cv_dac1, MCP4728_CHANNEL_A, value);
        MCP4728_Write_Voltage(state.cfg.cv_dac1, MCP4728_CHANNEL_B, velocity * 4);

        if (state.cfg.trigger_mode == MH_TRIGGER_ON) {
            HAL_GPIO_WritePin(GATE_1_OUT_GPIO_Port, GATE_1_OUT_Pin, GPIO_PIN_RESET);
            HAL_Delay(10);
        }

        HAL_GPIO_WritePin(GATE_1_OUT_GPIO_Port, GATE_1_OUT_Pin, GPIO_PIN_SET);
        HAL_Delay(10);

    } else {
        // Assign right channel for the midi-channel

        midi_handler_trigger_channel trigger_channel;
        midi_channel_to_trigger_channel(midi_event->channel, &trigger_channel);

        // -- Write all values
        // -- Todo, the MCP shoudld have a bulk write for 2 channels?
        MCP4728_Write_Voltage(state.cfg.cv_dac1, trigger_channel.channel, value);

        // velocity goes from 1 --> 127
        // Output goes from 0 --> 2000
        // 2000 / 127 = 15 --> we use a 15x translation for a wide velocity amount
        // Todo: check if this actually makes sense, or set a bit for high/low range?
        MCP4728_Write_Voltage(state.cfg.vel_dac2, trigger_channel.channel, velocity * 4);

        midi_handler_trigger_channel gate_channel;
        midi_channel_to_trigger_channel(midi_event->channel, &gate_channel);

        if (state.cfg.trigger_mode == MH_TRIGGER_ON) {
            HAL_GPIO_WritePin(gate_channel.port, gate_channel.pin, GPIO_PIN_RESET);
            HAL_Delay(10);
        }
        HAL_GPIO_WritePin(gate_channel.port, gate_channel.pin, GPIO_PIN_SET);
        HAL_Delay(10);
    }

}

void midi_handle_note_off(MIDI_event *midi_event) {
    if (state.cfg.mode == MH_SINGLE_DAC) {
        // only 1 channel is supported in MH_SINGLE_DAC mode
        if (midi_event->channel != 0) {
            return;
        }

        HAL_GPIO_WritePin(GATE_1_OUT_GPIO_Port, GATE_1_OUT_Pin, GPIO_PIN_RESET);
        HAL_Delay(10);
    } else {
        midi_handler_trigger_channel trigger_channel;
        midi_channel_to_trigger_channel(midi_event->channel, &trigger_channel);

        HAL_GPIO_WritePin(trigger_channel.port, trigger_channel.pin, GPIO_PIN_RESET);
        HAL_Delay(10);
    }
}

void midi_handle_pitch(MIDI_event *midi_event) {
    uint8_t pitch = midi_event->data_byte[0];

    // I think this should be a configuration value
    // used to multiply the CV source (higher == more range, maximum is ~15)
    uint8_t multiplier = 5;

    if (state.cfg.mode == MH_SINGLE_DAC) {
        MCP4728_Write_Voltage(state.cfg.cv_dac1, MCP4728_CHANNEL_C, pitch * multiplier);
    } else {
        midi_handler_trigger_channel trigger_channel;
        midi_channel_to_trigger_channel(midi_event->channel, &trigger_channel);

        // return if no channel assigned
        if (trigger_channel.channel > 0x04) {
            return;
        }

        MCP4728_Write_Voltage(state.cfg.vel_dac2, trigger_channel.channel, pitch * multiplier);
    }
}

void midi_handle_cc(MIDI_event *midi_event) {
    // CC sends on data_byte[0] to control knob
    // CC sends on data_byte[1] to value
    uint8_t control = midi_event->data_byte[0];
    uint8_t value = midi_event->data_byte[1];

    // I think this should be a configuration value
    // used to multiply the CV source (higher == more range, maximum is ~15)
    uint8_t multiplier = 5;

    // Mod wheel!
    if (control == 1) {
        if (state.cfg.mode == MH_SINGLE_DAC) {
            MCP4728_Write_Voltage(state.cfg.cv_dac1, MCP4728_CHANNEL_D, value * multiplier);
        } else {
            midi_handler_trigger_channel trigger_channel;
            midi_channel_to_trigger_channel(midi_event->channel, &trigger_channel);

            MCP4728_Write_Voltage(state.cfg.mod_dac3, trigger_channel.channel, value * multiplier);
        }
    }
}

// ********************
// Helper functions
// ********************

uint8_t midi_get_note(uint8_t midi_note) {
    uint8_t note = NOTE_C;

    // find note value by simple division
    uint8_t restValue = midi_note % 12;

    switch (restValue) {
        case 0:
            note = NOTE_C;
            break;
        case 1:
            note = NOTE_CS;
            break;
        case 2:
            note = NOTE_D;
            break;
        case 3:
            note = NOTE_DS;
            break;
        case 4:
            note = NOTE_E;
            break;
        case 5:
            note = NOTE_F;
            break;
        case 6:
            note = NOTE_FS;
            break;
        case 7:
            note = NOTE_G;
            break;
        case 8:
            note = NOTE_GS;
            break;
        case 9:
            note = NOTE_A;
            break;
        case 10:
            note = NOTE_AS;
            break;
        case 11:
            note = NOTE_B;
            break;
        default:
            note = NOTE_C;
            break;
    }

    return note;
}

uint8_t midi_get_octave(uint8_t midi_note) {
    uint8_t octave = OCTAVE_0;

    if (midi_note < 24) {
        octave = OCTAVE_0;
    } else if (midi_note >= 24 && midi_note < 36) {
        octave = OCTAVE_1;
    } else if (midi_note >= 36 && midi_note < 48) {
        octave = OCTAVE_2;
    } else if (midi_note >= 48 && midi_note < 60) {
        octave = OCTAVE_3;
    } else if (midi_note >= 60 && midi_note < 72) {
        octave = OCTAVE_4;
    } else if (midi_note >= 72 && midi_note < 84) {
        octave = OCTAVE_5;
    } else if (midi_note >= 84) {
        octave = OCTAVE_6;
    }

    return octave;
}

void midi_channel_to_trigger_channel(uint8_t midi_channel, midi_handler_trigger_channel *trigger_channel) {
    uint8_t channel_to_select;

    if (state.cfg.assignment_mode == MH_CHANNEL) {
        channel_to_select = midi_channel;
    } else {
        // will be incremented by each note trigger
        // based on available channels
        channel_to_select = state.current_channel;
    }

    if (channel_to_select == 0) {
        trigger_channel->port = GATE_1_OUT_GPIO_Port;
        trigger_channel->pin = GATE_1_OUT_Pin;
        trigger_channel->channel = MCP4728_CHANNEL_A;
    } else if (channel_to_select == 1) {
        trigger_channel->port = GATE_2_OUT_GPIO_Port;
        trigger_channel->pin = GATE_2_OUT_Pin;
        trigger_channel->channel = MCP4728_CHANNEL_B;
    } else if (channel_to_select == 2) {
        trigger_channel->port = GATE_3_OUT_GPIO_Port;
        trigger_channel->pin = GATE_3_OUT_Pin;
        trigger_channel->channel = MCP4728_CHANNEL_C;
    } else if (channel_to_select == 3) {
        trigger_channel->port = GATE_4_OUT_GPIO_Port;
        trigger_channel->pin = GATE_4_OUT_Pin;
        trigger_channel->channel = MCP4728_CHANNEL_D;
    } else {
        trigger_channel->port = CH_16_OUT_GPIO_Port;
        trigger_channel->pin = CH_16_OUT_Pin;

        // channel does not exists on the MCP4728
        trigger_channel->channel = 0x04;
    }
}