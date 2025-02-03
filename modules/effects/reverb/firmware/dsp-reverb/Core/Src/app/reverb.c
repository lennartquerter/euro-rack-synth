//
// Created by Lennart Querter on 02.02.25.
//

#include "app/reverb.h"

void REVERB_run(REVERB_parameters* parameters) {

}

// Process the audio channels
void REVERB_process(int32_t* input, int32_t* output, uint16_t offset, uint16_t size)
{
    for (uint16_t i = 0; i < size; i++)
    {
        // Get left and right channels
        int32_t leftIn = input[offset + i * 2];
        int32_t rightIn = input[offset + i * 2 + 1];

        // Your reverb processing here
        int32_t leftOut = leftIn; // Replace with reverb
        int32_t rightOut = rightIn; // Replace with reverb

        // Write to output buffer
        output[offset + i * 2] = leftOut;
        output[offset + i * 2 + 1] = rightOut;
    }
}
