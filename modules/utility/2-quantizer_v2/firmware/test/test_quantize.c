/*
******************************************************************************
* @file           : test_quantize.c
* @author         : Lennart Querter
* @brief          : Host-side unit tests for the quantize.c core (plain C,
*                   no HAL). Build & run with `make` in this directory.
******************************************************************************
*/

#include <stdio.h>

#include "app/quantize.h"

static int tests_run;
static int tests_failed;

#define ASSERT_EQ(actual, expected, message)                                   \
    do                                                                         \
    {                                                                          \
        tests_run++;                                                           \
        long a = (long)(actual);                                               \
        long e = (long)(expected);                                             \
        if (a != e)                                                            \
        {                                                                      \
            tests_failed++;                                                    \
            printf("FAIL %s:%d %s (got %ld, want %ld)\n", __FILE__, __LINE__,  \
                   message, a, e);                                             \
        }                                                                      \
    } while (0)

/*
 * Test factories — build quantizer states/inputs in one place instead of
 * ad-hoc setup in every test.
 */
static struct QUANTIZE_state make_quantizer(uint16_t mask, int16_t root)
{
    struct QUANTIZE_state q;
    QUANTIZE_init(&q, mask, root);
    return q;
}

static struct QUANTIZE_state make_settled_quantizer(uint16_t mask, int16_t root,
                                                    int32_t cents)
{
    struct QUANTIZE_state q = make_quantizer(mask, root);
    QUANTIZE_process(&q, cents); // seed last_note so hysteresis is active
    return q;
}

// ramp helper: count output note changes over an input sweep
static int count_note_changes(struct QUANTIZE_state* q, int32_t from_cents,
                              int32_t to_cents, int32_t step)
{
    int changes = 0;
    int16_t last = QUANTIZE_process(q, from_cents);
    for (int32_t cents = from_cents; (step > 0) ? (cents <= to_cents)
                                                : (cents >= to_cents);
         cents += step)
    {
        int16_t note = QUANTIZE_process(q, cents);
        if (note != last)
        {
            changes++;
            last = note;
        }
    }
    return changes;
}

static void test_chromatic_rounding(void)
{
    struct QUANTIZE_state q = make_quantizer(QUANTIZE_SCALE_CHROMATIC, 0);

    ASSERT_EQ(QUANTIZE_process(&q, 0), 0, "0 cents is note 0");

    q = make_quantizer(QUANTIZE_SCALE_CHROMATIC, 0);
    ASSERT_EQ(QUANTIZE_process(&q, 49), 0, "49 cents rounds down");

    q = make_quantizer(QUANTIZE_SCALE_CHROMATIC, 0);
    ASSERT_EQ(QUANTIZE_process(&q, 51), 1, "51 cents rounds up");

    q = make_quantizer(QUANTIZE_SCALE_CHROMATIC, 0);
    ASSERT_EQ(QUANTIZE_process(&q, 1200), 12, "1V is note 12");

    q = make_quantizer(QUANTIZE_SCALE_CHROMATIC, 0);
    ASSERT_EQ(QUANTIZE_process(&q, -1200), -12, "-1V is note -12");

    q = make_quantizer(QUANTIZE_SCALE_CHROMATIC, 0);
    ASSERT_EQ(QUANTIZE_process(&q, -151), -2, "negative rounding is symmetric");
}

static void test_hysteresis_band(void)
{
    // settled on note 0; midpoint to note 1 is 50, hysteresis adds 25
    struct QUANTIZE_state q =
        make_settled_quantizer(QUANTIZE_SCALE_CHROMATIC, 0, 0);

    ASSERT_EQ(QUANTIZE_process(&q, 74), 0, "74 cents still holds note 0");
    ASSERT_EQ(QUANTIZE_process(&q, 76), 1, "76 cents switches to note 1");
    // now settled on 1 (100 cents); going back needs < 25
    ASSERT_EQ(QUANTIZE_process(&q, 26), 1, "26 cents still holds note 1");
    ASSERT_EQ(QUANTIZE_process(&q, 24), 0, "24 cents drops back to note 0");
}

static void test_ramp_has_no_flutter(void)
{
    struct QUANTIZE_state q = make_quantizer(QUANTIZE_SCALE_CHROMATIC, 0);
    // slow 1-cent ramp over one octave: exactly one change per semitone
    ASSERT_EQ(count_note_changes(&q, 0, 1200, 1), 12, "12 changes going up");
    ASSERT_EQ(count_note_changes(&q, 1200, 0, -1), 12, "12 changes going down");
}

static void test_sparse_mask_nearest(void)
{
    // fifths: only C (0) and G (7) allowed
    struct QUANTIZE_state q = make_quantizer(QUANTIZE_SCALE_FIFTHS, 0);
    ASSERT_EQ(QUANTIZE_process(&q, 300), 0, "300 cents nearer to 0 than 700");

    q = make_quantizer(QUANTIZE_SCALE_FIFTHS, 0);
    ASSERT_EQ(QUANTIZE_process(&q, 400), 7, "400 cents nearer to 700");

    q = make_quantizer(QUANTIZE_SCALE_FIFTHS, 0);
    ASSERT_EQ(QUANTIZE_process(&q, 1000), 12, "1000 cents wraps to next C");

    // exact tie prefers the lower note
    q = make_quantizer(QUANTIZE_SCALE_FIFTHS, 0);
    ASSERT_EQ(QUANTIZE_process(&q, 350), 0, "tie at 350 goes low");
}

static void test_sparse_mask_hysteresis_scales_with_gap(void)
{
    // settled on 0, gap to 7 is 700: switch point is 350 + 25
    struct QUANTIZE_state q =
        make_settled_quantizer(QUANTIZE_SCALE_FIFTHS, 0, 0);
    ASSERT_EQ(QUANTIZE_process(&q, 374), 0, "374 still holds 0");
    ASSERT_EQ(QUANTIZE_process(&q, 376), 7, "376 switches to 7");
}

static void test_major_scale(void)
{
    // C major: D# (300 cents) sits exactly between D (200) and E (400)
    struct QUANTIZE_state q = make_quantizer(QUANTIZE_SCALE_MAJOR, 0);
    ASSERT_EQ(QUANTIZE_process(&q, 300), 2, "tie between D and E goes low");

    q = make_quantizer(QUANTIZE_SCALE_MAJOR, 0);
    ASSERT_EQ(QUANTIZE_process(&q, 310), 4, "310 is nearer E");

    q = make_quantizer(QUANTIZE_SCALE_MAJOR, 0);
    ASSERT_EQ(QUANTIZE_process(&q, 100), 0, "C# pulls to C on a tie");
}

static void test_root_rotation(void)
{
    // D major: F# (600 cents, note 6) allowed, F (500, note 5) not
    struct QUANTIZE_state q = make_quantizer(QUANTIZE_SCALE_MAJOR, 2);
    ASSERT_EQ(QUANTIZE_note_allowed(&q, 6), 1, "F# allowed in D major");
    ASSERT_EQ(QUANTIZE_note_allowed(&q, 5), 0, "F not allowed in D major");
    ASSERT_EQ(QUANTIZE_note_allowed(&q, -6), 1,
              "rotation holds for negative notes");

    ASSERT_EQ(QUANTIZE_process(&q, 500), 4,
              "F ties between E and F#, goes low");

    q = make_quantizer(QUANTIZE_SCALE_MAJOR, 2);
    ASSERT_EQ(QUANTIZE_process(&q, 520), 6, "520 cents lands on F#");
}

static void test_empty_mask_holds(void)
{
    struct QUANTIZE_state q =
        make_settled_quantizer(QUANTIZE_SCALE_CHROMATIC, 0, 700);
    QUANTIZE_set_mask(&q, 0);
    ASSERT_EQ(QUANTIZE_process(&q, 0), 7, "empty mask holds the last note");

    // empty mask with no history settles on 0
    q = make_quantizer(0, 0);
    ASSERT_EQ(QUANTIZE_process(&q, 700), 0, "empty mask without history is 0");
}

static void test_mask_edit_requantizes(void)
{
    struct QUANTIZE_state q =
        make_settled_quantizer(QUANTIZE_SCALE_CHROMATIC, 0, 100);
    ASSERT_EQ(q.last_note, 1, "settled on note 1");

    // remove C# from the mask: no hysteresis, jump straight to neighbour
    QUANTIZE_set_mask(&q, QUANTIZE_SCALE_CHROMATIC & ~(1u << 1));
    ASSERT_EQ(QUANTIZE_process(&q, 100), 0,
              "disallowed last note re-quantizes immediately");
}

static void test_note_to_cents(void)
{
    ASSERT_EQ(QUANTIZE_note_to_cents(0), 0, "note 0");
    ASSERT_EQ(QUANTIZE_note_to_cents(12), 1200, "note 12");
    ASSERT_EQ(QUANTIZE_note_to_cents(-7), -700, "negative note");
}

int main(void)
{
    test_chromatic_rounding();
    test_hysteresis_band();
    test_ramp_has_no_flutter();
    test_sparse_mask_nearest();
    test_sparse_mask_hysteresis_scales_with_gap();
    test_major_scale();
    test_root_rotation();
    test_empty_mask_holds();
    test_mask_edit_requantizes();
    test_note_to_cents();

    printf("%d tests, %d failed\n", tests_run, tests_failed);
    return tests_failed == 0 ? 0 : 1;
}
