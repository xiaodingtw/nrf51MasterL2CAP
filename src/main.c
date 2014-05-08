#include "simple_uart.h"
#include "nrf_delay.h"
#include "leds.h"
#include "nrf.h"
#include "board.h"
#include "clock-nrf.h"
#include "rtimer-arch.h"
#include "clock-init.h"

int
main(void)
{
	hfclk_xtal_init();
	lfclk_init();
	leds_init();
    simple_uart_config(RTS_PIN_NUMBER, TX_PIN_NUMBER, CTS_PIN_NUMBER, RX_PIN_NUMBER,
    		UART_BAUDRATE_BAUDRATE_Baud115200, 12, HWFC);

    printf("\r\nStart:\r\n");

  while(true) {
	  __WFI();
	  leds_toggle(LED_RGB_RED);
  }
}
