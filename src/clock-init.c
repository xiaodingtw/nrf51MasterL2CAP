/*
 * clock-init.c
 *
 *  Created on: 07-May-2014
 *      Author: prithvi
 */

#include "clock-init.h"
#include "nrf51.h"
#include "nrf51_bitfields.h"
#include "nrf51_deprecated.h"

/** @brief Function starting the LFCLK oscillator. Use the #define in the header file to configure the source.
 */
void
lfclk_init(void)
{
  if(!(NRF_CLOCK->LFCLKSTAT & CLOCK_LFCLKSTAT_STATE_Running)){
	NRF_CLOCK->LFCLKSRC = (SRC_LFCLK << CLOCK_LFCLKSRC_SRC_Pos);

    NRF_CLOCK->INTENSET = CLOCK_INTENSET_LFCLKSTARTED_Msk;
    // Enable wake-up on event
    SCB->SCR |= SCB_SCR_SEVONPEND_Msk;

    NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
	NRF_CLOCK->TASKS_LFCLKSTART = 1;
	/* Wait for the external oscillator to start up. */
    while (NRF_CLOCK->EVENTS_LFCLKSTARTED == 0){
	  __WFE();
    }
    NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
    NVIC_ClearPendingIRQ(POWER_CLOCK_IRQn);
  	NRF_CLOCK->INTENSET = 0;
  }
}

/** @brief Function stopping the LFCLK oscillator.
 */
void
lfclk_deinit(void)
{
  NRF_CLOCK->TASKS_LFCLKSTOP = 1;
}

/** @brief Function starting the HFCLK XTAL 16 MHz crystal oscillator.
 */
void
hfclk_xtal_init(void)
{
  /* Check if 16 MHz crystal oscillator is already running. */
  if (!(NRF_CLOCK->HFCLKSTAT & CLOCK_HFCLKSTAT_SRC_Xtal)){
	  NRF_CLOCK->INTENSET = CLOCK_INTENSET_HFCLKSTARTED_Msk;
	  // Enable wake-up on event
	  SCB->SCR |= SCB_SCR_SEVONPEND_Msk;

	  NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
	  NRF_CLOCK->TASKS_HFCLKSTART = 1;
	  /* Wait for the external oscillator to start up. */
	  while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0){
		__WFE();
	  }
	  NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
	  NVIC_ClearPendingIRQ(POWER_CLOCK_IRQn);
	  NRF_CLOCK->INTENSET = 0;
  }
}

/** @brief Function stopping the HFCLK XTAL 16 MHz crystal oscillator. MCU will run on 16 MHz RC oscillator
 */
void
hfclk_xtal_deinit(void)
{
  NRF_CLOCK->TASKS_HFCLKSTOP = 1;
}

