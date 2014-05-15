#include "uart_nrf.h"
#include "leds.h"
#include "nrf.h"
#include "board.h"
#include "clock-nrf.h"
#include "clock-init.h"
#include "string.h"
#include "appli.h"
#include "timer.h"

//void dummy (uint8_t * ptr){
//	printf("Data sent: %s\n", ptr);
//}

int
main(void)
{
	hfclk_xtal_init();
	timer_init();
	lfclk_init();
	leds_init();
	ms_timer_init();
	uart_init();
	scan_radio_init();

	/* Enable the low latency mode */
	NRF_POWER->TASKS_CONSTLAT = 1;

    printf("Start Scanning:\n");


    scan_begin();

	while(true) {
	  __WFI();
	}
}
