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

    uint16_t cv_dac_calibrated;
    uint16_t vel_dac_calibrated;
    uint16_t mod_dac_calibrated;

    uint16_t pitch_bend_range;

    MIDI_PROCESSOR_channel midi_processor_channel[4];
};

static struct midi_processor_state state;

// DEFINE PRIVATE FUNCTIONS

void MIDI_PROCESSOR_note_on(MIDI_event* midi_event);
void MIDI_PROCESSOR_note_off(MIDI_event* midi_event);
void MIDI_PROCESSOR_cc(MIDI_event* midi_event);
void MIDI_PROCESSOR_pitch(MIDI_event* midi_event);

void reset_channels();
float get_pitch_bend(unsigned short pitch_value);
uint16_t note_to_voltage(uint8_t midi_note, uint16_t calibrated_voltage);
uint16_t gate_to_gpio_pin(uint8_t gate);

void write();

// PUBLIC FUNCTIONS

uint8_t MIDI_PROCESSOR_init(const struct MIDI_PROCESSOR_config* cfg)
{
    if (cfg == NULL)
    {
        return -1;
    }

    memset(&state, 0, sizeof(state));
    state.cfg = *cfg;

    // used to track channel mode
    state.current_channel = 0;
    state.cv_dac_calibrated = DEFAULT_DAC_MAX_VALUE;
    state.vel_dac_calibrated = DEFAULT_DAC_MAX_VALUE;
    state.mod_dac_calibrated = DEFAULT_DAC_MAX_VALUE;
    state.pitch_bend_range = DEFAULT_PITCH_BEND;

    reset_channels();

    return 0;
}

void MIDI_PROCESSOR_handle_event(MIDI_event* midi_event)
{
    switch (midi_event->type)
    {
    case MSG_NOTE_ON:
        MIDI_PROCESSOR_note_on(midi_event);
        break;

    case MSG_NOTE_OFF:
        MIDI_PROCESSOR_note_off(midi_event);
        break;

    case MSG_PITCH:
        MIDI_PROCESSOR_pitch(midi_event);
        break;
    case MSG_CC:
        MIDI_PROCESSOR_cc(midi_event);
        break;

    default:
        break;
    }

    write();
}

void MIDI_PROCESSOR_mode_changed()
{
    GPIO_PinState sw_mode_1 = HAL_GPIO_ReadPin(SW_MODE_1_GPIO_Port, SW_MODE_1_Pin);
    GPIO_PinState sw_mode_2 = HAL_GPIO_ReadPin(SW_MODE_2_GPIO_Port, SW_MODE_2_Pin);

    if (sw_mode_1 == GPIO_PIN_SET && sw_mode_2 == GPIO_PIN_SET)
    {
        state.cfg.mode = MIDI_MODE_POLY;
    }
    else if (sw_mode_2 == GPIO_PIN_SET)
    {
        state.cfg.mode = MIDI_MODE_CHANNEL;
    }
    else if (sw_mode_1 == GPIO_PIN_SET)
    {
        state.cfg.mode = MIDI_MODE_SEQUENCE;
    }

    // reset all channel configurations
    state.current_channel = 0x00;

    reset_channels();
}

// PRIVATE FUNCTIONS

// ********************
// Process each type of MIDI message
// This will change the internal state
// ********************

void MIDI_PROCESSOR_note_on(MIDI_event* midi_event)
{
    if (midi_event->channel == 0 && (state.cfg.mode == MIDI_MODE_POLY || state.cfg.mode == MIDI_MODE_SEQUENCE))
    {
        // we only accept messages on channel 1 for SEQ/POLU
        return;
    }

    const uint8_t note = midi_event->data_byte[0];
    const uint8_t velocity = midi_event->data_byte[1];

    const uint16_t voltage_pitch = note_to_voltage(note, state.cv_dac_calibrated);
    const uint16_t voltage_velocity = note_to_voltage(velocity, state.vel_dac_calibrated);
    if (state.cfg.mode == MIDI_MODE_CHANNEL)
    {
        state.midi_processor_channel[midi_event->channel].notes[0].is_on = true;
        state.midi_processor_channel[midi_event->channel].notes[0].note_value = note;
        state.midi_processor_channel[midi_event->channel].notes[0].cv = voltage_pitch;
        state.midi_processor_channel[midi_event->channel].notes[0].velocity = voltage_velocity;
    }
    else if (state.cfg.mode == MIDI_MODE_POLY)
    {
        // Maybe only use the available channels?
        for (int note_idx = 0; note_idx < NUMBER_OF_NOTES_PER_CHANNEL; note_idx++)
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
    else if (state.cfg.mode == MIDI_MODE_SEQUENCE)
    {
        // Increase the channel on every note played
        state.current_channel++;

        // only allow a maximum of 4 channels
        if (state.current_channel > 3)
        {
            state.current_channel = 0;
        }

        state.midi_processor_channel[state.current_channel].notes[0].is_on = true;
        state.midi_processor_channel[state.current_channel].notes[0].note_value = note;

        state.midi_processor_channel[state.current_channel].notes[0].cv = voltage_pitch;
        state.midi_processor_channel[state.current_channel].notes[0].velocity = voltage_velocity;
    }
}

void MIDI_PROCESSOR_note_off(MIDI_event* midi_event)
{
    if (midi_event->channel == 0 && (state.cfg.mode == MIDI_MODE_POLY || state.cfg.mode == MIDI_MODE_SEQUENCE))
    {
        // we only accept messages on channel 1 for SEQ/POLU
        return;
    }

    const uint8_t note = midi_event->data_byte[0];
    const uint8_t release_velocity = midi_event->data_byte[1];

    if (state.cfg.mode == MIDI_MODE_CHANNEL)
    {
        state.midi_processor_channel[midi_event->channel].notes[0].is_on = false;
        state.midi_processor_channel[midi_event->channel].notes[0].velocity = release_velocity;
    }
    else if (state.cfg.mode == MIDI_MODE_POLY)
    {
        // Maybe only use the available channels?
        for (int note_idx = 0; note_idx < NUMBER_OF_NOTES_PER_CHANNEL; note_idx++)
        {
            // find the first one that matches the note being played; turn it off; break;
            if (state.midi_processor_channel[0].notes[note_idx].note_value == note
                && state.midi_processor_channel[0].notes[note_idx].is_on == true)
            {
                state.midi_processor_channel[0].notes[note_idx].is_on = false;
                state.midi_processor_channel[0].notes[note_idx].velocity = release_velocity;
                break;
            }
        }
    }
    else if (state.cfg.mode == MIDI_MODE_SEQUENCE)
    {
        // find the channel where the note is playing
        // stop the note from playing
        for (int channel_idx = 0; channel_idx < NUMBER_OF_CHANNELS; channel_idx++)
        {
            if (state.midi_processor_channel[channel_idx].notes[0].note_value == note
                && state.midi_processor_channel[channel_idx].notes[0].is_on == true)
            {
                state.midi_processor_channel[channel_idx].notes[0].is_on = false;
                break;
            }
        }
    }
}

void MIDI_PROCESSOR_cc(MIDI_event* midi_event)
{
    const uint8_t control_message = midi_event->data_byte[0];
    switch (control_message)
    {
    case CC_ALL_NOTES_OFF:
        if (state.cfg.mode == MIDI_MODE_CHANNEL)
        {
            state.midi_processor_channel[midi_event->channel].notes[0].is_on = false;
        }
        else if (state.cfg.mode == MIDI_MODE_POLY)
        {
            for (int note_idx = 0; note_idx < 4; note_idx++)
            {
                state.midi_processor_channel[0].notes[note_idx].is_on = false;
            }
        }
        break;
    default:
        // No idea what the message is, so don't even try
        break;
    }
}

void MIDI_PROCESSOR_pitch(MIDI_event* midi_event)
{
    if (midi_event->channel == 0 && (state.cfg.mode == MIDI_MODE_POLY || state.cfg.mode == MIDI_MODE_SEQUENCE))
    {
        // we only accept messages on channel 1 for SEQ/POLU
        return;
    }

    uint16_t pitch_value = midi_event->data_byte[0];
    pitch_value <<= 8;
    pitch_value |= (uint16_t)midi_event->data_byte[1];

    float pitch_bend = get_pitch_bend(pitch_value);

    // The minimum voltage here will be 0, the maximum will be 1
    // output of 500mV --> 2V [Gain of 4 after DAC]
    // output of 0mV --> 0V [Gain of 4 after DAC]

    // Calculate pitch bend voltage:
    // Start from the center voltage (no bend)
    const uint16_t center_voltage = state.mod_dac_calibrated / 2;
    // Then add or subtract based on pitch bend value (-1 to 1)
    const int32_t pitch_bend_voltage = (int32_t)(pitch_bend * state.pitch_bend_range);
    // scaled by the pitch bend range
    uint16_t voltage_pitch = (center_voltage + pitch_bend_voltage);

    // Ensure voltage_pitch is within the valid range
    if (voltage_pitch > state.mod_dac_calibrated)
    {
        voltage_pitch = state.mod_dac_calibrated;
    }
    else if (voltage_pitch < 0)
    {
        voltage_pitch = 0;
    }

    if (state.cfg.mode == MIDI_MODE_CHANNEL)
    {
        state.midi_processor_channel[midi_event->channel].notes[0].mod = voltage_pitch;
    }
    else // both valid for Sequence and Poly Mode, All channels, with all notes will be set.
    {
        for (int channel_idx = 0; channel_idx < NUMBER_OF_CHANNELS; channel_idx++)
        {
            for (int note_idx = 0; note_idx < 4; note_idx++)
            {
                state.midi_processor_channel[channel_idx].notes[note_idx].mod = voltage_pitch;
            }
        }
    }
}

// ********************
// Render the output values
// Will consume the internal state and set the DAC/GPIO to the values
// ********************

void write()
{
    switch (state.cfg.mode)
    {
    case MIDI_MODE_CHANNEL:
        for (int channel_idx = 0; channel_idx < NUMBER_OF_CHANNELS; channel_idx++)
        {
            const MIDI_PROCESSOR_channel midi_channel = state.midi_processor_channel[channel_idx];
            const uint16_t pin = gate_to_gpio_pin(midi_channel.channel);

            if (pin > -1)
            {
                if (midi_channel.notes[0].is_on)
                {
                    HAL_GPIO_WritePin(GPIOB, pin, GPIO_PIN_SET);
                }
                else
                {
                    HAL_GPIO_WritePin(GPIOB, pin, GPIO_PIN_RESET);
                }
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
        for (int note_idx = 0; note_idx < NUMBER_OF_NOTES_PER_CHANNEL; note_idx++)
        {
            const uint16_t pin = gate_to_gpio_pin(note_idx);

            if (pin > -1)
            {
                if (state.midi_processor_channel[0].notes[note_idx].is_on)
                {
                    HAL_GPIO_WritePin(GPIOB, pin, GPIO_PIN_SET);
                }
                else
                {
                    HAL_GPIO_WritePin(GPIOB, pin, GPIO_PIN_RESET);
                }
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
        for (int channel_idx = 0; channel_idx < NUMBER_OF_CHANNELS; channel_idx++)
        {
            const MIDI_PROCESSOR_channel midi_channel = state.midi_processor_channel[channel_idx];
            const uint16_t pin = gate_to_gpio_pin(midi_channel.channel);

            if (pin > -1)
            {
                if (midi_channel.notes[0].is_on)
                {
                    HAL_GPIO_WritePin(GPIOB, pin, GPIO_PIN_SET);
                }
                else
                {
                    HAL_GPIO_WritePin(GPIOB, pin, GPIO_PIN_RESET);
                }
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
    default:
        break;
    }
}

// ********************
// Helper Functions
// ********************

uint16_t note_to_voltage(uint8_t midi_note, uint16_t calibrated_voltage)
{
    // Ensure MIDI note is within valid range (0-127)
    if (midi_note > MIDI_NOTES_LENGTH)
    {
        midi_note = MIDI_NOTES_LENGTH;
    }

    // Convert MIDI note to voltage (0-2000) [amplified by 4x gain]
    // 2000 / 127 ≈ 15.748, but we'll use 15.75 for integer math
    const uint16_t voltage = (uint32_t)midi_note * calibrated_voltage / MIDI_NOTES_LENGTH;

    return voltage;
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
    return -1;
}

void reset_channels()
{
    for (int channel_idx = 0; channel_idx < NUMBER_OF_CHANNELS; channel_idx++)
    {
        state.midi_processor_channel[channel_idx].channel = channel_idx;

        for (int note_idx = 0; note_idx < NUMBER_OF_NOTES_PER_CHANNEL; note_idx++)
        {
            state.midi_processor_channel[channel_idx].notes[note_idx].is_on = false;
            state.midi_processor_channel[channel_idx].notes[note_idx].number = note_idx;
            state.midi_processor_channel[channel_idx].notes[note_idx].cv = 0;
            state.midi_processor_channel[channel_idx].notes[note_idx].velocity = 0;
            state.midi_processor_channel[channel_idx].notes[note_idx].mod = 250;
        }
    }
}

float get_pitch_bend(unsigned short pitch_value)
{
    // Convert to signed value centered at 0
    int signed_pitch = (int)pitch_value - PITCH_CENTER;

    // Calculate pitch bend as a float from -1.0 to +1.0
    float pitch_bend = (float)signed_pitch / PITCH_RANGE;

    return pitch_bend;
}
