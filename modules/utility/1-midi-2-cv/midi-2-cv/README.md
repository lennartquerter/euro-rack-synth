# Midi-2-cv software

### Current status

This software is currently a work in progress. Some parts work, others do not.

Use at your own risk!

## Hardware

This project is meant to run on the blackpill board, with an STM32F401CCU6 chip.

It uses 3 x MCP4728 DAC chips to power the CV, Velocity and MOD output. Each of these outputs
is run over an OpAmp to scale up the voltage from the reference (2.048V) to standard Euro rack 10V 
(or at least I hope that is a standard).

## What does it do?

This program is meant to convert MIDI signals, over a DIN socket, into CV outputs.

It contains 1 input, the MIDI cable.

It has a set of 4 x 4 outputs:
- 4x Gate output
- 4x CV output (1V/oct)
- 4x Velocity output
- 4x MOD output (???)

It might contain additional gate outputs, but I'm not 100% sure of that.

## How does it work?

Midi is received on the USART input (RX). On each interrupt (=every time a new byte has been read from the input), 
the byte is sent to an internal buffer (buffer.c). 

During the WHILE loop in main.c, the midi.c file will check the value in the buffer. If the buffer contains
a valid MIDI message, it will be analyzed and returned to the main loop.

If the main loop has a valid MIDI message, the midi_handler.c will translate it
into the correct CV/VEL/MOD signals, each driven by their own MCP4728. 
This works over the I2C protocol. The driver can be found in the mcp4728.c file.

## How to compile?

I personally use a mix of CLion and STM32CubeIDE, but both should be capable of
building and uploading the software to the device over a STLinkV2. 

## TODO:

1. Add support for MOD
2. Add support for multi-notes on same channel (each note on different CV output)
3. Add support for multi-channel (each channel on different CV output)
4. Routing of midi messages to subsequent channels
5. Add driver for LED lights (could also just be taken from the Gate out)
6. Understand which GATE sockets are connected (mainly for subsequent channels)

## Nice to have:

1. MIDI clock output
