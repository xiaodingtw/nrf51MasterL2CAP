/*
 * clock-nrf.h
 *
 *  Created on: 30-Jan-2014
 *      Author: prithvi
 */

#ifndef CLOCK_NRF_H_
#define CLOCK_NRF_H_

#include "nrf.h"
#include "clock-init.h"
#include "leds.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "uart_nrf.h"

#define DIV_ROUNDED(x,y) 			((x + y/2)/y)

/* RTC clock freq from LF clock in Hertz. Changable, but should be usable by RTC_TICKS */
#define RTC_FREQUENCY				32768
/* Check if RTC_FREQUENCY is power of 2 and less than or equal to 32.768 kHz */
#if (!(!(RTC_FREQUENCY & (RTC_FREQUENCY-1)) && RTC_FREQUENCY && (RTC_FREQUENCY<=32768)))
#error RTC_FREQUENCY must be a power of 2 with a maximum frequency of 32768 Hz
#endif
#define RTC_PRESCALER         		((LFCLK_FREQUENCY/RTC_FREQUENCY) - 1) /* f = LFCLK/(prescaler + 1) */

#define RTC_TICKS(ms)				((uint32_t) DIV_ROUNDED( (RTC_FREQUENCY*ms) , 1000) )

typedef enum {
	MS_TIMER0,
	MS_TIMER1,
	MS_TIMER2,
	MS_TIMER3,
	MS_TIMER_MAX
}timer_num;

typedef enum {
		SINGLE_CALL,
		REPEATED_CALL
}timer_mode;

void ms_timer_init(void);
void start_ms_timer(timer_num id, timer_mode mode, uint32_t ticks, void (*handler)(void));
void stop_ms_timer(timer_num id);

#endif /* CLOCK_NRF_H_ */
