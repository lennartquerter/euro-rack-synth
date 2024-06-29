#ifndef _MIDI_HANDLER_H_
#define _MIDI_HANDLER_H_

#include "midi.h"

void midi_handler_init(I2C_HandleTypeDef *cv_i2c, I2C_HandleTypeDef *vel_i2c, I2C_HandleTypeDef *mod_i2c);

void midi_handler_run(MIDI_event *midi_event);

#endif /* _MIDI_HANDLER_H_ */