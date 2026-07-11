/*
******************************************************************************
* @file           : quantize.c
* @author         : Lennart Querter
* @brief          : Note-mask quantization core with hysteresis (pure C)
******************************************************************************
*/

#include "app/quantize.h"

const struct QUANTIZE_scale QUANTIZE_scales[QUANTIZE_SCALES_COUNT] = {
    {"chrom", QUANTIZE_SCALE_CHROMATIC}, {"major", QUANTIZE_SCALE_MAJOR},
    {"minor", QUANTIZE_SCALE_MINOR},     {"harm", QUANTIZE_SCALE_HARM_MINOR},
    {"pentmaj", QUANTIZE_SCALE_PENT_MAJ}, {"pentmin", QUANTIZE_SCALE_PENT_MIN},
    {"blues", QUANTIZE_SCALE_BLUES},     {"dorian", QUANTIZE_SCALE_DORIAN},
    {"mixo", QUANTIZE_SCALE_MIXOLYDIAN}, {"whole", QUANTIZE_SCALE_WHOLE_TONE},
    {"oct", QUANTIZE_SCALE_OCTAVES},     {"fifths", QUANTIZE_SCALE_FIFTHS},
};

void QUANTIZE_init(struct QUANTIZE_state* q, uint16_t mask, int16_t root)
{
    q->mask = mask & 0x0FFF;
    q->root = (int16_t)(root % 12);
    q->last_note = QUANTIZE_NO_NOTE;
}

void QUANTIZE_set_mask(struct QUANTIZE_state* q, uint16_t mask)
{
    q->mask = mask & 0x0FFF;
}

void QUANTIZE_set_root(struct QUANTIZE_state* q, int16_t root)
{
    q->root = (int16_t)(root % 12);
}

int32_t QUANTIZE_note_to_cents(int16_t note)
{
    return (int32_t)note * 100;
}

uint8_t QUANTIZE_note_allowed(const struct QUANTIZE_state* q, int16_t note)
{
    // ((note - root) mod 12) with a result in 0..11 for negative notes too
    int16_t degree = (int16_t)((note - q->root) % 12);
    if (degree < 0)
    {
        degree += 12;
    }
    return (q->mask >> degree) & 1u;
}

// floor division so negative cents round toward the note below, not toward zero
static int32_t floor_div(int32_t a, int32_t b)
{
    int32_t d = a / b;
    if ((a % b != 0) && ((a < 0) != (b < 0)))
    {
        d--;
    }
    return d;
}

static int32_t abs32(int32_t v)
{
    return v < 0 ? -v : v;
}

int16_t QUANTIZE_process(struct QUANTIZE_state* q, int32_t cents)
{
    if (q->mask == 0)
    {
        // nothing allowed: hold the last note
        if (q->last_note == QUANTIZE_NO_NOTE)
        {
            q->last_note = 0;
        }
        return q->last_note;
    }

    // nearest chromatic note, then search out ±1 octave for the nearest
    // allowed note (any non-empty mask has an allowed note within 12)
    int16_t nearest = (int16_t)floor_div(cents + 50, 100);
    int16_t best = QUANTIZE_NO_NOTE;
    int32_t best_dist = INT32_MAX;

    for (int16_t offset = -12; offset <= 12; offset++)
    {
        int16_t note = (int16_t)(nearest + offset);
        if (!QUANTIZE_note_allowed(q, note))
        {
            continue;
        }
        int32_t dist = abs32(cents - QUANTIZE_note_to_cents(note));
        // strict < keeps the lower note on an exact tie
        if (dist < best_dist)
        {
            best_dist = dist;
            best = note;
        }
    }

    /*
     * Hysteresis: only leave the previous note once the input has moved
     * past the midpoint between old and new note by an extra
     * QUANTIZE_HYSTERESIS_CENTS. Skipped when the previous note is no
     * longer allowed (mask/root edit) so it re-quantizes immediately.
     */
    if (q->last_note != QUANTIZE_NO_NOTE && best != q->last_note &&
        QUANTIZE_note_allowed(q, q->last_note))
    {
        int32_t dist_last = abs32(cents - QUANTIZE_note_to_cents(q->last_note));
        int32_t gap = abs32(QUANTIZE_note_to_cents(best) -
                            QUANTIZE_note_to_cents(q->last_note));
        if (dist_last <= gap / 2 + QUANTIZE_HYSTERESIS_CENTS)
        {
            best = q->last_note;
        }
    }

    q->last_note = best;
    return best;
}
