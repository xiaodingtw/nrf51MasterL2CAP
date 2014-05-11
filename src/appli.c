/*
 * appli.c
 *
 *  Created on: 11-May-2014
 *      Author: prithvi
 */

#include "nrf51.h"
#include "nrf51_bitfields.h"
#include "nrf51_deprecated.h"
#include "string.h"
#include "clock-nrf.h"
#include "appli.h"

#define SET_BIT(n)      (1UL << n)
#define MAX_PDU_SIZE    (48UL)

#define SCAN_INTERVAL  	1500 		/* In ms */
#define SCAN_WINDOW  	5000 		/* In ms */

uint8_t pdu[MAX_PDU_SIZE]  =  { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
								0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
								0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

int32_t rssi = 0;

/** @brief: Function for handling the Radio interrupts.
 */
void RADIO_IRQHandler(){
	if(NRF_RADIO->EVENTS_END == 1){
		print_packet();
	}
	if(NRF_RADIO->EVENTS_RSSIEND == 1){
		NRF_RADIO->TASKS_RSSISTOP = 1;
		rssi = -1 * (NRF_RADIO->RSSISAMPLE);
	}
}

static void print_packet(){
//#pragma GCC diagnostic push
//#pragma GCC diagnostic ignored "-Wformat"
	uint32_t channel_used = (NRF_RADIO->FREQUENCY == 2)?37 :
							((NRF_RADIO->FREQUENCY == 26)?38:39);
	printf("Mat:%d\n",(int) NRF_RADIO->RXMATCH);
	printf("AdvCh %d\n", (int) channel_used);
	printf("RSSI %d\n", (int) rssi);
	printf("CRC:%s\n",
		(NRF_RADIO->CRCSTATUS == 1) ? "Y" : "N");
	printf("PDU:0x%02x\n", (pdu[0] >> 4) & 0xF);
	printf("TxA:0x%02x\n", (pdu[0] >> 1) & 0x1);
	printf("RxA:0x%02x\n", pdu[0] & 0x1);
	printf("Len:0x%02x\n", pdu[1] & 0x3F);

	uint8_t i;
	uint8_t f = pdu[1] + 3;
	printf("Payload:");
	for (i = 0; i < f; i++) {
		printf("0x%02X ", pdu[i]);
	}
	printf("CRC:");
	printf("0x%02X ", NRF_RADIO->RXCRC && 0xFF);
	printf("0x%02X ", (NRF_RADIO->RXCRC>>8) && 0xFF);
	printf("0x%02X ", (NRF_RADIO->RXCRC>>16) && 0xFF);
	printf("\r\n");
//#pragma GCC diagnostic pop
}

static void scan_start(){
	memset(pdu, 0, sizeof(pdu));
	uint32_t temp = NRF_RADIO->FREQUENCY;
	temp = (temp == 2)?26 : ((temp == 26)?80:2);
	printf("CH %d\n",(int) temp);
	NRF_RADIO->FREQUENCY = temp;
	NRF_RADIO->DATAWHITEIV = (temp == 2)?37 : ((temp == 26)?38:39);
	NRF_RADIO->EVENTS_READY = 0UL;
	NRF_RADIO->TASKS_RXEN = 1UL;
}

static void scan_end(){
	NRF_RADIO->EVENTS_DISABLED = 0UL;
	NRF_RADIO->TASKS_DISABLE = 1UL;
	while (NRF_RADIO->EVENTS_DISABLED == 0UL);
}

void radio_init(){

    /* Start to configure the RADIO.
     *
     * We clear PCNF0 and CPNF1 registers to use OR operations in the next
     * operations.
     */
    NRF_RADIO->PCNF0 = 0UL;
    NRF_RADIO->PCNF1 = 0UL;

    /* Set RADIO mode to Bluetooth Low Energy. */
    NRF_RADIO->MODE = RADIO_MODE_MODE_Ble_1Mbit << RADIO_MODE_MODE_Pos;

    /* Set transmission power to 0dBm. */
    NRF_RADIO->TXPOWER = RADIO_TXPOWER_TXPOWER_0dBm
                            << RADIO_TXPOWER_TXPOWER_Pos;

    /* Set access address to 0x8E89BED6. This is the access address to be used
     * when send packets in advertise channels.
     *
     * Since the access address is 4 bytes long and the prefix is 1 byte long,
     * we first set the base address length to be 3 bytes long.
     *
     * Then we split the full access address in:
     * 1. Prefix0:  0x0000008E (LSB -> Logic address 0)
     * 2. Base0:    0x89BED600 (3 MSB)
     *
     * At last, we enable reception for this address.
     */
    NRF_RADIO->PCNF1        |= 3UL << RADIO_PCNF1_BALEN_Pos;
    NRF_RADIO->BASE0        = 0x89BED600;
    NRF_RADIO->PREFIX0      = 0x0000008E;
    NRF_RADIO->RXADDRESSES  = 0x00000001;

    /* Enable data whitening. */
    NRF_RADIO->PCNF1 |= RADIO_PCNF1_WHITEEN_Enabled << RADIO_PCNF1_WHITEEN_Pos;

    /* Set maximum PAYLOAD size. */
    NRF_RADIO->PCNF1 |= MAX_PDU_SIZE << RADIO_PCNF1_MAXLEN_Pos;

    /* Configure CRC.
     *
     * First, we set the length of CRC field to 3 bytes long and ignore the
     * access address in the CRC calculation.
     *
     * Then we set CRC initial value to 0x555555.
     *
     * The last step is to set the CRC polynomial to
     * x^24 + x^10 + x^9 + x^6 + x^4 + x^3 + x + 1.
     */
    NRF_RADIO->CRCCNF =     RADIO_CRCCNF_LEN_Three << RADIO_CRCCNF_LEN_Pos |
                            RADIO_CRCCNF_SKIP_ADDR_Skip
                                            << RADIO_CRCCNF_SKIP_ADDR_Pos;
    NRF_RADIO->CRCINIT =    0x555555UL;
    NRF_RADIO->CRCPOLY =    SET_BIT(24) | SET_BIT(10) | SET_BIT(9) |
                            SET_BIT(6) | SET_BIT(4) | SET_BIT(3) |
                            SET_BIT(1) | SET_BIT(0);

    /* Configure header size.
     *
     * The Advertise has the following format:
     * PDU Type(4b) | RFU(2b) | TxAdd(1b) | RxAdd(1b) | Length(6b) | RFU(2b)
     *
     * And the nRF51822 RADIO packet has the following format
     * (directly editable fields):
     * S0 (0/1 bytes) | LENGTH ([0, 8] bits) | S1 ([0, 8] bits)
     *
     * We can match those fields with the Link Layer fields:
     * S0 (1 byte)      --> PDU Type(4bits)|RFU(2bits)|TxAdd(1bit)|RxAdd(1bit)
     * LENGTH (6 bits)  --> Length(6bits)
     * S1 (2 bits)      --> S1(2bits)
     */
    NRF_RADIO->PCNF0 |= (1 << RADIO_PCNF0_S0LEN_Pos) |  /* 1 byte */
                        (6 << RADIO_PCNF0_LFLEN_Pos) |  /* 6 bits */
                        (2 << RADIO_PCNF0_S1LEN_Pos);   /* 2 bits */

    /* Set the pointer to write the incoming packet. */
    NRF_RADIO->PACKETPTR = (uint32_t) pdu;

    /* Copy the BLE override registers from FICR */
    NRF_RADIO->OVERRIDE0 = 	NRF_FICR->BLE_1MBIT[0];
    NRF_RADIO->OVERRIDE1 = 	NRF_FICR->BLE_1MBIT[1];
    NRF_RADIO->OVERRIDE2 = 	NRF_FICR->BLE_1MBIT[2];
    NRF_RADIO->OVERRIDE3 = 	NRF_FICR->BLE_1MBIT[3];
    NRF_RADIO->OVERRIDE4 = 	NRF_FICR->BLE_1MBIT[4];

    /* Set the initial freq to scan as channel 37 */
	NRF_RADIO->FREQUENCY = 2;//ADV_CHANNEL[0];

    /* Configure the shorts
     * READY event and START task
     * ADDRESS event and RSSISTART task
     * END event and DISABLE task
     * */
    NRF_RADIO->SHORTS = RADIO_SHORTS_READY_START_Msk |
    					RADIO_SHORTS_ADDRESS_RSSISTART_Msk |
    					RADIO_SHORTS_END_DISABLE_Msk;

    NRF_RADIO->INTENSET = RADIO_INTENSET_RSSIEND_Msk |
    					  RADIO_INTENSET_END_Msk;

    // Enable Interrupt for RADIO in the core.
    NVIC_SetPriority(RADIO_IRQn, 3);
	NVIC_EnableIRQ(RADIO_IRQn);
}

void scan_interval(void){
	scan_end();
	printf("EndScan\n");
}

void scan_window(void){
	printf("AdvSt\n");

	scan_start();
	start_ms_timer(MS_TIMER1, SINGLE_CALL, RTC_TICKS(SCAN_INTERVAL), &scan_interval);
}

void scan_begin(){
    start_ms_timer(MS_TIMER0, REPEATED_CALL, RTC_TICKS(SCAN_WINDOW), &scan_window);
}
