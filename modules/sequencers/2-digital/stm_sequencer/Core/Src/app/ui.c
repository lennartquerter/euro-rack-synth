//
// Created by Lennart Querter on 05.10.24.
//

#include "app/ui.h"
#include "libs/scales.h"

void ui_init(UI_config* config, COMMAND_QUEUE_config* command_queue_config, SEQUENCER_config* sequencer_config)
{
    config->sequencer_config = sequencer_config;
    config->command_queue_config = command_queue_config;
    config->channel_a_selected = 0;
    config->channel_b_selected = 0;

    config->mode = UI_MODE_RUN;
    config->edit_mode = UI_EDIT_NOTE;
}

void do_next(UI_config* config)
{
    SEQUENCER_config* sequencer_config = config->sequencer_config;

    if (config->channel_a_selected >= (sizeof(CMinor) / sizeof(CMinor[0])) - 1)
    {
        return;
    }
    config->channel_a_selected += 1;
    config->channel_b_selected += 1;

    sequencer_config->channel_a.sequence_notes[sequencer_config->current_index] = CMinor[config->channel_a_selected];
    sequencer_config->channel_b.sequence_notes[sequencer_config->current_index] = CMinor[config->channel_b_selected];
}

void do_prev(UI_config* config)
{
    SEQUENCER_config* sequencer_config = config->sequencer_config;
    if (config->channel_a_selected <= 0)
    {
        return;
    }
    config->channel_a_selected -= 1;
    config->channel_b_selected -= 1;

    sequencer_config->channel_a.sequence_notes[sequencer_config->current_index] = CMinor[config->channel_a_selected];
    sequencer_config->channel_b.sequence_notes[sequencer_config->current_index] = CMinor[config->channel_b_selected];
}

void do_click(UI_config* config)
{
    switch (config->mode)
    {
    case UI_MODE_RUN:
        config->mode = UI_MODE_STOP;
        break;
    case UI_MODE_STOP:
        config->mode = UI_MODE_RUN;
        break;
    case UI_MODE_EDIT:
        if (config->edit_mode == UI_EDIT_NOTE)
        {
            config->edit_mode = UI_EDIT_GATE;
        }
        else
        {
            config->edit_mode = UI_EDIT_NOTE;
        }
        break;
    default:



    }

    config->handled = 1;
}


void ui_run(UI_config* config)
{
    COMMAND_QUEUE_config* command_queue_config = config->command_queue_config;

    if (command_queue_config->idx_rear != command_queue_config->idx_front)
    {
        COMMAND_QUEUE_EVENT event;
        COMMAND_QUEUE_STATUS const state = command_queue_pop(command_queue_config, &event);
        if (state == COMMAND_QUEUE_FAILURE)
        {
            return;
        }

        if (event == ENCODER_CLICK)
        {
            do_click(config);
        }

        if (event == ENCODER_UP)
        {
            do_next(config);
        }

        if (event == ENCODER_DOWN)
        {
            do_prev(config);
        }
    }
}
