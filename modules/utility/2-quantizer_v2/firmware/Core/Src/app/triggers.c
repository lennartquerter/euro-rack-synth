/*
******************************************************************************
* @file           : triggers.c
* @author         : Lennart Querter
* @brief          : Trigger in/out with inversion handling, jack detect
******************************************************************************
*/

#include "app/triggers.h"

#include "main.h"

static GPIO_TypeDef* const trig_in_port[2] = {TRIG_A_IN_GPIO_Port,
                                              TRIG_B_IN_GPIO_Port};
static const uint16_t trig_in_pin[2] = {TRIG_A_IN_Pin, TRIG_B_IN_Pin};

static GPIO_TypeDef* const trig_out_port[2] = {TRIG_A_OUT_GPIO_Port,
                                               TRIG_B_OUT_GPIO_Port};
static const uint16_t trig_out_pin[2] = {TRIG_A_OUT_Pin, TRIG_B_OUT_Pin};

static GPIO_TypeDef* const jack_port[2] = {JACK_DET_A_GPIO_Port,
                                           JACK_DET_B_GPIO_Port};
static const uint16_t jack_pin[2] = {JACK_DET_A_Pin, JACK_DET_B_Pin};

static uint8_t gate_state[2];
static uint8_t edge_pending[2];
static volatile uint16_t pulse_remaining[2];

static uint8_t jack_state[2];
static uint8_t jack_counter[2];

void TRIGGERS_init(void)
{
    for (uint8_t ch = 0; ch < 2; ch++)
    {
        gate_state[ch] = 0;
        edge_pending[ch] = 0;
        pulse_remaining[ch] = 0;
        jack_counter[ch] = 0;
        HAL_GPIO_WritePin(trig_out_port[ch], trig_out_pin[ch],
                          TRIG_OUT_IDLE_LEVEL);
        // start from the live jack state so boot picks the right mode
        jack_state[ch] =
            HAL_GPIO_ReadPin(jack_port[ch], jack_pin[ch]) == GPIO_PIN_SET;
    }
}

void TRIGGERS_poll(void)
{
    for (uint8_t ch = 0; ch < 2; ch++)
    {
        uint8_t pin =
            HAL_GPIO_ReadPin(trig_in_port[ch], trig_in_pin[ch]) == GPIO_PIN_SET;
#if TRIG_IN_ACTIVE_LOW
        uint8_t gate = !pin;
#else
        uint8_t gate = pin;
#endif
        if (gate && !gate_state[ch])
        {
            edge_pending[ch] = 1;
        }
        gate_state[ch] = gate;
    }
}

uint8_t TRIGGERS_take_edge(uint8_t channel)
{
    channel &= 1;
    if (!edge_pending[channel])
    {
        return 0;
    }
    edge_pending[channel] = 0;
    return 1;
}

uint8_t TRIGGERS_gate(uint8_t channel)
{
    return gate_state[channel & 1];
}

uint8_t TRIGGERS_jack_present(uint8_t channel)
{
    return jack_state[channel & 1];
}

void TRIGGERS_pulse(uint8_t channel)
{
    channel &= 1;
    HAL_GPIO_WritePin(trig_out_port[channel], trig_out_pin[channel],
                      TRIG_OUT_ACTIVE_LEVEL);
    pulse_remaining[channel] = TRIGGERS_PULSE_MS;
}

void TRIGGERS_out_level(uint8_t channel, uint8_t active)
{
    channel &= 1;
    pulse_remaining[channel] = 0;
    HAL_GPIO_WritePin(trig_out_port[channel], trig_out_pin[channel],
                      active ? TRIG_OUT_ACTIVE_LEVEL : TRIG_OUT_IDLE_LEVEL);
}

void TRIGGERS_tick(void)
{
    for (uint8_t ch = 0; ch < 2; ch++)
    {
        if (pulse_remaining[ch] > 0 && --pulse_remaining[ch] == 0)
        {
            HAL_GPIO_WritePin(trig_out_port[ch], trig_out_pin[ch],
                              TRIG_OUT_IDLE_LEVEL);
        }

        uint8_t jack =
            HAL_GPIO_ReadPin(jack_port[ch], jack_pin[ch]) == GPIO_PIN_SET;
        if (jack == jack_state[ch])
        {
            jack_counter[ch] = 0;
        }
        else if (++jack_counter[ch] >= TRIGGERS_JACK_DEBOUNCE_TICKS)
        {
            jack_counter[ch] = 0;
            jack_state[ch] = jack;
        }
    }
}
