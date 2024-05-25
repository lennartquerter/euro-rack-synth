//
// Created by Lennart Querter on 13.05.24.
//

#include <memory.h>
#include "notes.h"

struct notes piano;
double set_voltage_divider = 0.08333333333;

int32_t NOTES_init() {
    memset(&piano, 0, sizeof(piano));

    piano.keys[NOTE_0_C].voltage = 0;
    piano.keys[NOTE_0_C].offset = 0;
    piano.keys[NOTE_0_C].note_name = NOTE_0_C;

    piano.keys[NOTE_0_C_SHARP].voltage = set_voltage_divider * 1;
    piano.keys[NOTE_0_C_SHARP].offset = 0;
    piano.keys[NOTE_0_C_SHARP].note_name = NOTE_0_C_SHARP;

    piano.keys[NOTE_0_D].voltage = set_voltage_divider * 2;
    piano.keys[NOTE_0_D].offset = 0;
    piano.keys[NOTE_0_D].note_name = NOTE_0_D;

    piano.keys[NOTE_0_D_SHARP].voltage = set_voltage_divider * 3;
    piano.keys[NOTE_0_D_SHARP].offset = 0;
    piano.keys[NOTE_0_D_SHARP].note_name = NOTE_0_D_SHARP;

    piano.keys[NOTE_0_E].voltage = set_voltage_divider * 4;
    piano.keys[NOTE_0_E].offset = 0;
    piano.keys[NOTE_0_E].note_name = NOTE_0_E;

    piano.keys[NOTE_0_F].voltage = set_voltage_divider * 5;
    piano.keys[NOTE_0_F].offset = 0;
    piano.keys[NOTE_0_F].note_name = NOTE_0_F;

    piano.keys[NOTE_0_F_SHARP].voltage = set_voltage_divider * 6;
    piano.keys[NOTE_0_F_SHARP].offset = 0;
    piano.keys[NOTE_0_F_SHARP].note_name = NOTE_0_F_SHARP;

    piano.keys[NOTE_0_G].voltage = set_voltage_divider * 7;
    piano.keys[NOTE_0_G].offset = 0;
    piano.keys[NOTE_0_G].note_name = NOTE_0_G;

    piano.keys[NOTE_0_G_SHARP].voltage = set_voltage_divider * 8;
    piano.keys[NOTE_0_G_SHARP].offset = 0;
    piano.keys[NOTE_0_G_SHARP].note_name = NOTE_0_G_SHARP;

    piano.keys[NOTE_0_A].voltage = set_voltage_divider * 9;
    piano.keys[NOTE_0_A].offset = 0;
    piano.keys[NOTE_0_A].note_name = NOTE_0_A;

    piano.keys[NOTE_0_A_SHARP].voltage = set_voltage_divider * 10;
    piano.keys[NOTE_0_A_SHARP].offset = 0;
    piano.keys[NOTE_0_A_SHARP].note_name = NOTE_0_A_SHARP;

    piano.keys[NOTE_0_B].voltage = set_voltage_divider * 11;
    piano.keys[NOTE_0_B].offset = 0;
    piano.keys[NOTE_0_B].note_name = NOTE_0_B;

    // --------- 2d octave

    piano.keys[NOTE_1_C].voltage = 0;
    piano.keys[NOTE_1_C].offset = 1;
    piano.keys[NOTE_1_C].note_name = NOTE_1_C;

    piano.keys[NOTE_1_C_SHARP].voltage = set_voltage_divider * 1;
    piano.keys[NOTE_1_C_SHARP].offset = 1;
    piano.keys[NOTE_1_C_SHARP].note_name = NOTE_1_C_SHARP;

    piano.keys[NOTE_1_D].voltage = set_voltage_divider * 2;
    piano.keys[NOTE_1_D].offset = 1;
    piano.keys[NOTE_1_D].note_name = NOTE_1_D;

    piano.keys[NOTE_1_D_SHARP].voltage = set_voltage_divider * 3;
    piano.keys[NOTE_1_D_SHARP].offset = 1;
    piano.keys[NOTE_1_D_SHARP].note_name = NOTE_1_D_SHARP;

    piano.keys[NOTE_1_E].voltage = set_voltage_divider * 4;
    piano.keys[NOTE_1_E].offset = 1;
    piano.keys[NOTE_1_E].note_name = NOTE_1_E;

    piano.keys[NOTE_1_F].voltage = set_voltage_divider * 5;
    piano.keys[NOTE_1_F].offset = 1;
    piano.keys[NOTE_1_F].note_name = NOTE_1_F;

    piano.keys[NOTE_1_F_SHARP].voltage = set_voltage_divider * 6;
    piano.keys[NOTE_1_F_SHARP].offset = 1;
    piano.keys[NOTE_1_F_SHARP].note_name = NOTE_1_F_SHARP;

    piano.keys[NOTE_1_G].voltage = set_voltage_divider * 7;
    piano.keys[NOTE_1_G].offset = 1;
    piano.keys[NOTE_1_G].note_name = NOTE_1_G;

    piano.keys[NOTE_1_G_SHARP].voltage = set_voltage_divider * 8;
    piano.keys[NOTE_1_G_SHARP].offset = 1;
    piano.keys[NOTE_1_G_SHARP].note_name = NOTE_1_G_SHARP;

    piano.keys[NOTE_1_A].voltage = set_voltage_divider * 9;
    piano.keys[NOTE_1_A].offset = 1;
    piano.keys[NOTE_1_A].note_name = NOTE_1_A;

    piano.keys[NOTE_1_A_SHARP].voltage = set_voltage_divider * 10;
    piano.keys[NOTE_1_A_SHARP].offset = 1;
    piano.keys[NOTE_1_A_SHARP].note_name = NOTE_1_A_SHARP;

    piano.keys[NOTE_1_B].voltage = set_voltage_divider * 11;
    piano.keys[NOTE_1_B].offset = 1;
    piano.keys[NOTE_1_B].note_name = NOTE_1_B;

    // --------- 3rd octave

    piano.keys[NOTE_2_C].voltage = 0;
    piano.keys[NOTE_2_C].offset = 2;
    piano.keys[NOTE_2_C].note_name = NOTE_2_C;

    piano.keys[NOTE_2_C_SHARP].voltage = set_voltage_divider * 1;
    piano.keys[NOTE_2_C_SHARP].offset = 2;
    piano.keys[NOTE_2_C_SHARP].note_name = NOTE_2_C_SHARP;

    piano.keys[NOTE_2_D].voltage = set_voltage_divider * 2;
    piano.keys[NOTE_2_D].offset = 2;
    piano.keys[NOTE_2_D].note_name = NOTE_2_D;

    piano.keys[NOTE_2_D_SHARP].voltage = set_voltage_divider * 3;
    piano.keys[NOTE_2_D_SHARP].offset = 2;
    piano.keys[NOTE_2_D_SHARP].note_name = NOTE_2_D_SHARP;

    piano.keys[NOTE_2_E].voltage = set_voltage_divider * 4;
    piano.keys[NOTE_2_E].offset = 2;
    piano.keys[NOTE_2_E].note_name = NOTE_2_E;

    piano.keys[NOTE_2_F].voltage = set_voltage_divider * 5;
    piano.keys[NOTE_2_F].offset = 2;
    piano.keys[NOTE_2_F].note_name = NOTE_2_F;

    piano.keys[NOTE_2_F_SHARP].voltage = set_voltage_divider * 6;
    piano.keys[NOTE_2_F_SHARP].offset = 2;
    piano.keys[NOTE_2_F_SHARP].note_name = NOTE_2_F_SHARP;

    piano.keys[NOTE_2_G].voltage = set_voltage_divider * 7;
    piano.keys[NOTE_2_G].offset = 2;
    piano.keys[NOTE_2_G].note_name = NOTE_2_G;

    piano.keys[NOTE_2_G_SHARP].voltage = set_voltage_divider * 8;
    piano.keys[NOTE_2_G_SHARP].offset = 2;
    piano.keys[NOTE_2_G_SHARP].note_name = NOTE_2_G_SHARP;

    piano.keys[NOTE_2_A].voltage = set_voltage_divider * 9;
    piano.keys[NOTE_2_A].offset = 2;
    piano.keys[NOTE_2_A].note_name = NOTE_2_A;

    piano.keys[NOTE_2_A_SHARP].voltage = set_voltage_divider * 10;
    piano.keys[NOTE_2_A_SHARP].offset = 2;
    piano.keys[NOTE_2_A_SHARP].note_name = NOTE_2_A_SHARP;

    piano.keys[NOTE_2_B].voltage = set_voltage_divider * 11;
    piano.keys[NOTE_2_B].offset = 2;
    piano.keys[NOTE_2_B].note_name = NOTE_2_B;

    // --------- 4th octave

    piano.keys[NOTE_3_C].voltage = 0;
    piano.keys[NOTE_3_C].offset = 3;
    piano.keys[NOTE_3_C].note_name = NOTE_3_C;

    piano.keys[NOTE_3_C_SHARP].voltage = set_voltage_divider * 1;
    piano.keys[NOTE_3_C_SHARP].offset = 3;
    piano.keys[NOTE_3_C_SHARP].note_name = NOTE_3_C_SHARP;

    piano.keys[NOTE_3_D].voltage = set_voltage_divider * 2;
    piano.keys[NOTE_3_D].offset = 3;
    piano.keys[NOTE_3_D].note_name = NOTE_3_D;

    piano.keys[NOTE_3_D_SHARP].voltage = set_voltage_divider * 3;
    piano.keys[NOTE_3_D_SHARP].offset = 3;
    piano.keys[NOTE_3_D_SHARP].note_name = NOTE_3_D_SHARP;

    piano.keys[NOTE_3_E].voltage = set_voltage_divider * 4;
    piano.keys[NOTE_3_E].offset = 3;
    piano.keys[NOTE_3_E].note_name = NOTE_3_E;

    piano.keys[NOTE_3_F].voltage = set_voltage_divider * 5;
    piano.keys[NOTE_3_F].offset = 3;
    piano.keys[NOTE_3_F].note_name = NOTE_3_F;

    piano.keys[NOTE_3_F_SHARP].voltage = set_voltage_divider * 6;
    piano.keys[NOTE_3_F_SHARP].offset = 3;
    piano.keys[NOTE_3_F_SHARP].note_name = NOTE_3_F_SHARP;

    piano.keys[NOTE_3_G].voltage = set_voltage_divider * 7;
    piano.keys[NOTE_3_G].offset = 3;
    piano.keys[NOTE_3_G].note_name = NOTE_3_G;

    piano.keys[NOTE_3_G_SHARP].voltage = set_voltage_divider * 8;
    piano.keys[NOTE_3_G_SHARP].offset = 3;
    piano.keys[NOTE_3_G_SHARP].note_name = NOTE_3_G_SHARP;

    piano.keys[NOTE_3_A].voltage = set_voltage_divider * 9;
    piano.keys[NOTE_3_A].offset = 3;
    piano.keys[NOTE_3_A].note_name = NOTE_3_A;

    piano.keys[NOTE_3_A_SHARP].voltage = set_voltage_divider * 10;
    piano.keys[NOTE_3_A_SHARP].offset = 3;
    piano.keys[NOTE_3_A_SHARP].note_name = NOTE_3_A_SHARP;

    piano.keys[NOTE_3_B].voltage = set_voltage_divider * 11;
    piano.keys[NOTE_3_B].offset = 3;
    piano.keys[NOTE_3_B].note_name = NOTE_3_B;

    return 0;
}

void NOTES_GetNote(enum note_name noteName, struct note *note) {
    *note = piano.keys[noteName];
}

double NOTES_GetVoltage(enum note_name noteName) {
    double volt_value;
    volt_value = piano.keys[noteName].offset + piano.keys[noteName].voltage;

    return volt_value;
}