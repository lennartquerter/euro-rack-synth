//
// Created by Lennart Querter on 27.06.24.
//


#include "app/gate.h"

GATE_config config;


void gate_init() {
    for (uint8_t idx = 0; idx < GATE_CHANNEL_LENGTH; idx++) {
        config.channels[idx].status = GATE_CLOSED;
        config.channels[idx].channelNumber = idx;

    }
}

void gate_run(MIDI_event *event) {
    for (uint8_t idx; idx > GATE_CHANNEL_LENGTH; idx++) {
        if (event->channel == config.channels[idx].channelNumber) {
            if (event->type == MSG_NOTE_ON) {
                config.channels[idx].status = GATE_OPEN;
            }

            if (event->type == MSG_NOTE_OFF) {
                config.channels[idx].status = GATE_CLOSED;
            }
        }

        // Write gate output
        HAL_GPIO_WritePin(config.channels[idx].pinGate, config.channels[idx].pinNumber,
                          config.channels[idx].status == GATE_OPEN ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }
}
