/*
 * uart-nrf.h
 *
 *  Created on: 13-May-2014
 *      Author: prithvi
 */

  
#ifndef UART_NRF_H
#define UART_NRF_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define LINE_END				'\n'
#define UART_BAUDRATE 			UART_BAUDRATE_BAUDRATE_Baud460800

void rx_collect(uint8_t rx_data);
  
void set_rx_handler (void (*handler) (uint8_t * ptr) );

void uart_init(void);

void uart_putchar(uint8_t cr);

#endif
