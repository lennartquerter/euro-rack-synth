/*
******************************************************************************
* @file           : app_seq.h
* @author         : Lennart Querter
* @brief          : Header for app_seq.c file.
*                   Sequencer personality glue: clock sources (internal
*                   BPM clock / external 4 PPQN via the trig-in jack),
*                   reset input, engine outputs to DAC + gate jacks, and
*                   the accessors the CLI uses.
******************************************************************************
*/

#ifndef __APP_SEQ_H
#define __APP_SEQ_H

#include <stdint.h>

#include "app/seq.h"

#define SEQ_APP_BPM_MIN 30
#define SEQ_APP_BPM_MAX 300

// gate drops this long on a retrigger so envelopes refire
#define SEQ_APP_RETRIG_GAP_MS 3

// live engine (sequencer personality only)
struct SEQ_engine* SEQ_APP_engine(void);

// internal-clock run state; ignored while an external clock cable is in
uint8_t SEQ_APP_running(void);
void SEQ_APP_set_running(uint8_t on);

// 1 while a cable sits in the clock jack (jack detect)
uint8_t SEQ_APP_external_clock(void);

// per-lane bank access (lane A = lead bank, lane B = bass bank)
uint16_t SEQ_APP_bank_count(uint8_t lane);
const struct SEQ_sequence* SEQ_APP_bank_sequence(uint8_t lane, uint16_t index);

/*
 * Load a sequence with the given quantization (panel/CLI use
 * SEQ_QUANT_PHRASE — the "after the 4 bars" behavior; the conductor
 * passes its CMD quant bits through). Persists the selection.
 * Returns 0 when the index is outside the bank.
 */
uint8_t SEQ_APP_load(uint8_t lane, uint16_t index, enum SEQ_quant quant);

#endif
