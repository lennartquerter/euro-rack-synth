#include "app/midi_handler.h"
#include "app/midi_processor.h"
#include "main.h"
#include "app/mcp4728.h"

#include <string.h>
#include <sys/stat.h>

#include "stm32f4xx_hal.h"

struct midi_processor_state
{
    struct MIDI_PROCESSOR_config cfg;
    uint8_t current_channel;

    MIDI_PROCESSOR_channel midi_processor_channel[4];
};

static struct midi_processor_state state;

// PRIVATE FUNCTIONS

void MIDI_PROCESSOR_node_on(MIDI_event* midi_event);
void MIDI_PROCESSOR_node_off(MIDI_event* midi_event);
uint16_t midi_to_voltage(uint8_t midi_note);
uint16_t gate_to_gpio_pin(uint8_t gate);
GPIO_TypeDef* gate_to_gpio_port(uint8_t gate);

void write();

// PUBLIC FUNCTIONS

uint8_t MIDI_PROCESSOR_init(struct MIDI_PROCESSOR_config* cfg)
{
    if (cfg == NULL)
    {
        return -1;
    }

    memset(&state, 0, sizeof(state));
    state.cfg = *cfg;

    // used to track channel mode
    state.current_channel = 0x00;

    for (int channel_idx = 0; channel_idx < 4; channel_idx++)
    {
        state.midi_processor_channel[channel_idx].channel = channel_idx;

        for (int note_idx = 0; note_idx < 4; note_idx++)
        {
            state.midi_processor_channel[channel_idx].notes[note_idx].is_on = false;
            state.midi_processor_channel[channel_idx].notes[note_idx].number = note_idx;
        }
    }

    return 0;
}

void MIDI_PROCESSOR_handle_event(MIDI_event* midi_event)
{
    switch (midi_event->type)
    {
    case MSG_NOTE_ON:
        MIDI_PROCESSOR_node_on(midi_event);
        break;

    case MSG_NOTE_OFF:
        MIDI_PROCESSOR_node_off(midi_event);
        break;

    case MSG_PITCH:
        // midi_handle_pitch(midi_event);
        break;
    case MSG_CC:
        // midi_handle_cc(midi_event);
        break;

    default:
        break;
    }

    write();
}


void MIDI_PROCESSOR_node_on(MIDI_event* midi_event)
{
    const uint8_t note = midi_event->data_byte[0];
    const uint8_t velocity = midi_event->data_byte[1];

    const uint16_t voltage_pitch = midi_to_voltage(note);
    const uint16_t voltage_velocity = midi_to_voltage(velocity);
    if (state.cfg.mode == MIDI_MODE_CHANNEL)
    {
        state.midi_processor_channel[midi_event->channel].notes[0].is_on = true;
        state.midi_processor_channel[midi_event->channel].notes[0].note_value = note;
        state.midi_processor_channel[midi_event->channel].notes[0].cv = voltage_pitch;
        state.midi_processor_channel[midi_event->channel].notes[0].velocity = voltage_velocity;
    }
    else if (state.cfg.mode == MIDI_MODE_POLY)
    {
        for (int note_idx = 0; note_idx < 4; note_idx++)
        {
            // find the first one that is NOT on --> turn it on and break
            if (state.midi_processor_channel[0].notes[note_idx].is_on == false)
            {
                state.midi_processor_channel[0].notes[note_idx].is_on = true;
                state.midi_processor_channel[0].notes[note_idx].note_value = note;
                state.midi_processor_channel[0].notes[note_idx].cv = voltage_pitch;
                state.midi_processor_channel[0].notes[note_idx].velocity = voltage_velocity;
                break;
            }
        }
    }
}

void MIDI_PROCESSOR_node_off(MIDI_event* midi_event)
{
    const uint8_t note = midi_event->data_byte[0];
    state.midi_processor_channel[midi_event->channel].notes[0].is_on = false;
    if (state.cfg.mode == MIDI_MODE_CHANNEL)
    {
        state.midi_processor_channel[midi_event->channel].notes[0].is_on = false;
    }
    else if (state.cfg.mode == MIDI_MODE_POLY)
    {
        for (int note_idx = 0; note_idx < 4; note_idx++)
        {
            // find the first one that matches the note being played; turn it off; break;
            if (state.midi_processor_channel[0].notes[note_idx].note_value == note)
            {
                state.midi_processor_channel[0].notes[note_idx].is_on = false;
                break;
            }
        }
    }
}

uint16_t midi_to_voltage(uint8_t midi_note)
{
    // Ensure MIDI note is within valid range (0-127)
    if (midi_note > 127)
    {
        midi_note = 127;
    }

    // Convert MIDI note to voltage (0-2000) [amplified by 4x gain]
    // 2000 / 127 ≈ 15.748, but we'll use 15.75 for integer math
    const uint16_t voltage = (uint32_t)midi_note * 2000 / 127;

    return voltage;
}

GPIO_TypeDef* gate_to_gpio_port(uint8_t gate)
{
    switch (gate)
    {
    case 0:
        return GATE_1_OUT_GPIO_Port;
    case 1:
        return GATE_2_OUT_GPIO_Port;
    case 2:
        return GATE_3_OUT_GPIO_Port;
    case 3:
        return GATE_4_OUT_GPIO_Port;
    default: break;
    }
    return GATE_1_OUT_GPIO_Port;
}

uint16_t gate_to_gpio_pin(uint8_t gate)
{
    switch (gate)
    {
    case 0:
        return GATE_1_OUT_Pin;
    case 1:
        return GATE_2_OUT_Pin;
    case 2:
        return GATE_3_OUT_Pin;
    case 3:
        return GATE_4_OUT_Pin;
    default: break;
    }
    return GATE_1_OUT_Pin;
}


void write()
{
    switch (state.cfg.mode)
    {
    case MIDI_MODE_CHANNEL:
        int channel_idx;
        for (channel_idx = 0; channel_idx < 4; channel_idx++)
        {
            const MIDI_PROCESSOR_channel midi_channel = state.midi_processor_channel[channel_idx];
            GPIO_TypeDef* port = gate_to_gpio_port(midi_channel.channel);
            const uint16_t pin = gate_to_gpio_pin(midi_channel.channel);

            if (midi_channel.notes[0].is_on)
            {
                HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
            }
            else
            {
                HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
            }
        }

        MCP4728_FastWrite(state.cfg.cv_dac1,
                          state.midi_processor_channel[0].notes[0].cv,
                          state.midi_processor_channel[1].notes[0].cv,
                          state.midi_processor_channel[2].notes[0].cv,
                          state.midi_processor_channel[3].notes[0].cv);

        MCP4728_FastWrite(state.cfg.vel_dac2,
                          state.midi_processor_channel[0].notes[0].velocity,
                          state.midi_processor_channel[1].notes[0].velocity,
                          state.midi_processor_channel[2].notes[0].velocity,
                          state.midi_processor_channel[3].notes[0].velocity);

        MCP4728_FastWrite(state.cfg.mod_dac3,
                          state.midi_processor_channel[0].notes[0].mod,
                          state.midi_processor_channel[1].notes[0].mod,
                          state.midi_processor_channel[2].notes[0].mod,
                          state.midi_processor_channel[3].notes[0].mod);
        break;
    case MIDI_MODE_POLY:
        MIDI_PROCESSOR_channel midi_channel = state.midi_processor_channel[0];
        int note_idx;
        for (note_idx = 0; note_idx < 4; note_idx++)
        {
            GPIO_TypeDef* port = gate_to_gpio_port(note_idx);
            const uint16_t pin = gate_to_gpio_pin(note_idx);

            if (midi_channel.notes[note_idx].is_on)
            {
                HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
            }
            else
            {
                HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
            }
        }

        MCP4728_FastWrite(state.cfg.cv_dac1,
                          state.midi_processor_channel[0].notes[0].cv,
                          state.midi_processor_channel[0].notes[1].cv,
                          state.midi_processor_channel[0].notes[2].cv,
                          state.midi_processor_channel[0].notes[3].cv);

        MCP4728_FastWrite(state.cfg.vel_dac2,
                          state.midi_processor_channel[0].notes[0].velocity,
                          state.midi_processor_channel[0].notes[1].velocity,
                          state.midi_processor_channel[0].notes[2].velocity,
                          state.midi_processor_channel[0].notes[3].velocity);

        MCP4728_FastWrite(state.cfg.mod_dac3,
                          state.midi_processor_channel[0].notes[0].mod,
                          state.midi_processor_channel[0].notes[1].mod,
                          state.midi_processor_channel[0].notes[2].mod,
                          state.midi_processor_channel[0].notes[3].mod);
        break;
    case MIDI_MODE_SEQUENCE:
        break;
    default:
        break;
    }
}
