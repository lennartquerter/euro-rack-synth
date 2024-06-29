#include "app/midi_handler.h"

//******************
// Private Functions
//******************

void midi_handle_note_on(MIDI_event *midi_event);

void midi_handle_note_off(MIDI_event *midi_event);

void midi_handle_pitch(MIDI_event *midi_event);

//******************
// Implementation
//******************

void midi_handler_run(MIDI_event *midi_event) {
    switch (midi_event->type) {
        case MSG_NOTE_ON:
            midi_handle_note_on(midi_event);
            break;

        case MSG_NOTE_OFF:
            midi_handle_note_off(midi_event);
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
    // if === 0
    // midi_handle_note_off()
    // handle voltage setting
    // start gate for channel

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
//    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
}

void midi_handle_note_off(MIDI_event *midi_event) {
    // handle voltage setting
    // stop gate for channel
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
//    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
}

void midi_handle_pitch(MIDI_event *midi_event) {
    // handle voltage setting
    // stop gate for channel
}