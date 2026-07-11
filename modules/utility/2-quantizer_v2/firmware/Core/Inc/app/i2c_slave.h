/*
******************************************************************************
* @file           : i2c_slave.h
* @author         : Lennart Querter
* @brief          : Header for i2c_slave.c file.
*                   Conductor protocol slave on I2C1 (see
*                   ../../11-conductor/protocol.md v0.1):
*                   frame [CMD][LEN][PAYLOAD 0-32][SEQ][CHK], XOR
*                   checksum, duplicate suppression per command code,
*                   CMD bits 7-6 = quantized apply (NOW/BEAT/BAR/PHRASE)
*                   mapped straight onto the engine's arming API.
*
*                   Addressing: sequencer type 000001 -> 7-bit address
*                   0x04 + instance (0-3, from settings), plus the
*                   broadcast address (wire byte 0xFC = 7-bit 0x7E).
*
*                   Lane mapping (module-side convention, v1):
*                   SET_PATTERN bit 6 selects the lane (0-63 = A,
*                   64-127 = B); TRANSPOSE / SET_SCALE / SET_OCTAVE /
*                   MUTE apply to both lanes. MUTATE and SET_DENSITY
*                   are accepted but ignored in v1.
*
*                   Bytes are collected in the I2C event ISR; parsing
*                   and applying happen in the main loop (poll).
******************************************************************************
*/

#ifndef __I2C_SLAVE_H
#define __I2C_SLAVE_H

#include "stm32f4xx_hal.h"

#define I2C_SLAVE_TYPE_SEQUENCER 0x04 // 7-bit base address, + instance
#define I2C_SLAVE_BROADCAST 0x7E      // 7-bit; 0xFC on the wire
#define I2C_SLAVE_MAX_PAYLOAD 32

struct I2C_SLAVE_stats
{
    uint32_t applied;
    uint32_t duplicates;
    uint32_t bad_frames; // checksum/length failures
    uint32_t unknown;    // unknown command codes (silently dropped)
    uint32_t bus_errors;
};

// (re)configure I2C1 as a conductor slave using settings->i2c_instance
// and start listening; safe to call again after an instance change
void I2C_SLAVE_init(I2C_HandleTypeDef* hi2c);

// parse + apply a pending frame; call from the personality loop
void I2C_SLAVE_poll(void);

// resolved 7-bit own address (for the CLI)
uint8_t I2C_SLAVE_address(void);
const struct I2C_SLAVE_stats* I2C_SLAVE_get_stats(void);

/*
 * ISR forwarding — called from the HAL callbacks in main.c.
 */
void I2C_SLAVE_on_addr(uint8_t direction, uint16_t match_code);
void I2C_SLAVE_on_rx_complete(void);
void I2C_SLAVE_on_tx_complete(void);
void I2C_SLAVE_on_listen_complete(void);
void I2C_SLAVE_on_error(void);

#endif
