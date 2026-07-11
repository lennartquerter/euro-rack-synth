/*
******************************************************************************
* @file           : ui_seq.h
* @author         : Lennart Querter
* @brief          : Header for ui_seq.c file.
*                   Sequencer personality panel UI.
*
*                   Play mode:
*                     LEDs 1-8 / 9-12   beat chase / bar within the phrase
*                     button A / B      enter load mode for lane A / B
*                     button C          run/stop (internal clock) or mute
*                                       the focus lane (external clock)
*                     button D (hold)   shift layer
*                     encoder turn      BPM (internal clock)
*                     encoder click     toggle focus lane
*
*                   Load mode (button A or B):
*                     note buttons      toggle bits of the sequence number
*                                       (button n = weight 2^(n-1)); LEDs
*                                       show the value, blinking = out of
*                                       bank range
*                     same button / encoder click  confirm — armed to the
*                                       next phrase boundary
*                     other lane button switch target lane
*                     button C / timeout cancel
*
*                   Shift (hold D, play mode):
*                     A / B             mute/unmute lane A / B
*                     C                 cycle direction fwd/rev/pp/rand
*                     encoder turn      transpose the focus lane
*
*                   Status LEDs: 1/2 lane A/B (on = playing, blink =
*                   armed, off = muted), 3/4 gate activity (app_seq).
******************************************************************************
*/

#ifndef __UI_SEQ_H
#define __UI_SEQ_H

#include <stdint.h>

// load mode cancels itself after this much inactivity
#define UI_SEQ_LOAD_TIMEOUT_MS 8000

void UI_SEQ_init(void);

// scan inputs, apply edits, refresh LEDs; call at 1 kHz
void UI_SEQ_tick(void);

#endif
