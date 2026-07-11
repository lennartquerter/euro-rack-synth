/*
******************************************************************************
* @file           : test_seq.c
* @author         : Lennart Querter
* @brief          : Host-side unit tests for the sequencer engine (seq.c).
*                   Uses tiny locally-built sequences — the engine is
*                   bank-agnostic, so the real generated bank is not
*                   linked here on purpose. Build & run with `make`.
******************************************************************************
*/

#include <stdio.h>

#include "app/quantize.h" // scale mask constants
#include "app/seq.h"

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
 * Test factories.
 */

struct test_sequence
{
    struct SEQ_step steps[SEQ_PHRASE_STEPS];
    struct SEQ_sequence seq;
};

// all-rest phrase to place events into
static void make_sequence(struct test_sequence* t, const char* name)
{
    for (int i = 0; i < SEQ_PHRASE_STEPS; i++)
    {
        t->steps[i].note = SEQ_STEP_REST;
        t->steps[i].flags = 0;
    }
    t->seq.name = name;
    t->seq.step_count = SEQ_PHRASE_STEPS;
    t->seq.steps = t->steps;
}

static void put_note(struct test_sequence* t, int at, int8_t note)
{
    t->steps[at].note = note;
    t->steps[at].flags = SEQ_FLAG_GATE;
}

static void put_tie(struct test_sequence* t, int at, int8_t note)
{
    t->steps[at].note = note;
    t->steps[at].flags = SEQ_FLAG_GATE | SEQ_FLAG_TIE;
}

// every step gated with the same note — for "which sequence plays" tests
static void make_flat_sequence(struct test_sequence* t, const char* name,
                               int8_t note)
{
    make_sequence(t, name);
    for (int i = 0; i < SEQ_PHRASE_STEPS; i++)
    {
        put_note(t, i, note);
    }
}

// note == step index — for position/direction tests (keep length <= 120)
static void make_ramp_sequence(struct test_sequence* t, const char* name)
{
    make_sequence(t, name);
    for (int i = 0; i < SEQ_PHRASE_STEPS; i++)
    {
        put_note(t, i, (int8_t)(i <= SEQ_NOTE_MAX ? i : SEQ_NOTE_MAX));
    }
}

static struct SEQ_engine make_engine(const struct SEQ_sequence* a,
                                     const struct SEQ_sequence* b)
{
    struct SEQ_engine engine;
    SEQ_init(&engine, a, b, 42);
    return engine;
}

static struct SEQ_lane_output advance_lane_a(struct SEQ_engine* engine)
{
    struct SEQ_lane_output out[SEQ_LANES];
    SEQ_advance(engine, out);
    return out[0];
}

static void advance_pulses(struct SEQ_engine* engine, int count)
{
    struct SEQ_lane_output out[SEQ_LANES];
    while (count-- > 0)
    {
        SEQ_advance(engine, out);
    }
}

/*
 * Tests.
 */

static void test_first_pulse_plays_step0(void)
{
    struct test_sequence t;
    make_sequence(&t, "t");
    put_note(&t, 0, 60);
    struct SEQ_engine e = make_engine(&t.seq, &t.seq);

    struct SEQ_lane_output out = advance_lane_a(&e);
    ASSERT_EQ(out.note, 60, "step 0 note plays on the first pulse");
    ASSERT_EQ(out.gate, 1, "gate high");
    ASSERT_EQ(out.retrigger, 1, "onset retriggers");
    ASSERT_EQ(SEQ_position(&e), 0, "position is step 0");
}

static void test_rest_is_silent(void)
{
    struct test_sequence t;
    make_sequence(&t, "t");
    put_note(&t, 0, 60); // step 1 is a rest
    struct SEQ_engine e = make_engine(&t.seq, &t.seq);

    advance_pulses(&e, 1);
    struct SEQ_lane_output out = advance_lane_a(&e);
    ASSERT_EQ(out.gate, 0, "rest step has no gate");
    ASSERT_EQ(out.note, SEQ_STEP_REST, "rest step has no note");
}

static void test_tie_holds_without_retrigger(void)
{
    struct test_sequence t;
    make_sequence(&t, "t");
    put_note(&t, 0, 60);
    put_tie(&t, 1, 60);
    put_tie(&t, 2, 62); // legato pitch change
    struct SEQ_engine e = make_engine(&t.seq, &t.seq);

    advance_pulses(&e, 1);
    struct SEQ_lane_output out = advance_lane_a(&e);
    ASSERT_EQ(out.gate, 1, "tied step keeps the gate high");
    ASSERT_EQ(out.retrigger, 0, "tied step does not retrigger");

    out = advance_lane_a(&e);
    ASSERT_EQ(out.note, 62, "legato pitch change under the tie");
    ASSERT_EQ(out.retrigger, 0, "legato does not retrigger");
}

static void test_musical_time_counters(void)
{
    struct test_sequence t;
    make_sequence(&t, "t");
    struct SEQ_engine e = make_engine(&t.seq, &t.seq);

    advance_pulses(&e, 1); // pulse 0
    ASSERT_EQ(SEQ_beat(&e), 0, "starts at beat 0");
    ASSERT_EQ(SEQ_bar(&e), 0, "starts at bar 0");

    advance_pulses(&e, SEQ_SUBSTEPS_PER_BEAT); // pulse 4
    ASSERT_EQ(SEQ_beat(&e), 1, "beat 1 after one beat of pulses");

    struct SEQ_engine e2 = make_engine(&t.seq, &t.seq);
    advance_pulses(&e2, SEQ_BAR_STEPS + 1); // pulse == bar boundary
    ASSERT_EQ(SEQ_bar(&e2), 1, "bar 1 after one bar of pulses");
    ASSERT_EQ(SEQ_beat(&e2), 0, "beat wraps at the bar");

    struct SEQ_engine e3 = make_engine(&t.seq, &t.seq);
    advance_pulses(&e3, SEQ_PHRASE_STEPS + 1); // one full phrase + 1
    ASSERT_EQ(SEQ_bar(&e3), 0, "bar wraps at the phrase");
    ASSERT_EQ(SEQ_position(&e3), 0, "position wraps at the phrase");
}

static void test_transpose_octave_and_clamp(void)
{
    struct test_sequence t;
    make_flat_sequence(&t, "t", 60);
    struct SEQ_engine e = make_engine(&t.seq, &t.seq);

    SEQ_arm_transpose(&e, SEQ_LANE_A, 7, SEQ_QUANT_NOW);
    struct SEQ_lane_output out = advance_lane_a(&e);
    ASSERT_EQ(out.note, 67, "transpose applied");

    SEQ_arm_octave(&e, SEQ_LANE_A, 1, SEQ_QUANT_NOW);
    out = advance_lane_a(&e);
    ASSERT_EQ(out.note, 79, "octave adds 12 semitones");

    SEQ_arm_transpose(&e, SEQ_LANE_A, 24, SEQ_QUANT_NOW);
    SEQ_arm_octave(&e, SEQ_LANE_A, 3, SEQ_QUANT_NOW);
    out = advance_lane_a(&e);
    ASSERT_EQ(out.note, SEQ_NOTE_MAX, "output clamps to the DAC range");

    SEQ_arm_transpose(&e, SEQ_LANE_A, -100, SEQ_QUANT_NOW); // clamps to -24
    SEQ_arm_octave(&e, SEQ_LANE_A, -3, SEQ_QUANT_NOW);
    out = advance_lane_a(&e);
    ASSERT_EQ(out.note, 0, "60 - 24 - 36 clamps at 0");
}

static void test_scale_quantize(void)
{
    struct test_sequence t;
    make_flat_sequence(&t, "t", 3); // D#
    struct SEQ_engine e = make_engine(&t.seq, &t.seq);

    SEQ_set_scale(&e, SEQ_LANE_A, QUANTIZE_SCALE_FIFTHS, 0); // C and G only
    struct SEQ_lane_output out = advance_lane_a(&e);
    ASSERT_EQ(out.note, 0, "note 3 snaps down to C (nearest, tie goes low)");

    struct test_sequence t2;
    make_flat_sequence(&t2, "t2", 5); // F
    struct SEQ_engine e2 = make_engine(&t2.seq, &t2.seq);
    SEQ_set_scale(&e2, SEQ_LANE_A, QUANTIZE_SCALE_FIFTHS, 0);
    out = advance_lane_a(&e2);
    ASSERT_EQ(out.note, 7, "note 5 snaps up to G");
}

static void test_mute(void)
{
    struct test_sequence t;
    make_flat_sequence(&t, "t", 60);
    struct SEQ_engine e = make_engine(&t.seq, &t.seq);

    SEQ_arm_mute(&e, SEQ_LANE_A, 1, SEQ_QUANT_NOW);
    struct SEQ_lane_output out = advance_lane_a(&e);
    ASSERT_EQ(out.gate, 0, "muted lane has no gate");

    SEQ_arm_mute(&e, SEQ_LANE_A, 0, SEQ_QUANT_NOW);
    out = advance_lane_a(&e);
    ASSERT_EQ(out.gate, 1, "unmuted lane plays again");
}

static void test_arm_sequence_waits_for_phrase(void)
{
    struct test_sequence old_seq, new_seq;
    make_flat_sequence(&old_seq, "old", 10);
    make_flat_sequence(&new_seq, "new", 20);
    struct SEQ_engine e = make_engine(&old_seq.seq, &old_seq.seq);

    advance_pulses(&e, 5);
    SEQ_arm_sequence(&e, SEQ_LANE_A, &new_seq.seq, SEQ_QUANT_PHRASE);
    ASSERT_EQ(SEQ_sequence_armed(&e, SEQ_LANE_A), 1, "switch is armed");

    // pulses 5..127 still play the old sequence
    struct SEQ_lane_output out = advance_lane_a(&e); // pulse 5
    ASSERT_EQ(out.note, 10, "old sequence keeps playing after arming");
    advance_pulses(&e, SEQ_PHRASE_STEPS - 7); // up to pulse 126
    out = advance_lane_a(&e);                 // pulse 127: last of the phrase
    ASSERT_EQ(out.note, 10, "old sequence plays to the end of the phrase");

    out = advance_lane_a(&e); // pulse 128: phrase boundary
    ASSERT_EQ(out.note, 20, "new sequence starts at the phrase top");
    ASSERT_EQ(SEQ_position(&e), 0, "phrase boundary is step 0");
    ASSERT_EQ(SEQ_sequence_armed(&e, SEQ_LANE_A), 0, "arming consumed");
}

static void test_arm_beat_and_bar_boundaries(void)
{
    struct test_sequence old_seq, new_seq;
    make_flat_sequence(&old_seq, "old", 10);
    make_flat_sequence(&new_seq, "new", 20);

    struct SEQ_engine e = make_engine(&old_seq.seq, &old_seq.seq);
    advance_pulses(&e, 2); // mid-beat (pulse 1)
    SEQ_arm_sequence(&e, SEQ_LANE_A, &new_seq.seq, SEQ_QUANT_BEAT);
    struct SEQ_lane_output out = advance_lane_a(&e); // pulse 2
    ASSERT_EQ(out.note, 10, "beat-armed change waits mid-beat");
    advance_pulses(&e, 1);       // pulse 3
    out = advance_lane_a(&e);    // pulse 4 = beat boundary
    ASSERT_EQ(out.note, 20, "beat-armed change applies on the next beat");

    struct SEQ_engine e2 = make_engine(&old_seq.seq, &old_seq.seq);
    advance_pulses(&e2, 6); // pulse 5
    SEQ_arm_sequence(&e2, SEQ_LANE_A, &new_seq.seq, SEQ_QUANT_BAR);
    advance_pulses(&e2, SEQ_BAR_STEPS - 7); // up to pulse 30
    out = advance_lane_a(&e2);              // pulse 31: still bar 0
    ASSERT_EQ(out.note, 10, "bar-armed change waits inside the bar");
    out = advance_lane_a(&e2); // pulse 32 = bar boundary
    ASSERT_EQ(out.note, 20, "bar-armed change applies at the bar");
}

static void test_rearm_replaces_pending(void)
{
    struct test_sequence s1, s2, s3;
    make_flat_sequence(&s1, "s1", 10);
    make_flat_sequence(&s2, "s2", 20);
    make_flat_sequence(&s3, "s3", 30);
    struct SEQ_engine e = make_engine(&s1.seq, &s1.seq);

    advance_pulses(&e, 1);
    SEQ_arm_sequence(&e, SEQ_LANE_A, &s2.seq, SEQ_QUANT_PHRASE);
    SEQ_arm_sequence(&e, SEQ_LANE_A, &s3.seq, SEQ_QUANT_PHRASE);
    advance_pulses(&e, SEQ_PHRASE_STEPS - 1);
    struct SEQ_lane_output out = advance_lane_a(&e); // phrase boundary
    ASSERT_EQ(out.note, 30, "last armed sequence wins");
}

static void test_lanes_are_independent(void)
{
    struct test_sequence sa, sb, sn;
    make_flat_sequence(&sa, "a", 10);
    make_flat_sequence(&sb, "b", 20);
    make_flat_sequence(&sn, "n", 30);
    struct SEQ_engine e = make_engine(&sa.seq, &sb.seq);

    advance_pulses(&e, 1);
    SEQ_arm_sequence(&e, SEQ_LANE_B, &sn.seq, SEQ_QUANT_PHRASE);

    struct SEQ_lane_output out[SEQ_LANES];
    ASSERT_EQ(SEQ_sequence_armed(&e, SEQ_LANE_A), 0, "lane A not armed");
    ASSERT_EQ(SEQ_sequence_armed(&e, SEQ_LANE_B), 1, "lane B armed");
    advance_pulses(&e, SEQ_PHRASE_STEPS - 1);
    SEQ_advance(&e, out); // phrase boundary
    ASSERT_EQ(out[0].note, 10, "lane A unchanged");
    ASSERT_EQ(out[1].note, 30, "lane B switched");
}

static void test_reset_restarts_phrase(void)
{
    struct test_sequence old_seq, new_seq;
    make_ramp_sequence(&old_seq, "ramp");
    make_flat_sequence(&new_seq, "new", 99);
    struct SEQ_engine e = make_engine(&old_seq.seq, &old_seq.seq);

    advance_pulses(&e, 50);
    SEQ_reset(&e);
    struct SEQ_lane_output out = advance_lane_a(&e);
    ASSERT_EQ(out.note, 0, "reset restarts at step 0");
    ASSERT_EQ(SEQ_bar(&e), 0, "reset restarts musical time");

    // a phrase-armed change also applies on the reset pulse
    advance_pulses(&e, 10);
    SEQ_arm_sequence(&e, SEQ_LANE_A, &new_seq.seq, SEQ_QUANT_PHRASE);
    SEQ_reset(&e);
    out = advance_lane_a(&e);
    ASSERT_EQ(out.note, 99, "phrase-armed change applies on reset");
}

static void test_direction_reverse(void)
{
    struct test_sequence t;
    make_ramp_sequence(&t, "ramp");
    struct SEQ_engine e = make_engine(&t.seq, &t.seq);

    SEQ_arm_length_beats(&e, 4, SEQ_QUANT_NOW); // 16 steps
    SEQ_arm_direction(&e, SEQ_DIR_REVERSE, SEQ_QUANT_NOW);

    struct SEQ_lane_output out = advance_lane_a(&e);
    ASSERT_EQ(out.note, 15, "reverse starts at the last step");
    out = advance_lane_a(&e);
    ASSERT_EQ(out.note, 14, "reverse walks down");

    advance_pulses(&e, 13);
    out = advance_lane_a(&e);
    ASSERT_EQ(out.note, 0, "reverse reaches step 0");
    out = advance_lane_a(&e);
    ASSERT_EQ(out.note, 15, "reverse wraps to the end");
}

static void test_direction_pingpong(void)
{
    struct test_sequence t;
    make_ramp_sequence(&t, "ramp");
    struct SEQ_engine e = make_engine(&t.seq, &t.seq);

    SEQ_arm_length_beats(&e, 1, SEQ_QUANT_NOW); // 4 steps
    SEQ_arm_direction(&e, SEQ_DIR_PINGPONG, SEQ_QUANT_NOW);

    static const int expected[] = {0, 1, 2, 3, 2, 1, 0, 1, 2, 3, 2};
    for (unsigned i = 0; i < sizeof(expected) / sizeof(expected[0]); i++)
    {
        struct SEQ_lane_output out = advance_lane_a(&e);
        ASSERT_EQ(out.note, expected[i], "ping-pong bounce order");
    }
}

static void test_direction_random(void)
{
    struct test_sequence t;
    make_ramp_sequence(&t, "ramp");

    struct SEQ_engine e1, e2;
    SEQ_init(&e1, &t.seq, &t.seq, 1234);
    SEQ_init(&e2, &t.seq, &t.seq, 1234);
    SEQ_arm_length_beats(&e1, 2, SEQ_QUANT_NOW); // 8 steps
    SEQ_arm_direction(&e1, SEQ_DIR_RANDOM, SEQ_QUANT_NOW);
    SEQ_arm_length_beats(&e2, 2, SEQ_QUANT_NOW);
    SEQ_arm_direction(&e2, SEQ_DIR_RANDOM, SEQ_QUANT_NOW);

    for (int i = 0; i < 64; i++)
    {
        struct SEQ_lane_output o1 = advance_lane_a(&e1);
        struct SEQ_lane_output o2 = advance_lane_a(&e2);
        tests_run++;
        if (o1.note != o2.note)
        {
            tests_failed++;
            printf("FAIL %s:%d same seed diverged at pulse %d\n", __FILE__,
                   __LINE__, i);
        }
        tests_run++;
        if (o1.note < 0 || o1.note >= 8)
        {
            tests_failed++;
            printf("FAIL %s:%d random position out of range: %d\n", __FILE__,
                   __LINE__, o1.note);
        }
    }
}

static void test_short_length_keeps_phrase_grid(void)
{
    struct test_sequence old_seq, new_seq;
    make_ramp_sequence(&old_seq, "ramp");
    make_flat_sequence(&new_seq, "new", 99);
    struct SEQ_engine e = make_engine(&old_seq.seq, &old_seq.seq);

    SEQ_arm_length_beats(&e, 2, SEQ_QUANT_NOW); // 8-step loop
    advance_pulses(&e, 9);
    ASSERT_EQ(SEQ_position(&e), 0, "loop wraps at the shortened length");

    // ...but the phrase boundary for arming stays on the 128-pulse grid
    SEQ_arm_sequence(&e, SEQ_LANE_A, &new_seq.seq, SEQ_QUANT_PHRASE);
    advance_pulses(&e, SEQ_PHRASE_STEPS - 9); // up to pulse 127
    struct SEQ_lane_output out = advance_lane_a(&e); // pulse 128
    ASSERT_EQ(out.note, 99, "phrase arming ignores the loop length");

    struct SEQ_engine probe = make_engine(&old_seq.seq, &old_seq.seq);
    SEQ_arm_length_beats(&probe, 2, SEQ_QUANT_NOW);
    advance_pulses(&probe, 1); // consume pulse 0 (a phrase boundary itself)
    SEQ_arm_sequence(&probe, SEQ_LANE_A, &new_seq.seq, SEQ_QUANT_PHRASE);
    advance_pulses(&probe, 7);        // up to pulse 7
    out = advance_lane_a(&probe);     // pulse 8: the 8-step loop wraps
    ASSERT_EQ(out.note, 0, "loop wrap alone does not fire phrase arming");
    ASSERT_EQ(SEQ_sequence_armed(&probe, SEQ_LANE_A), 1,
              "switch stays armed across the loop wrap");
}

int main(void)
{
    test_first_pulse_plays_step0();
    test_rest_is_silent();
    test_tie_holds_without_retrigger();
    test_musical_time_counters();
    test_transpose_octave_and_clamp();
    test_scale_quantize();
    test_mute();
    test_arm_sequence_waits_for_phrase();
    test_arm_beat_and_bar_boundaries();
    test_rearm_replaces_pending();
    test_lanes_are_independent();
    test_reset_restarts_phrase();
    test_direction_reverse();
    test_direction_pingpong();
    test_direction_random();
    test_short_length_keeps_phrase_grid();

    printf("%d tests, %d failed\n", tests_run, tests_failed);
    return tests_failed == 0 ? 0 : 1;
}
