/*
 * rtimer-arch.h
 *
 *  Created on: 19-Feb-2014
 *      Author: prithvi
 */

#ifndef TIMER_ARCH_H_
#define TIMER_ARCH_H_

#include "nrf.h"
#include "clock-nrf.h"

/* For the RTIMER clock. Timer1 is used in nrf51822*/
#define TIMER_PRESCALER 	0
#define TIMER_BITSIZE TIMER_BITMODE_BITMODE_08Bit
#if TIMER_BITSIZE == TIMER_BITMODE_BITMODE_08Bit
#define TIMER_COMPARE_FREQ	256
#endif
#define RTIMER_ARCH_SECOND (((HFCLK_FREQUENCY)/(1<<TIMER_PRESCALER))/TIMER_COMPARE_FREQ)

void rtimer_arch_init(void);

#endif /* TIMER_ARCH_H_ */
