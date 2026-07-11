/*
******************************************************************************
* @file           : personality.h
* @author         : Lennart Querter
* @brief          : Personality dispatch. The same hardware boots as a
*                   quantizer or a sequencer (flipped faceplate); app.c
*                   picks one at boot (PB2 MODE strap / flash override /
*                   boot combo) and drives it through this interface.
******************************************************************************
*/

#ifndef __PERSONALITY_H
#define __PERSONALITY_H

struct PERSONALITY
{
    const char* name;
    void (*init)(void);      // after shared drivers are up
    void (*fast_loop)(void); // every main-loop iteration
    void (*tick)(void);      // 1 kHz
};

extern const struct PERSONALITY PERSONALITY_quantizer; // app_quant.c
extern const struct PERSONALITY PERSONALITY_sequencer; // app_seq.c

#endif
