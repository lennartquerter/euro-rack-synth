/*
******************************************************************************
* @file           : seq.c
* @author         : Lennart Querter
* @brief          : Sequencer engine core (pure C, host-testable)
******************************************************************************
*/

#include "app/seq.h"

#include <string.h>

#include "app/quantize.h"

static uint32_t rng_next(struct SEQ_engine* engine)
{
    uint32_t x = engine->rng;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    engine->rng = x ? x : 0x1DEA5EEDUL;
    return engine->rng;
}

static int16_t clamp16(int16_t value, int16_t min, int16_t max)
{
    if (value < min)
    {
        return min;
    }
    if (value > max)
    {
        return max;
    }
    return value;
}

void SEQ_init(struct SEQ_engine* engine, const struct SEQ_sequence* seq_a,
              const struct SEQ_sequence* seq_b, uint32_t rng_seed)
{
    memset(engine, 0, sizeof(*engine));

    engine->lanes[SEQ_LANE_A].seq = seq_a;
    engine->lanes[SEQ_LANE_B].seq = seq_b;
    for (uint8_t lane = 0; lane < SEQ_LANES; lane++)
    {
        engine->lanes[lane].scale_mask = 0x0FFF; // chromatic = pass-through
    }

    engine->length_steps = SEQ_PHRASE_STEPS;
    engine->direction = SEQ_DIR_FORWARD;
    engine->pingpong_dir = 1;
    engine->rng = rng_seed ? rng_seed : 0x1DEA5EEDUL;
}

void SEQ_reset(struct SEQ_engine* engine)
{
    engine->reset_armed = 1;
}

/*
 * Pending-change plumbing. NOW applies immediately; everything else is
 * upserted into a slot keyed by (type, lane) so re-arming replaces the
 * previous request (last command wins).
 */

static uint8_t change_is_transport_wide(uint8_t type)
{
    return type == SEQ_CHANGE_DIRECTION || type == SEQ_CHANGE_LENGTH;
}

static void apply_change(struct SEQ_engine* engine,
                         const struct SEQ_pending* change)
{
    struct SEQ_lane* lane = &engine->lanes[change->lane % SEQ_LANES];

    switch (change->type)
    {
        case SEQ_CHANGE_SEQUENCE:
            if (change->seq != NULL)
            {
                lane->seq = change->seq;
            }
            break;
        case SEQ_CHANGE_TRANSPOSE:
            lane->transpose = (int8_t)clamp16(change->value, SEQ_TRANSPOSE_MIN,
                                              SEQ_TRANSPOSE_MAX);
            break;
        case SEQ_CHANGE_OCTAVE:
            lane->octave =
                (int8_t)clamp16(change->value, SEQ_OCTAVE_MIN, SEQ_OCTAVE_MAX);
            break;
        case SEQ_CHANGE_MUTE:
            lane->mute = change->value ? 1 : 0;
            break;
        case SEQ_CHANGE_DIRECTION:
            engine->direction = (uint8_t)(change->value & 3);
            break;
        case SEQ_CHANGE_LENGTH:
        {
            int16_t beats = clamp16(change->value, 1,
                                    SEQ_BARS_PER_PHRASE * SEQ_BEATS_PER_BAR);
            engine->length_steps = (uint16_t)(beats * SEQ_SUBSTEPS_PER_BEAT);
            if (engine->position >= engine->length_steps)
            {
                engine->position = (int16_t)(engine->position %
                                             engine->length_steps);
            }
            break;
        }
        default:
            break;
    }
}

static void arm_change(struct SEQ_engine* engine, uint8_t type, uint8_t lane,
                       int16_t value, const struct SEQ_sequence* seq,
                       enum SEQ_quant quant)
{
    struct SEQ_pending change = {1, type, (uint8_t)(lane % SEQ_LANES),
                                 (uint8_t)quant, value, seq};

    if (quant == SEQ_QUANT_NOW)
    {
        apply_change(engine, &change);
        return;
    }

    struct SEQ_pending* free_slot = NULL;
    for (uint8_t i = 0; i < SEQ_PENDING_SLOTS; i++)
    {
        struct SEQ_pending* slot = &engine->pending[i];
        if (slot->used && slot->type == type &&
            (change_is_transport_wide(type) || slot->lane == change.lane))
        {
            *slot = change; // re-arm replaces the previous request
            return;
        }
        if (!slot->used && free_slot == NULL)
        {
            free_slot = slot;
        }
    }
    if (free_slot != NULL)
    {
        *free_slot = change;
    }
    // all slots busy (can't happen with one slot per type+lane): drop
}

// largest boundary starting at this pulse; compare with enum SEQ_quant
static uint8_t boundary_level(uint32_t pulse)
{
    if (pulse % SEQ_PHRASE_STEPS == 0)
    {
        return SEQ_QUANT_PHRASE;
    }
    if (pulse % SEQ_BAR_STEPS == 0)
    {
        return SEQ_QUANT_BAR;
    }
    if (pulse % SEQ_SUBSTEPS_PER_BEAT == 0)
    {
        return SEQ_QUANT_BEAT;
    }
    return SEQ_QUANT_NOW; // mid-beat: nothing queued can apply
}

static void apply_pending(struct SEQ_engine* engine, uint8_t level)
{
    for (uint8_t i = 0; i < SEQ_PENDING_SLOTS; i++)
    {
        struct SEQ_pending* slot = &engine->pending[i];
        if (slot->used && slot->quant <= level)
        {
            slot->used = 0;
            apply_change(engine, slot);
        }
    }
}

/*
 * Transport movement.
 */

static int16_t start_position(struct SEQ_engine* engine)
{
    switch (engine->direction)
    {
        case SEQ_DIR_REVERSE:
            return (int16_t)(engine->length_steps - 1);
        case SEQ_DIR_RANDOM:
            return (int16_t)(rng_next(engine) % engine->length_steps);
        default:
            return 0;
    }
}

static void move_position(struct SEQ_engine* engine)
{
    int16_t length = (int16_t)engine->length_steps;

    if (engine->position >= length)
    {
        engine->position = (int16_t)(engine->position % length);
    }

    switch (engine->direction)
    {
        case SEQ_DIR_REVERSE:
            engine->position =
                (int16_t)(engine->position == 0 ? length - 1
                                                : engine->position - 1);
            break;
        case SEQ_DIR_PINGPONG:
        {
            int16_t next = (int16_t)(engine->position + engine->pingpong_dir);
            if (next >= length)
            {
                engine->pingpong_dir = -1;
                next = (int16_t)(length >= 2 ? length - 2 : 0);
            }
            else if (next < 0)
            {
                engine->pingpong_dir = 1;
                next = (int16_t)(length >= 2 ? 1 : 0);
            }
            engine->position = next;
            break;
        }
        case SEQ_DIR_RANDOM:
            engine->position = (int16_t)(rng_next(engine) % length);
            break;
        default:
            engine->position = (int16_t)((engine->position + 1) % length);
            break;
    }
}

/*
 * Note pipeline: step note + transpose + octave, optionally snapped to
 * the lane's scale mask (nearest allowed semitone, ties go low), clamped
 * to the DAC range.
 */

static int16_t nearest_allowed(uint16_t mask, int16_t root, int16_t note)
{
    struct QUANTIZE_state state;
    state.mask = mask & 0x0FFF;
    state.root = (int16_t)(root % 12);
    state.last_note = QUANTIZE_NO_NOTE;

    if (state.mask == 0 || state.mask == 0x0FFF)
    {
        return note;
    }
    for (int16_t distance = 0; distance <= 12; distance++)
    {
        if (QUANTIZE_note_allowed(&state, (int16_t)(note - distance)))
        {
            return (int16_t)(note - distance);
        }
        if (QUANTIZE_note_allowed(&state, (int16_t)(note + distance)))
        {
            return (int16_t)(note + distance);
        }
    }
    return note;
}

static void emit_lane(const struct SEQ_lane* lane, int16_t position,
                      struct SEQ_lane_output* out)
{
    const struct SEQ_step* step = &lane->seq->steps[position];

    if (!(step->flags & SEQ_FLAG_GATE) || lane->mute)
    {
        out->note = SEQ_STEP_REST;
        out->gate = 0;
        out->retrigger = 0;
        out->accent = 0;
        return;
    }

    int16_t note = (int16_t)(step->note + lane->transpose + 12 * lane->octave);
    note = nearest_allowed(lane->scale_mask, lane->scale_root, note);
    note = clamp16(note, 0, SEQ_NOTE_MAX);

    out->note = (int8_t)note;
    out->gate = 1;
    out->retrigger = !(step->flags & SEQ_FLAG_TIE);
    out->accent = (step->flags & SEQ_FLAG_ACCENT) ? 1 : 0;
}

void SEQ_advance(struct SEQ_engine* engine,
                 struct SEQ_lane_output out[SEQ_LANES])
{
    if (!engine->started || engine->reset_armed)
    {
        engine->started = 1;
        engine->reset_armed = 0;
        engine->pulse = 0;
        engine->pingpong_dir = 1;
        engine->position = start_position(engine);
    }
    else
    {
        engine->pulse++;
        move_position(engine);
    }

    apply_pending(engine, boundary_level(engine->pulse));

    for (uint8_t lane = 0; lane < SEQ_LANES; lane++)
    {
        emit_lane(&engine->lanes[lane], engine->position, &out[lane]);
    }
}

/*
 * Arming API.
 */

void SEQ_arm_sequence(struct SEQ_engine* engine, uint8_t lane,
                      const struct SEQ_sequence* seq, enum SEQ_quant quant)
{
    arm_change(engine, SEQ_CHANGE_SEQUENCE, lane, 0, seq, quant);
}

void SEQ_arm_transpose(struct SEQ_engine* engine, uint8_t lane,
                       int16_t semitones, enum SEQ_quant quant)
{
    arm_change(engine, SEQ_CHANGE_TRANSPOSE, lane, semitones, NULL, quant);
}

void SEQ_arm_octave(struct SEQ_engine* engine, uint8_t lane, int16_t octave,
                    enum SEQ_quant quant)
{
    arm_change(engine, SEQ_CHANGE_OCTAVE, lane, octave, NULL, quant);
}

void SEQ_arm_mute(struct SEQ_engine* engine, uint8_t lane, uint8_t on,
                  enum SEQ_quant quant)
{
    arm_change(engine, SEQ_CHANGE_MUTE, lane, on, NULL, quant);
}

void SEQ_arm_direction(struct SEQ_engine* engine, enum SEQ_direction direction,
                       enum SEQ_quant quant)
{
    arm_change(engine, SEQ_CHANGE_DIRECTION, 0, (int16_t)direction, NULL,
               quant);
}

void SEQ_arm_length_beats(struct SEQ_engine* engine, uint16_t beats,
                          enum SEQ_quant quant)
{
    arm_change(engine, SEQ_CHANGE_LENGTH, 0, (int16_t)beats, NULL, quant);
}

void SEQ_set_scale(struct SEQ_engine* engine, uint8_t lane, uint16_t mask,
                   int16_t root)
{
    lane %= SEQ_LANES;
    engine->lanes[lane].scale_mask = mask & 0x0FFF;
    engine->lanes[lane].scale_root = (int16_t)(root % 12);
}

uint8_t SEQ_sequence_armed(const struct SEQ_engine* engine, uint8_t lane)
{
    lane %= SEQ_LANES;
    for (uint8_t i = 0; i < SEQ_PENDING_SLOTS; i++)
    {
        const struct SEQ_pending* slot = &engine->pending[i];
        if (slot->used && slot->type == SEQ_CHANGE_SEQUENCE &&
            slot->lane == lane)
        {
            return 1;
        }
    }
    return 0;
}

uint16_t SEQ_position(const struct SEQ_engine* engine)
{
    return (uint16_t)engine->position;
}

uint8_t SEQ_beat(const struct SEQ_engine* engine)
{
    return (uint8_t)((engine->pulse % SEQ_BAR_STEPS) / SEQ_SUBSTEPS_PER_BEAT);
}

uint8_t SEQ_bar(const struct SEQ_engine* engine)
{
    return (uint8_t)((engine->pulse % SEQ_PHRASE_STEPS) / SEQ_BAR_STEPS);
}
