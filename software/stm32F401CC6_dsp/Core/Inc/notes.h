//
// Created by Lennart Querter on 13.05.24.
//

#ifndef STM32F401CC6_DSP_NOTES_H
#define STM32F401CC6_DSP_NOTES_H

#include "stm32f4xx_hal.h"

enum note_name {
    NOTE_0_C = 0,
    NOTE_0_C_SHARP,
    NOTE_0_D,
    NOTE_0_D_SHARP,
    NOTE_0_E,
    NOTE_0_F,
    NOTE_0_F_SHARP,
    NOTE_0_G,
    NOTE_0_G_SHARP,
    NOTE_0_A,
    NOTE_0_A_SHARP,
    NOTE_0_B,

    NOTE_1_C,
    NOTE_1_C_SHARP,
    NOTE_1_D,
    NOTE_1_D_SHARP,
    NOTE_1_E,
    NOTE_1_F,
    NOTE_1_F_SHARP,
    NOTE_1_G,
    NOTE_1_G_SHARP,
    NOTE_1_A,
    NOTE_1_A_SHARP,
    NOTE_1_B,

    NOTE_2_C,
    NOTE_2_C_SHARP,
    NOTE_2_D,
    NOTE_2_D_SHARP,
    NOTE_2_E,
    NOTE_2_F,
    NOTE_2_F_SHARP,
    NOTE_2_G,
    NOTE_2_G_SHARP,
    NOTE_2_A,
    NOTE_2_A_SHARP,
    NOTE_2_B,

    NOTE_3_C,
    NOTE_3_C_SHARP,
    NOTE_3_D,
    NOTE_3_D_SHARP,
    NOTE_3_E,
    NOTE_3_F,
    NOTE_3_F_SHARP,
    NOTE_3_G,
    NOTE_3_G_SHARP,
    NOTE_3_A,
    NOTE_3_A_SHARP,
    NOTE_3_B,
};

struct note {
    double voltage;
    uint16_t offset;
    enum note_name note_name;
};

struct notes {
    struct note keys[88];
};

int32_t NOTES_init();

void NOTES_GetNote(enum note_name noteName, struct note *note);
double NOTES_GetVoltage(enum note_name noteName);

#endif //STM32F401CC6_DSP_NOTES_H
