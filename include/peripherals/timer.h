/*
 * timer.h
 *
 *  Created on: 19-Feb-2014
 *      Author: prithvi
 */

#ifndef TIMER_H_
#define TIMER_H_

#include "stdint.h"
#include "stdio.h"

#include "nrf51.h"
#include "nrf51_bitfields.h"
#include "nrf51_deprecated.h"

#define TIMER_PRESCALER 	0
#define TIMER_BITSIZE TIMER_BITMODE_BITMODE_32Bit

#define PRINT_TIME	NRF_TIMER0->TASKS_CAPTURE[3] = 1; \
					printfcomma(NRF_TIMER0->CC[3]/16); \
					printf("us\n")

#define PROFILE_START	NRF_TIMER0->TASKS_CAPTURE[2] = 1;

#define PROFILE_STOP	NRF_TIMER0->TASKS_CAPTURE[3] = 1; \
						printfcomma((NRF_TIMER0->CC[3] - NRF_TIMER0->CC[2])/16);	  \
						printf(",%03d",(int)((((NRF_TIMER0->CC[3] - NRF_TIMER0->CC[2]) & 0x0F)*125)/2)); \
						printf("ns\n")

void timer_init(void);
void printfcomma(uint32_t num);
inline uint32_t read_time_us(void){
	NRF_TIMER0->TASKS_CAPTURE[3] = 1;
	return(NRF_TIMER0->CC[3]/16);
}
#endif /* TIMER_H_ */
