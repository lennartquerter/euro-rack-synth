/*
******************************************************************************
* @file           : ui.h
* @author         : Lennart Querter
* @brief          : Header for ui.c file.
*                   Handles updates from the command_queue, sets LED's and other UI values
******************************************************************************
*/

#ifndef __UI_H
#define __UI_H

#include "app/sequencer.h"
#include "app/command_queue.h"

typedef enum
{
    UI_MODE_RUN,
    UI_MODE_STOP,
    UI_MODE_EDIT,

    UI_MODE_MENU,
} UI_MODE;

typedef enum
{
    UI_EDIT_NOTE,
    UI_EDIT_GATE,
} UI_EDIT_MODE;

typedef struct
{
    SEQUENCER_config* sequencer_config;
    COMMAND_QUEUE_config* command_queue_config;

    UI_MODE mode;
    UI_EDIT_MODE edit_mode;

    int channel_a_selected;
    int channel_b_selected;

    int handled;
    int next_pressed;

    // interface options

    // multiplexer for 2x8 status LED



} UI_config;

void ui_init(UI_config* config, COMMAND_QUEUE_config* command_queue_config, SEQUENCER_config* sequencer_config);

void ui_run(UI_config* config);

#endif //__UI_H
