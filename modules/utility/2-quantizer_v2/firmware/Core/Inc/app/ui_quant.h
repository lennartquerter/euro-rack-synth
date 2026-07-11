/*
******************************************************************************
* @file           : ui_quant.h
* @author         : Lennart Querter
* @brief          : Header for ui_quant.c file.
*                   Quantizer personality panel UI: note-mask editing,
*                   scale preset browsing, root/transpose on the encoder,
*                   channel select and the LED feedback for all of it.
*
*                   Control mapping (Phase 6 decision):
*                     note buttons     toggle semitone in the active mask
*                     button A / B     select channel A / B
*                     button C         scale browse mode: encoder steps
*                                      through presets (applied live);
*                                      C/click/timeout exits
*                     button D (hold)  shift: root LED blinks; D + note
*                                      button sets the root to that note
*                     encoder turn     transpose (play) / browse (scale)
*                     encoder click    toggle channel (play) / exit (scale)
******************************************************************************
*/

#ifndef __UI_QUANT_H
#define __UI_QUANT_H

#include <stdint.h>

// scale browse mode exits by itself after this much inactivity
#define UI_QUANT_SCALE_TIMEOUT_MS 8000

void UI_QUANT_init(void);

// scan inputs, apply edits, refresh LEDs; call at 1 kHz
void UI_QUANT_tick(void);

#endif
