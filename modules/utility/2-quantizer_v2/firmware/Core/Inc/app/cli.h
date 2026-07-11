/*
******************************************************************************
* @file           : cli.h
* @author         : Lennart Querter
* @brief          : Header for cli.c file.
*                   Debug shell on USART1 (115200 8N1). RX is interrupt
*                   driven into a ring buffer; parsing and TX happen in the
*                   main loop via CLI_tick().
******************************************************************************
*/

#ifndef __CLI_H
#define __CLI_H

#include "stm32f4xx_hal.h"

void CLI_init(UART_HandleTypeDef* huart);

// called from HAL_UART_RxCpltCallback (main.c); re-arms reception
void CLI_on_rx_isr(void);

// echo + parse + execute pending input, call from the main loop
void CLI_tick(void);

// printf-style output, blocking; safe to call from app code
void CLI_printf(const char* format, ...);

#endif
