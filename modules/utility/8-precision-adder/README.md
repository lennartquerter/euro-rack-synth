# Numica Core

A eurorack module to add two signals together. It is based on the Precision Adder module by Doepfer, the A-185-2.

## Features

- **Voltage Level Control**: The module has a voltage level control, used for a fine adjustment of the initial CV level
  for the first input (0-1V).
- **Four inputs**: The module has four inputs for CV signals, They will be mixed together and added to the output. If
  not patched, the inputs will be set to exactly 1V.
- **Four CV toggles**: The module has four toggles, which are used to control the level of each input signal. The
  toggles can be used to route the input signals to either the inverted or normal buffer.
- **Three outputs**: The module has four outputs for CV signals, 3 of them are the sum of the inputs
- **Inverted output**: The module has an inverted output, which is the sum of the inputs multiplied by -1.

## Usage

The module can be used to add two or more CV signals together. The output of the module will be the sum of the input
signals, with the option to invert the output signal. The module can also be used to control the level of each input
signal using the toggles.
The module can be used in a variety of applications, including:

- Mixing CV signals
- Controlling the level of CV signals
- Creating complex CV signals
- Creating custom CV signals
- 1V/Octave control (add or subtract 1V to the signal)

## Components

[IBOM](./ibom.html)

