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

    uint16_t cv_dac_codes_per_volt;
    uint16_t vel_dac_calibrated;
    uint16_t mod_dac_calibrated;

    uint16_t pitch_bend_semitones;

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
uint16_t note_to_voltage(uint8_t midi_value, uint16_t calibrated_voltage);
uint16_t note_to_pitch_cv(uint8_t midi_note, uint16_t codes_per_volt);
uint16_t bent_pitch_cv(uint8_t midi_note, int16_t bend_codes, uint16_t codes_per_volt);
void refresh_channel_cv(uint8_t channel_idx);
bool is_event_for_us(const MIDI_event* midi_event);
int32_t gate_to_gpio_pin(uint8_t gate);

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
    state.cv_dac_codes_per_volt = DAC_CODES_PER_OUTPUT_VOLT;
    state.vel_dac_calibrated = DEFAULT_DAC_MAX_VALUE;
    state.mod_dac_calibrated = DEFAULT_DAC_MAX_VALUE;
    state.pitch_bend_semitones = DEFAULT_PITCH_BEND_SEMITONES;

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
    if (!is_event_for_us(midi_event))
    {
        return;
    }

    const uint8_t note = midi_event->data_byte[0];
    const uint8_t velocity = midi_event->data_byte[1];

    const uint16_t voltage_velocity = note_to_voltage(velocity, state.vel_dac_calibrated);
    if (state.cfg.mode == MIDI_MODE_CHANNEL)
    {
        state.midi_processor_channel[midi_event->channel].notes[0].is_on = true;
        state.midi_processor_channel[midi_event->channel].notes[0].note_value = note;
        state.midi_processor_channel[midi_event->channel].notes[0].cv = bent_pitch_cv(
            note,
            state.midi_processor_channel[midi_event->channel].pitch_bend_codes,
            state.cv_dac_codes_per_volt);
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

                state.midi_processor_channel[0].notes[note_idx].cv = bent_pitch_cv(
                    note,
                    state.midi_processor_channel[0].pitch_bend_codes,
                    state.cv_dac_codes_per_volt);
                state.midi_processor_channel[0].notes[note_idx].velocity = voltage_velocity;
                break;
            }
        }
    }
    else if (state.cfg.mode == MIDI_MODE_SEQUENCE)
    {
        // Take the slot the round-robin is pointing at, then advance it for the next note. Taking
        // before advancing means the first note after a reset lands on output 1, not output 2.
        const uint8_t target_channel = state.current_channel;
        state.current_channel = (uint8_t)((state.current_channel + 1) % NUMBER_OF_CHANNELS);

        state.midi_processor_channel[target_channel].notes[0].is_on = true;
        state.midi_processor_channel[target_channel].notes[0].note_value = note;

        state.midi_processor_channel[target_channel].notes[0].cv = bent_pitch_cv(
            note,
            state.midi_processor_channel[target_channel].pitch_bend_codes,
            state.cv_dac_codes_per_volt);
        state.midi_processor_channel[target_channel].notes[0].velocity = voltage_velocity;
    }
}

void MIDI_PROCESSOR_note_off(MIDI_event* midi_event)
{
    if (!is_event_for_us(midi_event))
    {
        return;
    }

    const uint8_t note = midi_event->data_byte[0];
    const uint8_t release_velocity = midi_event->data_byte[1];

    if (state.cfg.mode == MIDI_MODE_CHANNEL)
    {
        state.midi_processor_channel[midi_event->channel].notes[0].is_on = false;
        state.midi_processor_channel[midi_event->channel].notes[0].velocity =
            note_to_voltage(release_velocity, state.vel_dac_calibrated);
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
                state.midi_processor_channel[0].notes[note_idx].velocity =
                    note_to_voltage(release_velocity, state.vel_dac_calibrated);
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
    if (!is_event_for_us(midi_event))
    {
        return;
    }

    const uint8_t control_message = midi_event->data_byte[0];
    switch (control_message)
    {
    case CC_MOD_WHEEL:
    {
        // CC value is the second data byte; spread 0..127 over the full MOD span (0V..8.00V)
        const uint16_t mod_voltage = note_to_voltage(midi_event->data_byte[1], state.mod_dac_calibrated);

        if (state.cfg.mode == MIDI_MODE_CHANNEL)
        {
            for (int note_idx = 0; note_idx < NUMBER_OF_NOTES_PER_CHANNEL; note_idx++)
            {
                state.midi_processor_channel[midi_event->channel].notes[note_idx].mod = mod_voltage;
            }
        }
        else // one MIDI channel feeds every output in Sequence and Poly mode
        {
            for (int channel_idx = 0; channel_idx < NUMBER_OF_CHANNELS; channel_idx++)
            {
                for (int note_idx = 0; note_idx < NUMBER_OF_NOTES_PER_CHANNEL; note_idx++)
                {
                    state.midi_processor_channel[channel_idx].notes[note_idx].mod = mod_voltage;
                }
            }
        }
        break;
    }
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
    if (!is_event_for_us(midi_event))
    {
        return;
    }

    // Pitch bend is a 14-bit value sent as two 7-bit data bytes, LSB first: 0..16383 centred at 8192
    const uint16_t pitch_value = ((uint16_t)midi_event->data_byte[1] << 7)
        | (uint16_t)midi_event->data_byte[0];

    // -1.0 .. +1.0 of the configured bend range, converted to a signed pitch CV offset.
    // A semitone is codes_per_volt / 12 DAC codes, so a full bend is range_semitones of those.
    const float pitch_bend = get_pitch_bend(pitch_value);
    const int16_t bend_codes = (int16_t)(pitch_bend
        * (float)state.pitch_bend_semitones
        * (float)state.cv_dac_codes_per_volt
        / (float)SEMITONES_PER_OCTAVE);

    if (state.cfg.mode == MIDI_MODE_CHANNEL)
    {
        // Bend is per MIDI channel, and in this mode each MIDI channel owns one CV output
        state.midi_processor_channel[midi_event->channel].pitch_bend_codes = bend_codes;
        refresh_channel_cv(midi_event->channel);
    }
    else // Sequence and Poly both take everything from one MIDI channel, so all outputs bend
    {
        for (int channel_idx = 0; channel_idx < NUMBER_OF_CHANNELS; channel_idx++)
        {
            state.midi_processor_channel[channel_idx].pitch_bend_codes = bend_codes;
            refresh_channel_cv(channel_idx);
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
            const int32_t pin = gate_to_gpio_pin(midi_channel.channel);

            if (pin >= 0)
            {
                if (midi_channel.notes[0].is_on)
                {
                    HAL_GPIO_WritePin(GPIOB, (uint16_t)pin, GPIO_PIN_SET);
                }
                else
                {
                    HAL_GPIO_WritePin(GPIOB, (uint16_t)pin, GPIO_PIN_RESET);
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
            const int32_t pin = gate_to_gpio_pin(note_idx);

            if (pin >= 0)
            {
                if (state.midi_processor_channel[0].notes[note_idx].is_on)
                {
                    HAL_GPIO_WritePin(GPIOB, (uint16_t)pin, GPIO_PIN_SET);
                }
                else
                {
                    HAL_GPIO_WritePin(GPIOB, (uint16_t)pin, GPIO_PIN_RESET);
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
            const int32_t pin = gate_to_gpio_pin(midi_channel.channel);

            if (pin >= 0)
            {
                if (midi_channel.notes[0].is_on)
                {
                    HAL_GPIO_WritePin(GPIOB, (uint16_t)pin, GPIO_PIN_SET);
                }
                else
                {
                    HAL_GPIO_WritePin(GPIOB, (uint16_t)pin, GPIO_PIN_RESET);
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

// Maps a 7-bit MIDI value across the full DAC span. Correct for VEL and MOD, where the output is
// a proportional controller value; NOT correct for pitch, which needs note_to_pitch_cv below.
uint16_t note_to_voltage(uint8_t midi_value, uint16_t calibrated_voltage)
{
    // Ensure the MIDI value is within valid range (0-127)
    if (midi_value > MIDI_NOTES_LENGTH)
    {
        midi_value = MIDI_NOTES_LENGTH;
    }

    // Spread 0..127 over 0..calibrated_voltage DAC codes (4000 codes => 0V .. 8.00V at the jack)
    const uint16_t voltage = (uint32_t)midi_value * calibrated_voltage / MIDI_NOTES_LENGTH;

    return voltage;
}

// Maps a MIDI note to a 1V/oct pitch CV. One semitone is codes_per_volt / 12 DAC codes
// (500 / 12 = 41.667 by default), which is 83.33mV per semitone at the jack after the 4x gain.
// codes_per_volt is the calibration knob: trim it to compensate for Vref tolerance and the
// resistor tolerance of the gain stage.
uint16_t note_to_pitch_cv(uint8_t midi_note, uint16_t codes_per_volt)
{
    // Notes below the 0V reference note cannot be represented on a unipolar output
    if (midi_note <= CV_LOWEST_NOTE)
    {
        return 0;
    }

    if (midi_note > MIDI_NOTES_LENGTH)
    {
        midi_note = MIDI_NOTES_LENGTH;
    }

    const uint32_t semitones = (uint32_t)(midi_note - CV_LOWEST_NOTE);

    // Round to the nearest DAC code rather than truncating, so the error stays under half a code
    // instead of accumulating downwards over the range
    uint32_t code = (semitones * codes_per_volt + (SEMITONES_PER_OCTAVE / 2)) / SEMITONES_PER_OCTAVE;

    // The top of the keyboard runs past the DAC; clamp instead of wrapping
    if (code > DAC_MAX_CODE)
    {
        code = DAC_MAX_CODE;
    }

    return (uint16_t)code;
}

// Decides whether an incoming event belongs to this module, and -- just as importantly -- whether
// its channel nibble is safe to use as an index into midi_processor_channel[NUMBER_OF_CHANNELS].
//
//  Channel mode:  MIDI channels 1-4 drive outputs 1-4. Channels 5-16 are somebody else's traffic,
//                 and indexing the array with them would run off the end of the struct.
//  Poly mode:     MIDI channel 1 drives all four outputs, up to four keys at once.
//  Sequence mode: MIDI channel 1 drives the four outputs round-robin, one note each.
bool is_event_for_us(const MIDI_event* midi_event)
{
    if (state.cfg.mode == MIDI_MODE_CHANNEL)
    {
        return midi_event->channel < NUMBER_OF_CHANNELS;
    }

    return midi_event->channel == MIDI_INPUT_CHANNEL;
}

// Combines a note's 1V/oct position with the channel's current bend, clamped to the DAC
uint16_t bent_pitch_cv(uint8_t midi_note, int16_t bend_codes, uint16_t codes_per_volt)
{
    int32_t code = (int32_t)note_to_pitch_cv(midi_note, codes_per_volt) + (int32_t)bend_codes;

    if (code < 0)
    {
        code = 0;
    }
    else if (code > DAC_MAX_CODE)
    {
        code = DAC_MAX_CODE;
    }

    return (uint16_t)code;
}

// Re-renders the CV of every note on a channel from its stored note number. Called when the bend
// changes, so a held note glides instead of only taking effect on the next note-on.
void refresh_channel_cv(uint8_t channel_idx)
{
    for (int note_idx = 0; note_idx < NUMBER_OF_NOTES_PER_CHANNEL; note_idx++)
    {
        MIDI_PROCESSOR_note* note = &state.midi_processor_channel[channel_idx].notes[note_idx];

        note->cv = bent_pitch_cv(note->note_value,
                                 state.midi_processor_channel[channel_idx].pitch_bend_codes,
                                 state.cv_dac_codes_per_volt);
    }
}

// Returns the GPIO pin mask for a gate output, or -1 when the gate index has no pin. The return
// type must be signed: pin masks are uint16_t, so a uint16_t -1 would come back as 0xFFFF and
// compare as a valid pin.
int32_t gate_to_gpio_pin(uint8_t gate)
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
        state.midi_processor_channel[channel_idx].pitch_bend_codes = 0;

        for (int note_idx = 0; note_idx < NUMBER_OF_NOTES_PER_CHANNEL; note_idx++)
        {
            state.midi_processor_channel[channel_idx].notes[note_idx].is_on = false;
            state.midi_processor_channel[channel_idx].notes[note_idx].number = note_idx;
            state.midi_processor_channel[channel_idx].notes[note_idx].cv = 0;
            state.midi_processor_channel[channel_idx].notes[note_idx].velocity = 0;
            state.midi_processor_channel[channel_idx].notes[note_idx].mod = 0;
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
