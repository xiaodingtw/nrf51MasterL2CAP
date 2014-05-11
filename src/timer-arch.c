/*
 * timer-arch.c
 *
 *  Created on: 19-Feb-2014
 *      Author: prithvi
 */

#include "timer-arch.h"
#include "clock-nrf.h"
#include "clock-init.h"

/* Initialize the RTIMER using TIMER1 */
void rtimer_arch_init(void){
	/* Initialize the HF clock if it is not already running*/
	hfclk_xtal_init();

	NRF_TIMER1->MODE           = TIMER_MODE_MODE_Timer;  	// Set the timer in Timer Mode.
	NRF_TIMER1->PRESCALER      = TIMER_PRESCALER;			// Prescaler 0 produces 16MHz.
	NRF_TIMER1->BITMODE        = TIMER_BITSIZE;  			// 32 bit mode.
	NRF_TIMER1->TASKS_CLEAR    = 1;                         // clear the task first to be usable for later.

/*	NRF_TIMER1->EVENTS_COMPARE[0]  = 0;

    NRF_TIMER1->CC[0]          = 128;

    // Enable overflow event and overflow interrupt:
    NRF_TIMER1->INTENSET      = TIMER_INTENSET_COMPARE0_Msk;

    NVIC_EnableIRQ(TIMER1_IRQn);    // Enable Interrupt for TIMER1 in the core. */

    NRF_TIMER1->TASKS_START   = 1;                    		// Start timer.
}

/** @brief: Function for handling the TIMER1 interrupts.h
 * Triggered on COMPARE[0]
 */
void
TIMER1_IRQHandler(){

}
