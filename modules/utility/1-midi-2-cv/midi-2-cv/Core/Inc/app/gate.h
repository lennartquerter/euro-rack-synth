//
// Created by Lennart Querter on 27.06.24.
//

#ifndef MIDI_2_CV_GATE_H
#define MIDI_2_CV_GATE_H

#include "midi.h"

#define GATE_CHANNEL_LENGTH (5)

typedef enum {
    GATE_OPEN,
    GATE_CLOSED,
} GATE_STATUS;

typedef struct {
    GPIO_TypeDef *pinGate;
    uint16_t pinNumber;

    GATE_STATUS status;
    uint8_t channelNumber;
} GATE_channel;

typedef struct {
    GATE_channel channels[GATE_CHANNEL_LENGTH];
} GATE_config;

void gate_init();

void gate_run(MIDI_event *event);

#endif //MIDI_2_CV_GATE_H
