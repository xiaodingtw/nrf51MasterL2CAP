/*
 * clock-nrf.c
 *
 *  Created on: 30-Jan-2014
 *      Author: prithvi
 */

#include "clock-nrf.h"

struct ms_timer{
	uint32_t timer_mode_t;
	void (*timer_handler)(void);
};

static struct ms_timer ms_timer_t[MS_TIMER_MAX];
/* Timers currently used based on the bit */
static uint32_t ms_timers_status;

void ms_timer_init(void){

	NRF_RTC1->TASKS_STOP = 1;

	for(uint32_t i = MS_TIMER0; i < MS_TIMER_MAX; i++){
		ms_timer_t[i].timer_mode_t = SINGLE_CALL;
		ms_timer_t[i].timer_handler  = NULL;
	}

	ms_timers_status = 0;
	NRF_RTC1->PRESCALER = RTC_PRESCALER;      // Set prescaler to a TICK of RTC_FREQUENCY.
	NVIC_SetPriority(RTC1_IRQn,2);
	NVIC_EnableIRQ(RTC1_IRQn);    // Enable Interrupt for RTC1 in the core.
}

/** Works only for input less than 512000 milli-seconds, or 8.5 min without when RTC_PRESCALER is 0 */
void start_ms_timer(timer_num id, timer_mode mode, uint32_t ticks, void (*handler)(void)){
	//ticks = RTC_TICKS(ticks);
	/* make sure the number of ticks to interrupt is less than 2^24 */
	ticks &= 0xFFFFFF;

	ms_timer_t[id].timer_handler = handler;
	if(mode == SINGLE_CALL){
		ms_timer_t[id].timer_mode_t  = SINGLE_CALL;
	}else{
		ms_timer_t[id].timer_mode_t  = ticks;
	}

	NRF_RTC1->CC[id]	= NRF_RTC1->COUNTER + ticks;

	NRF_RTC1->EVENTS_COMPARE[id] = 0;
	NRF_RTC1->EVTENSET 		= 1 << (RTC_INTENSET_COMPARE0_Pos + id);
	NRF_RTC1->INTENSET 		= 1 << (RTC_INTENSET_COMPARE0_Pos + id);

	if(ms_timers_status == 0){
		lfclk_init();
		NRF_RTC1->TASKS_START = 1;
	}
	ms_timers_status |= 1 << id;
}

void stop_ms_timer(timer_num id){
	ms_timer_t[id].timer_mode_t  = SINGLE_CALL;
	ms_timer_t[id].timer_handler = NULL;
	ms_timers_status &= ~(1 << id);
	NRF_RTC1->EVTENCLR 		= 1 << (RTC_INTENSET_COMPARE0_Pos + id);
	NRF_RTC1->INTENCLR 		= 1 << (RTC_INTENSET_COMPARE0_Pos + id);

	if(ms_timers_status == 0){
		NRF_RTC1->TASKS_STOP = 1;
		lfclk_deinit();
	}
}

/** @brief: Function for handling the RTC1 interrupts.
 * Triggered CC of timer ID
 */
void
RTC1_IRQHandler(){
	for(uint32_t i = MS_TIMER0; i < MS_TIMER_MAX; i++){
		if(NRF_RTC1->EVENTS_COMPARE[i]){
			NRF_RTC1->EVENTS_COMPARE[i] = 0;

			if(ms_timer_t[i].timer_handler != NULL){
				ms_timer_t[i].timer_handler();
			}

			if(ms_timer_t[i].timer_mode_t == SINGLE_CALL){
				stop_ms_timer(i);
			}else{
				NRF_RTC1->CC[i] += ms_timer_t[i].timer_mode_t;
			}
		}
	}
}

