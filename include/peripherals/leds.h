/*
 * leds.h
 *
 *  Created on: 03-Feb-2014
 *      Author: prithvi
 */

#ifndef LEDS_H_
#define LEDS_H_

#include "board.h"
#include "nrf_gpio.h"

/**@brief Function to initialize the LEDS in the PCA10000 board
 *
 */
static __INLINE  void leds_init(void){
  nrf_gpio_cfg_output(LED_RGB_RED);
  nrf_gpio_pin_set(LED_RGB_RED);
  nrf_gpio_cfg_output(LED_RGB_GREEN);
  nrf_gpio_pin_set(LED_RGB_GREEN);
  nrf_gpio_cfg_output(LED_RGB_BLUE);
  nrf_gpio_pin_set(LED_RGB_BLUE);
}

static __INLINE void leds_on(uint32_t led){
	  nrf_gpio_pin_clear(led);
}

static __INLINE void leds_off(uint32_t led){
	  nrf_gpio_pin_set(led);
}

static __INLINE void leds_toggle(uint32_t led){
	  nrf_gpio_pin_toggle(led);
}

#endif /* LEDS_H_ */

