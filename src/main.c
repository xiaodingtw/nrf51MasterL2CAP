#include "simple_uart.h"
#include "leds.h"
#include "nrf.h"
#include "board.h"
#include "clock-nrf.h"
#include "clock-init.h"
#include "string.h"
#include "appli.h"

int
main(void)
{
	hfclk_xtal_init();
	lfclk_init();
	leds_init();
	ms_timer_init();
	simple_uart_config(RTS_PIN_NUMBER, TX_PIN_NUMBER, CTS_PIN_NUMBER, RX_PIN_NUMBER,
    		UART_BAUDRATE_BAUDRATE_Baud115200, 12, HWFC);
	radio_init();

	/* Enable the low latency mode */
	NRF_POWER->TASKS_CONSTLAT = 1;

    printf("\r\nStart Scanning:\r\n");
    scan_begin();

  while(true) {
	  __WFI();
  }
}
