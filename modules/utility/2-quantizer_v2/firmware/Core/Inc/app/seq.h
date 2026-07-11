/*
******************************************************************************
* @file           : seq.h
* @author         : Lennart Querter
* @brief          : Header for seq.c file.
*                   Sequencer engine core: transport position (4 PPQN
*                   sub-steps over the 4-bar phrase grid), per-lane
*                   playback (transpose, octave, scale, mute) and the
*                   arming mechanism that queues changes to the next
*                   beat/bar/phrase boundary — shared by the panel UI,
*                   the CLI and the Conductor I2C commands.
*
*                   Pure C, no HAL — unit-tested host-side (test_seq.c).
*                   The engine is bank-agnostic: it plays any
*                   struct SEQ_sequence handed to it.
******************************************************************************
*/

#ifndef __SEQ_H
#define __SEQ_H

#include <stdint.h>

#include "app/sequences.h"

#define SEQ_LANES 2
#define SEQ_LANE_A 0
#define SEQ_LANE_B 1

// DAC output range at 1V/oct: 0-10V
#define SEQ_NOTE_MAX 120

#define SEQ_TRANSPOSE_MIN (-24)
#define SEQ_TRANSPOSE_MAX 24
#define SEQ_OCTAVE_MIN (-3)
#define SEQ_OCTAVE_MAX 3

// queued changes waiting for a boundary (one per type+lane, last wins)
#define SEQ_PENDING_SLOTS 8

// matches the conductor protocol CMD bits 7-6
enum SEQ_quant
{
    SEQ_QUANT_NOW = 0,
    SEQ_QUANT_BEAT = 1,
    SEQ_QUANT_BAR = 2,
    SEQ_QUANT_PHRASE = 3,
};

// matches the conductor SET_DIR payload
enum SEQ_direction
{
    SEQ_DIR_FORWARD = 0,
    SEQ_DIR_REVERSE = 1,
    SEQ_DIR_PINGPONG = 2,
    SEQ_DIR_RANDOM = 3,
};

// what one lane does during one sub-step
struct SEQ_lane_output
{
    int8_t note;       // final note (0..SEQ_NOTE_MAX), SEQ_STEP_REST if silent
    uint8_t gate;      // 1 = gate high during this sub-step
    uint8_t retrigger; // 1 = new onset (fire the trig out / restart env)
    uint8_t accent;    // reserved flag passed through
};

struct SEQ_lane
{
    const struct SEQ_sequence* seq;
    int8_t transpose;
    int8_t octave;
    uint16_t scale_mask; // 0xFFF (or 0) = no quantization
    int16_t scale_root;
    uint8_t mute;
};

enum SEQ_change_type
{
    SEQ_CHANGE_SEQUENCE = 0,
    SEQ_CHANGE_TRANSPOSE,
    SEQ_CHANGE_OCTAVE,
    SEQ_CHANGE_MUTE,
    SEQ_CHANGE_DIRECTION, // transport-wide
    SEQ_CHANGE_LENGTH,    // transport-wide, value in beats
};

struct SEQ_pending
{
    uint8_t used;
    uint8_t type;
    uint8_t lane;
    uint8_t quant;
    int16_t value;
    const struct SEQ_sequence* seq;
};

struct SEQ_engine
{
    struct SEQ_lane lanes[SEQ_LANES];
    struct SEQ_pending pending[SEQ_PENDING_SLOTS];

    uint32_t pulse;        // musical time in sub-steps since (re)start
    uint8_t started;       // 0 until the first advance
    uint8_t reset_armed;   // next advance restarts at the phrase top
    uint16_t length_steps; // loop length in sub-steps
    uint8_t direction;     // enum SEQ_direction
    int16_t position;      // current pattern step index
    int8_t pingpong_dir;
    uint32_t rng;
};

void SEQ_init(struct SEQ_engine* engine, const struct SEQ_sequence* seq_a,
              const struct SEQ_sequence* seq_b, uint32_t rng_seed);

/*
 * One 4 PPQN clock pulse: moves the transport, applies pending changes
 * whose boundary has been reached, and fills out[] for both lanes.
 */
void SEQ_advance(struct SEQ_engine* engine,
                 struct SEQ_lane_output out[SEQ_LANES]);

// next pulse restarts at the phrase top (standard modular reset behavior);
// phrase-armed changes apply on that pulse too
void SEQ_reset(struct SEQ_engine* engine);

/*
 * Arming: with SEQ_QUANT_NOW the change applies immediately, otherwise it
 * is queued to the next beat/bar/phrase boundary. Panel, CLI and the
 * Conductor all use these — one code path for "switch after the 4 bars".
 */
void SEQ_arm_sequence(struct SEQ_engine* engine, uint8_t lane,
                      const struct SEQ_sequence* seq, enum SEQ_quant quant);
void SEQ_arm_transpose(struct SEQ_engine* engine, uint8_t lane,
                       int16_t semitones, enum SEQ_quant quant);
void SEQ_arm_octave(struct SEQ_engine* engine, uint8_t lane, int16_t octave,
                    enum SEQ_quant quant);
void SEQ_arm_mute(struct SEQ_engine* engine, uint8_t lane, uint8_t on,
                  enum SEQ_quant quant);
void SEQ_arm_direction(struct SEQ_engine* engine, enum SEQ_direction direction,
                       enum SEQ_quant quant);
void SEQ_arm_length_beats(struct SEQ_engine* engine, uint16_t beats,
                          enum SEQ_quant quant);

// tone shaping, applied immediately (not a timing event)
void SEQ_set_scale(struct SEQ_engine* engine, uint8_t lane, uint16_t mask,
                   int16_t root);

// 1 while a sequence switch is queued for this lane (for the armed-blink)
uint8_t SEQ_sequence_armed(const struct SEQ_engine* engine, uint8_t lane);

// position for the panel display
uint16_t SEQ_position(const struct SEQ_engine* engine); // pattern step index
uint8_t SEQ_beat(const struct SEQ_engine* engine); // 0..SEQ_BEATS_PER_BAR-1
uint8_t SEQ_bar(const struct SEQ_engine* engine); // 0..SEQ_BARS_PER_PHRASE-1

#endif
