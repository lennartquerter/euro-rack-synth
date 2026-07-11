/*
******************************************************************************
* @file           : app_quant.h
* @author         : Lennart Querter
* @brief          : Header for app_quant.c file.
*                   Quantizer personality: fast CV->quantize->DAC path and
*                   the quantizer-domain state accessors used by the CLI
*                   and ui_quant.c.
******************************************************************************
*/

#ifndef __APP_QUANT_H
#define __APP_QUANT_H

#include <stdint.h>

#include "app/quantize.h"

struct QUANTIZE_state* QUANT_APP_state(uint8_t channel);
void QUANT_APP_apply_mask(uint8_t channel, uint16_t mask);
void QUANT_APP_apply_root(uint8_t channel, int16_t root);
void QUANT_APP_apply_transpose(uint8_t channel, int8_t transpose);

// last quantized note (before transpose), QUANTIZE_NO_NOTE if none yet
int16_t QUANT_APP_last_note(uint8_t channel);

// re-sync quantizer state after settings load/defaults/calibration
void QUANT_APP_reload_from_settings(void);

#endif
