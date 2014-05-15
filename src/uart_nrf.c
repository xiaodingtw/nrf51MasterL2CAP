/*
 * uart-nrf.c
 *
 *  Created on: 13-May-2014
 *      Author: prithvi
 */

#include "nrf.h"
#include "uart_nrf.h"
#include "nrf_gpio.h"
#include "board.h"

#define BUFFER_SIZE		128

uint8_t rx_buffer[BUFFER_SIZE];

void  (*rx_handler) (uint8_t * ptr);

void rx_collect(uint8_t rx_data){
	static uint32_t count = 0;
	if(rx_data != LINE_END){
		if(count < BUFFER_SIZE -1){
			rx_buffer[count] = rx_data;
			count++;
		}
	}else{

		rx_buffer[count] = '\0';
		if(rx_handler != NULL){
			rx_handler(rx_buffer);
		}
		count = 0;
	}
}

void
UART0_IRQHandler(void) 
{
	/* Wait for RX data to be received.
	 * No waiting actually since RX causes interrupt. */
	while(NRF_UART0->EVENTS_RXDRDY != 1);
	NRF_UART0->EVENTS_RXDRDY = 0;
	rx_collect((uint8_t)NRF_UART0->RXD);}/* XXX:Add interrupt and WFE to save power */void uart_putchar(uint8_t cr){
	NRF_UART0->TXD = (uint8_t) cr;	while(NRF_UART0->EVENTS_TXDRDY != 1){
		//__WFE();
	}
	NRF_UART0->EVENTS_TXDRDY = 0;
}

uint32_t
_write(int fd, char * str, int len){
	for (uint32_t i = 0; i < len; i++){
		uart_putchar(str[i]);
	}
	return len;
}
void set_rx_handler (void (*handler) (uint8_t * ptr) ){
	rx_handler = handler;
}

void uart_init(){
	/* Make rx_handler NULL, configure it with set_rx_handler */
	rx_handler = NULL;

	/* Configure TX and RX pins from board.h */
	nrf_gpio_cfg_output(TX_PIN_NUMBER);
	nrf_gpio_cfg_input(RX_PIN_NUMBER, NRF_GPIO_PIN_NOPULL);
	NRF_UART0->PSELTXD = TX_PIN_NUMBER;
	NRF_UART0->PSELRXD = RX_PIN_NUMBER;

	/* Configure CTS and RTS pins if HWFC is true in board.h */
	if(HWFC){
		nrf_gpio_cfg_output(RTS_PIN_NUMBER);
		nrf_gpio_cfg_input(CTS_PIN_NUMBER, NRF_GPIO_PIN_NOPULL);
		NRF_UART0->PSELRTS = RTS_PIN_NUMBER;
		NRF_UART0->PSELCTS = CTS_PIN_NUMBER;
		NRF_UART0->CONFIG = (UART_CONFIG_HWFC_Enabled << UART_CONFIG_HWFC_Pos);
	}

	/* Configure other UART parameters, BAUD rate is defined in uart_nrf.h	*/
	NRF_UART0->BAUDRATE = (UART_BAUDRATE << UART_BAUDRATE_BAUDRATE_Pos);
	NRF_UART0->ENABLE = (UART_ENABLE_ENABLE_Enabled << UART_ENABLE_ENABLE_Pos);
	NRF_UART0->EVENTS_RXDRDY = 0;

	// Enable UART RX interrupt only
	NRF_UART0->INTENSET = (UART_INTENSET_RXDRDY_Set << UART_INTENSET_RXDRDY_Pos);

	NVIC_SetPriority(UART0_IRQn, 4);
	NVIC_EnableIRQ(UART0_IRQn);

	/* Start reception and transmission */
	NRF_UART0->TASKS_STARTTX = 1;
	NRF_UART0->TASKS_STARTRX = 1;
}
