/*
 * timer.c
 *
 *  Created on: 19-Feb-2014
 *      Author: prithvi
 */

#include "timer.h"
#include "clock-init.h"


/* Initialize the RTIMER using TIMER0 */
void timer_init(void){
	/* Initialize the HF clock if it is not already running*/
	hfclk_xtal_init();

    NRF_TIMER0->TASKS_STOP	   = 1;                    		// Stop timer.
	NRF_TIMER0->MODE           = TIMER_MODE_MODE_Timer;  	// Set the timer in Timer Mode.
	NRF_TIMER0->PRESCALER      = TIMER_PRESCALER;			// Prescaler 0 produces 16 MHz.
	NRF_TIMER0->BITMODE        = TIMER_BITSIZE;  			// 32 bit mode.
	NRF_TIMER0->TASKS_CLEAR    = 1;                         // clear the task first to be usable for later.

/*	NRF_TIMER0->EVENTS_COMPARE[0]  = 0;

    NRF_TIMER0->CC[0]          = 128;

    // Enable overflow event and overflow interrupt:
    NRF_TIMER0->INTENSET      = TIMER_INTENSET_COMPARE0_Msk;

    NVIC_EnableIRQ(TIMER0_IRQn);    // Enable Interrupt for TIMER0 in the core. */

    NRF_TIMER0->TASKS_START   = 1;                    		// Start timer.
}

/* Function for handling the TIMER0 interrupts.h  */
void TIMER0_IRQHandler(){

}

void printfcomma (uint32_t num) {
    if (num < 1000) {
        printf ("%d", (int)num);
        return;
    }
    printfcomma (num/1000);
    printf (",%03d",(int) num%1000);
}

//inline uint32_t read_clock(void){
//	NRF_TIMER0->TASKS_CAPTURE[3] = 1;
//	return(NRF_TIMER0->CC[3]);
//}
