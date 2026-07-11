/*
******************************************************************************
* @file           : triggers.h
* @author         : Lennart Querter
* @brief          : Header for triggers.c file.
*                   Trigger inputs (PC4/PC5, NPN-inverted: pin low = gate
*                   high), jack detect (PC0/PC1, high = cable inserted) and
*                   trigger outputs (PB0/PB1, output stage inverts: GPIO
*                   low = jack ~8V). Inputs are polled from the fast loop
*                   for low-jitter edges; pulse timing and jack-detect
*                   debounce run on the 1 kHz tick.
******************************************************************************
*/

#ifndef __TRIGGERS_H
#define __TRIGGERS_H

#include "stm32f4xx_hal.h"

#define TRIGGERS_CHANNEL_A 0
#define TRIGGERS_CHANNEL_B 1

// output pulse width, plan asks for 5-10 ms
#define TRIGGERS_PULSE_MS 8

/*
 * TO CONFIRM ON SCOPE (development-plan §7): with Q1/Q2 off the 1K/2K
 * divider holds the jack at ~8V; driving the GPIO high turns Q on and pulls
 * the jack to 0V. So idle = GPIO high (jack 0V), pulse = GPIO low.
 */
#define TRIG_OUT_IDLE_LEVEL GPIO_PIN_SET
#define TRIG_OUT_ACTIVE_LEVEL GPIO_PIN_RESET

// input stage inverts: jack gate high pulls the pin low
#define TRIG_IN_ACTIVE_LOW 1

// consecutive 1 kHz samples before a jack detect change is believed
#define TRIGGERS_JACK_DEBOUNCE_TICKS 5

void TRIGGERS_init(void);

// sample trigger inputs + detect rising edges; call from the fast loop
void TRIGGERS_poll(void);

// consume a pending rising edge (returns 1 once per edge)
uint8_t TRIGGERS_take_edge(uint8_t channel);

// current logical gate level at the jack (inversion applied)
uint8_t TRIGGERS_gate(uint8_t channel);

// debounced jack detect: 1 = cable in the trig-in jack (triggered mode)
uint8_t TRIGGERS_jack_present(uint8_t channel);

// fire an output pulse (jack goes ~8V for TRIGGERS_PULSE_MS)
void TRIGGERS_pulse(uint8_t channel);

// direct gate-level control of the output jack (1 = ~8V); cancels a
// pending one-shot pulse — used by the sequencer for gate/tie shaping
void TRIGGERS_out_level(uint8_t channel, uint8_t active);

// pulse timing + jack detect debounce; call at 1 kHz
void TRIGGERS_tick(void);

#endif
