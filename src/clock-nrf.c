/*
 * clock.c
 *
 *  Created on: 30-Jan-2014
 *      Author: prithvi
 */

#include "clock-nrf.h"


//#define TICK

static volatile uint32_t current_seconds = 0;

#ifdef TICK
static volatile clock_time_t current_clock = 0;
static volatile unsigned int second_countdown = CLOCK_SECOND;
#else
static volatile uint32_t ovr = 0, offset = 0;
#endif

/** @brief Function for configuring the RTC increments at a freq of to (CLOCK_SECOND) Hz and COMPARE0 to 1 sec.
 */
void
rtc_init(void)
{
  NRF_RTC1->PRESCALER = COUNTER_PRESCALER;      // Set prescaler to a TICK of RTC_FREQUENCY.

#ifdef TICK
  // Enable TICK event and TICK interrupt:
  NRF_RTC1->EVTENSET = RTC_EVTENSET_TICK_Msk;
  NRF_RTC1->INTENSET = RTC_INTENSET_TICK_Msk;
#else
  // Enable overflow event and overflow interrupt:
  NRF_RTC1->EVTENSET      = RTC_EVTENSET_OVRFLW_Msk;;
  NRF_RTC1->INTENSET      = RTC_INTENSET_OVRFLW_Msk;
#endif

  NVIC_EnableIRQ(RTC1_IRQn);    // Enable Interrupt for RTC1 in the core.

  NRF_RTC1->TASKS_START = 1;

}

/** @brief Function to return the current_clock
 */
clock_time_t
nrf_clock_time(void)
{
#ifdef TICK
  return current_clock;
#else
  return NRF_RTC1->COUNTER;
#endif
}

/** @brief Function to return the current_seconds
 */
unsigned long
nrf_clock_seconds(void)
{
#ifndef TICK
	current_seconds = offset + ((ovr + NRF_RTC1->COUNTER) / CLOCK_SECOND);
#endif
  return current_seconds;
}

/** @brief Function to set the current_seconds
 */
void
nrf_clock_set_seconds(unsigned long sec)
{
#ifdef TICK
	current_seconds = sec;
#else
  offset = sec - ((ovr + NRF_RTC1->COUNTER) / CLOCK_SECOND);
#endif
}

#ifndef TICK
/*@brief Function which sends the expiration time
 *
 */
void
nrf_clock_update_expiration_time(clock_time_t expiration_time)
{
  // Enable COMPARE1 event and COMPARE1 interrupt:
  NRF_RTC1->EVTENSET      = RTC_EVTENSET_COMPARE1_Msk;
  NRF_RTC1->INTENSET      = RTC_INTENSET_COMPARE1_Msk;
  // Compare1 interrupt at expiration time
  NRF_RTC1->CC[1]         = expiration_time;
  NRF_RTC1->EVENTS_COMPARE[1] = 0;
}
#endif

/** @brief: Function for handling the RTC1 interrupts.
 * Triggered on TICK or OVERFLOW (#ifdef TICK)
 */
void
RTC1_IRQHandler()
{
#ifdef TICK
  NRF_RTC1->EVENTS_TICK = 0;
  current_clock++;

#if 0
  if(etimer_pending()){
    if(etimer_next_expiration_time() <= current_clock) {
      etimer_request_poll();
      /* printf("%d,%d\n", clock_time(),etimer_next_expiration_time   ()); */
    }
  }
#endif

  if(--second_countdown == 0) {
    current_seconds++;
    second_countdown = CLOCK_SECOND;
  }
#if 1
  if(current_seconds & 0x01)
    leds_on(LEDS_GREEN);
  else
	leds_off(LEDS_GREEN);
#endif  NRF_RTC1->EVENTS_OVRFLW = 0;

#else
  if(NRF_RTC1->EVENTS_OVRFLW == 1){
	  NRF_RTC1->EVENTS_OVRFLW = 0;
	  ovr += 16777216;
  }
  if(NRF_RTC1->EVENTS_COMPARE[1] == 1){
	  NRF_RTC1->EVENTS_COMPARE[1] = 0;
	  // Disable COMPARE1 event and COMPARE1 interrupt:
	  NRF_RTC1->EVTENCLR      = RTC_EVTENSET_COMPARE1_Msk;
	  NRF_RTC1->INTENCLR      = RTC_INTENSET_COMPARE1_Msk;
	  printf("poll\n");
	  //etimer_request_poll();
  }
#endif
}
