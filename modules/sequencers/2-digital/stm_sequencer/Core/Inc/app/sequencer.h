#ifndef _SEQUENCER_H_
#define _SEQUENCER_H_

#include "main.h"
#include "app/mcp4822.h"

extern TIM_HandleTypeDef htim3;

typedef struct
{
    uint16_t sequence_notes[32];
    uint16_t sequence_is_trigger[32];
} SEQUENCER_data;

typedef struct
{
    MCP4822_config* mcp4822_config;
    TIM_HandleTypeDef* timer;
    uint8_t sequence_length;
    uint8_t current_index;

    int usec_to_wait;

    SEQUENCER_data channel_a;
    SEQUENCER_data channel_b;
} SEQUENCER_config;

void sequencer_init(SEQUENCER_config* config, int BPM, MCP4822_config* mcp4822_config, TIM_HandleTypeDef* timer);

void sequencer_run(SEQUENCER_config* config);


#endif /* _SEQUENCER_H_ */
