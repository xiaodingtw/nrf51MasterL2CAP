#include "simple_uart.h"
#include "leds.h"
#include "nrf.h"
#include "board.h"
#include "clock-nrf.h"
#include "clock-init.h"

static void timer0(void){
	printf("Timer0 works!!\n");
}

static void timer1(void){
	static uint32_t blink = 0;
	blink++;
	printf("val:%d\n", blink);
	leds_toggle(LED_RGB_RED);
}


int
main(void)
{
	hfclk_xtal_init();
	lfclk_init();
	leds_init();
	ms_timer_init();
    simple_uart_config(RTS_PIN_NUMBER, TX_PIN_NUMBER, CTS_PIN_NUMBER, RX_PIN_NUMBER,
    		UART_BAUDRATE_BAUDRATE_Baud115200, 12, HWFC);

    printf("\r\nStart:\r\n");
    start_ms_timer(MS_TIMER0, SINGLE_CALL, RTC_TICKS(7000), &timer0);
    start_ms_timer(MS_TIMER1, REPEATED_CALL, RTC_TICKS(1000), &timer1);

  while(true) {
	  __WFI();
  }
}
