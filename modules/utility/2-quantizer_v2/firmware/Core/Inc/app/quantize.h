/*
******************************************************************************
* @file           : quantize.h
* @author         : Lennart Querter
* @brief          : Note-mask quantization core with hysteresis.
*                   Pure C, no HAL includes — unit-testable on the host
*                   (see firmware/test/).
*
*                   Pitch is expressed in cents relative to 0V (1V/oct,
*                   1 semitone = 100 cents). A "note" is the semitone index
*                   from 0V, so note 0 = 0V C, note 12 = 1V C, negative
*                   notes are below 0V.
******************************************************************************
*/

#ifndef __QUANTIZE_H
#define __QUANTIZE_H

#include <stdint.h>

// ±¼ semitone of hysteresis around the switching midpoint (development plan §5)
#define QUANTIZE_HYSTERESIS_CENTS 25

// sentinel for "no note quantized yet"
#define QUANTIZE_NO_NOTE INT16_MIN

// 12-bit note masks, bit 0 = C ... bit 11 = B (before root rotation)
#define QUANTIZE_SCALE_CHROMATIC  0x0FFF
#define QUANTIZE_SCALE_MAJOR      0x0AB5
#define QUANTIZE_SCALE_MINOR      0x05AD
#define QUANTIZE_SCALE_HARM_MINOR 0x09AD
#define QUANTIZE_SCALE_PENT_MAJ   0x0295
#define QUANTIZE_SCALE_PENT_MIN   0x04A9
#define QUANTIZE_SCALE_BLUES      0x04E9
#define QUANTIZE_SCALE_DORIAN     0x06AD
#define QUANTIZE_SCALE_MIXOLYDIAN 0x06B5
#define QUANTIZE_SCALE_WHOLE_TONE 0x0555
#define QUANTIZE_SCALE_OCTAVES    0x0001
#define QUANTIZE_SCALE_FIFTHS     0x0081

struct QUANTIZE_state
{
    uint16_t mask;      // allowed semitones, bit 0 = root
    int16_t root;       // 0-11, rotates the mask up in semitones
    int16_t last_note;  // last quantized note, QUANTIZE_NO_NOTE when none
};

// named preset list, shared by the UI (encoder browsing) and the CLI
struct QUANTIZE_scale
{
    const char* name;
    uint16_t mask;
};

#define QUANTIZE_SCALES_COUNT 12
extern const struct QUANTIZE_scale QUANTIZE_scales[QUANTIZE_SCALES_COUNT];

void QUANTIZE_init(struct QUANTIZE_state* q, uint16_t mask, int16_t root);
void QUANTIZE_set_mask(struct QUANTIZE_state* q, uint16_t mask);
void QUANTIZE_set_root(struct QUANTIZE_state* q, int16_t root);

/*
 * Quantize a pitch (in cents from 0V) to the nearest allowed note.
 * Applies hysteresis against the previously returned note so a slowly
 * moving input does not flutter at note boundaries.
 * Returns the note; with an empty mask the last note is held (0 if none yet).
 */
int16_t QUANTIZE_process(struct QUANTIZE_state* q, int32_t cents);

// note index -> cents (note * 100)
int32_t QUANTIZE_note_to_cents(int16_t note);

// 1 when the (root-rotated) mask allows this note
uint8_t QUANTIZE_note_allowed(const struct QUANTIZE_state* q, int16_t note);

#endif
