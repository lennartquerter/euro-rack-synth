#include "app/sequencer.h"

#include <math.h>
#include <sys/types.h>

void sequencer_init(SEQUENCER_config* config, int BPM, MCP4822_config* mcp4822_config, TIM_HandleTypeDef* timer)
{
    config->mcp4822_config = mcp4822_config;
    config->timer = timer;

    config->sequence_length = 8;
    config->current_index = 0;

    const int length = sizeof(config->channel_a.sequence_notes) / sizeof(config->channel_a.sequence_notes[0]);
    // Calculate the length of the array

    for (int i = 0; i < length; i++)
    {
        // Set default all values to 0 -- > 0V
        config->channel_a.sequence_notes[i] = 0;
        config->channel_b.sequence_notes[i] = 0;

        // Set default all values to 1 --> TRIGGER_ON
        config->channel_a.sequence_is_trigger[i] = 1;
        config->channel_b.sequence_is_trigger[i] = 1;
    }

    // 60 BPM is the slowest time we can handle
    if (BPM < 60)
    {
        BPM = 60;
    }

    // This does not make sense at the moment! need to check how it works!
    config->usec_to_wait = 60 / BPM * 1000;
}

uint16_t getVoltageForScale(uint16_t value)
{
    // Octave select required
    const uint16_t base = 1000;
    return base + value;
}

/**
 * @param config
 */
void sequencer_run(SEQUENCER_config* config)
{
    // set CV A value
    uint16_t const note_a = config->channel_a.sequence_notes[config->current_index];
    uint16_t const note_b = config->channel_b.sequence_notes[config->current_index];


    mcp4822_write_value(config->mcp4822_config, getVoltageForScale(note_a),
                        MCP4822_CHANNEL_A);
    // set CV B value
    mcp4822_write_value(config->mcp4822_config, getVoltageForScale(note_b),
                        MCP4822_CHANNEL_B);

    uint8_t gateLengthA = config->channel_a.sequence_is_trigger[config->current_index];

    if (gateLengthA > 0)
    {
        // send gate A (and probably reset after 10ms)
        HAL_GPIO_WritePin(GATE_A_OUT_GPIO_Port, GATE_A_OUT_Pin, GPIO_PIN_SET);
    }

    uint8_t gateLengthB = config->channel_b.sequence_is_trigger[config->current_index];

    if (gateLengthB > 0)
    {
        // send gate B (and probably reset after 10ms)
        HAL_GPIO_WritePin(GATE_B_OUT_GPIO_Port, GATE_B_OUT_Pin, GPIO_PIN_SET);
    }

    // Gate is currently only set to the value for trigger,
    // Gate could be reset with a global variable that knows when to reset the gate
    // should use a global interrupt for htim3?
    // maximum time of gate should be usec_to_wait - 10ms?

    // we calculate the least time we need to wait
    uint32_t min_wait_time = fmin(gateLengthA, gateLengthB);

    HAL_Delay(min_wait_time);
    // we see which gate should be turned off
    if (min_wait_time == gateLengthA)
    {
        HAL_GPIO_WritePin(GATE_A_OUT_GPIO_Port, GATE_A_OUT_Pin, GPIO_PIN_RESET);
    }
    else
    {
        HAL_GPIO_WritePin(GATE_B_OUT_GPIO_Port, GATE_B_OUT_Pin, GPIO_PIN_RESET);
    }

    // we wait the remaining time
    HAL_Delay(fabs(gateLengthA - gateLengthB));

    // reset both pins to reduce if/else
    HAL_GPIO_WritePin(GATE_B_OUT_GPIO_Port, GATE_B_OUT_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GATE_A_OUT_GPIO_Port, GATE_A_OUT_Pin, GPIO_PIN_RESET);

    // Set next step

    // Set current index to the next step
    config->current_index++;

    // Reset index when we reach the end of the sequence
    if (config->current_index >= config->sequence_length)
    {
        config->current_index = 0;
    }
}
