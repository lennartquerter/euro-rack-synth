#include "app/sequencer.h"
#include "app/mcp4822.h"

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

    // 60 BPM is the slowest time we can handle (for now)
    if (BPM < 60)
    {
        BPM = 60;
    }

    // This does not make sense at the moment! need to check how it works!
    config->usec_to_wait = 60 / BPM * 10000;
}

/**
 * @param config
 */
void sequencer_run(SEQUENCER_config* config)
{
    // set CV A value
    mcp4822_write_value(config->mcp4822_config, config->channel_a.sequence_notes[config->current_index],
                        MCP4822_CHANNEL_A);
    // set CV B value
    mcp4822_write_value(config->mcp4822_config, config->channel_b.sequence_notes[config->current_index],
                        MCP4822_CHANNEL_B);

    if (config->channel_a.sequence_is_trigger[config->current_index] > 0)
    {
        // send gate A (and probably reset after 10ms)
        HAL_GPIO_WritePin(GATE_A_OUT_GPIO_Port, GATE_A_OUT_Pin, GPIO_PIN_SET);
    }

    if (config->channel_b.sequence_is_trigger[config->current_index] > 0)
    {
        // send gate B (and probably reset after 10ms)
        HAL_GPIO_WritePin(GATE_B_OUT_GPIO_Port, GATE_B_OUT_Pin, GPIO_PIN_SET);
    }

    // Gate is currently only 10ms (should be changed based on settings?)
    // Gate could be reset with a global variable that knows when to reset the gate
    // should use a global interrupt for htim3 ?
    HAL_Delay(10);
    HAL_GPIO_WritePin(GATE_A_OUT_GPIO_Port, GATE_A_OUT_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GATE_B_OUT_GPIO_Port, GATE_B_OUT_Pin, GPIO_PIN_RESET);
}
