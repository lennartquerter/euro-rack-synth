#include "app/midi_handler.h"
#include "app/mcp4728.h"
#include "libs/notes.h"
#include "main.h"

//******************
// Private Functions
//******************

I2C_HandleTypeDef *cv_i2c_ref;
I2C_HandleTypeDef *vel_i2c_ref;
I2C_HandleTypeDef *mod_i2c_ref;


void midi_handle_note_on(MIDI_event *midi_event);

void midi_handle_note_off(MIDI_event *midi_event);

void midi_handle_pitch(MIDI_event *midi_event);

//******************
// Implementation
//******************

void midi_handler_init(I2C_HandleTypeDef *cv_i2c, I2C_HandleTypeDef *vel_i2c, I2C_HandleTypeDef *mod_i2c) {
    cv_i2c_ref = cv_i2c;
    vel_i2c_ref = vel_i2c;
    mod_i2c_ref = mod_i2c;
    // CV output
    MCP4728_Init(cv_i2c);

    // VEL output
    MCP4728_Init(vel_i2c);

    // MOD output
    MCP4728_Init(mod_i2c);
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

        default:
            break;
    }
}

void midi_handle_note_on(MIDI_event *midi_event) {
    // check velocity (probably in WAIT_DATA_1)
    // if === 0 ???
    uint8_t midi_note = midi_event->data_byte[0];
    uint8_t midi_channel = midi_event->channel;

    uint8_t octave = OCTAVE_0;
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

    uint16_t value = (octave * OCT_VALUE) + (note * (OCT_VALUE / 12));

    uint8_t channel = MCP4728_CHANNEL_A;

    GPIO_TypeDef *port = GATE_1_OUT_GPIO_Port;
    uint16_t pin = GATE_1_OUT_Pin;

    if (midi_channel == 0) {
        channel = MCP4728_CHANNEL_A;
        port = GATE_1_OUT_GPIO_Port;
        pin = GATE_1_OUT_Pin;
    } else if (midi_channel == 1) {
        channel = MCP4728_CHANNEL_B;
        port = GATE_2_OUT_GPIO_Port;
        pin = GATE_2_OUT_Pin;
    } else if (midi_channel == 2) {
        channel = MCP4728_CHANNEL_C;
        port = GATE_3_OUT_GPIO_Port;
        pin = GATE_3_OUT_Pin;
    } else if (midi_channel == 3) {
        channel = MCP4728_CHANNEL_D;
        port = GATE_4_OUT_GPIO_Port;
        pin = GATE_4_OUT_Pin;
    } else {
        port = CH_16_OUT_GPIO_Port;
        pin = CH_16_OUT_Pin;
    }

    // -- Write all values
    MCP4728_Write_Voltage(cv_i2c_ref, channel, value);

    HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
    HAL_Delay(10);
}

void midi_handle_note_off(MIDI_event *midi_event) {
    // handle voltage setting
    // stop gate for channel
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
}

void midi_handle_pitch(MIDI_event *midi_event) {
    // handle voltage setting
    // stop gate for channel
}