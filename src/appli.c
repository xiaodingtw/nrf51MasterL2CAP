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
#include "timer.h"

#define DEBUG

#define SCAN_ADRS		{0x11,0x22,0x33,0xE6,0x50,0xD8}

#define SET_BIT(n)      (1UL << n)

#define MAX_PDU_SIZE    (48UL)

#define SCAN_INTERVAL  	1250 		/* In ms */
#define SCAN_WINDOW  	5000 		/* In ms */

static uint8_t pdu[MAX_PDU_SIZE];

static uint8_t packets[10][MAX_PDU_SIZE+12];

static uint8_t scan_res_packet[MAX_PDU_SIZE+12];

static uint8_t rssi = 0;
static uint32_t count = 0;
enum {
	NOT_SCANNING,
	SCAN_NEEDED,
	SCAN_REQ_TO_BE_SENT,
	SCAN_REQ_SENT,
	SCAN_RSP_RECD
}scan_state;

uint32_t adrs_time;

uint8_t scan_address[6];

static void scan_start(void);
static void scan_end(void);
static void collect_packet(void);
static void print_collected(uint8_t * packet_to_print);
static void scan_req(uint8_t * ptr);

/** @brief: Function for handling the Radio interrupts.
 */
void RADIO_IRQHandler(){
	if(NRF_RADIO->EVENTS_ADDRESS == 1){
		NRF_RADIO->EVENTS_ADDRESS = 0;
		adrs_time = read_time_us();
	}

	if(NRF_RADIO->EVENTS_END == 1){

		NRF_RADIO->EVENTS_END = 0;
		collect_packet();


		/* See if scan is needed, channel is 37 or 38, and scan_address is the required address */
		if((scan_state == SCAN_NEEDED) 							  &&
		(NRF_RADIO->FREQUENCY == 2 || NRF_RADIO->FREQUENCY == 26) &&
		(strncmp((const char *)scan_address,(const char *) pdu+2, 6)==0)){

NRF_UART0->TXD = (uint8_t) '@';

			scan_state = SCAN_REQ_TO_BE_SENT;

			uint32_t temp = NRF_RADIO->FREQUENCY;
			temp = (temp == 2)?26 : 80;

			NRF_RADIO->EVENTS_DISABLED = 0UL;
			NRF_RADIO->TASKS_DISABLE = 1UL;
			while (NRF_RADIO->EVENTS_DISABLED == 0UL);
			NRF_RADIO->EVENTS_DISABLED = 0UL;

			NRF_RADIO->FREQUENCY = temp;
			NRF_RADIO->DATAWHITEIV = (temp == 26)?38 : 39;

			NRF_RADIO->SHORTS = RADIO_SHORTS_READY_START_Msk |
								RADIO_SHORTS_END_DISABLE_Msk |
								RADIO_SHORTS_DISABLED_TXEN_Msk;

			NRF_RADIO->TIFS = 150;
			NRF_RADIO->TXADDRESS = 0;

			NRF_RADIO->EVENTS_READY = 0UL;
			NRF_RADIO->TASKS_RXEN = 1UL;
		}
		else if((scan_state == SCAN_REQ_TO_BE_SENT) &&
		(strncmp((const char *)scan_address,(const char *) pdu+2, 6)==0)){

NRF_UART0->TXD = (uint8_t) '#';

			NRF_RADIO->SHORTS = /*RADIO_SHORTS_READY_START_Msk | */
								RADIO_SHORTS_END_DISABLE_Msk |
								RADIO_SHORTS_DISABLED_TXEN_Msk;

			/* Check address type of the advertiser. PDU type is SCAN_REQ */
			if(pdu[0] & 0x40){
				pdu[0] = 0x83;
			}else{
				pdu[0] = 0x03;
			}
			/* Length = 12 i.e. 6 byte Init address and 6 byte Adv address */
			pdu[1] = 0x0C;
			memcpy(pdu+8, pdu+2, 6);
			memcpy(pdu+2, (uint8_t [6]) SCAN_ADRS, 6);

			scan_state = SCAN_REQ_SENT;

		}
		else if(scan_state == SCAN_REQ_SENT){

NRF_UART0->TXD = (uint8_t) '$';

		    NRF_RADIO->EVENTS_DISABLED = 0UL;
			NRF_RADIO->TASKS_DISABLE = 1UL;
			while (NRF_RADIO->EVENTS_DISABLED == 0UL);
		    NRF_RADIO->EVENTS_DISABLED = 0UL;

		    NRF_RADIO->SHORTS = RADIO_SHORTS_READY_START_Msk |
		    					RADIO_SHORTS_ADDRESS_RSSISTART_Msk |
		    					RADIO_SHORTS_END_DISABLE_Msk;

			NRF_RADIO->EVENTS_READY = 0UL;
			NRF_RADIO->TASKS_RXEN = 1UL;

			scan_state = SCAN_RSP_RECD;
		}
		else if(scan_state == SCAN_RSP_RECD){

NRF_UART0->TXD = (uint8_t) '%';

		    NRF_RADIO->EVENTS_DISABLED = 0UL;
			NRF_RADIO->TASKS_DISABLE = 1UL;
			while (NRF_RADIO->EVENTS_DISABLED == 0UL);

			uint32_t temp = NRF_RADIO->FREQUENCY;
			temp = (temp == 26)?2 : 26;
			NRF_RADIO->FREQUENCY = temp;
			NRF_RADIO->DATAWHITEIV = (temp == 26)?37 : 38;

		    NRF_RADIO->SHORTS = RADIO_SHORTS_READY_START_Msk |
		    					RADIO_SHORTS_ADDRESS_RSSISTART_Msk |
		    					RADIO_SHORTS_END_START_Msk;

			NRF_RADIO->EVENTS_READY = 0UL;
			NRF_RADIO->TASKS_RXEN = 1UL;

			memcpy(scan_res_packet,pdu, pdu[1]+3);
			scan_res_packet[MAX_PDU_SIZE+4] = NRF_RADIO->DATAWHITEIV - 64;
			if(NRF_RADIO->CRCSTATUS == 1){
				scan_res_packet[MAX_PDU_SIZE+4] |= 0x80;
			}
			scan_res_packet[MAX_PDU_SIZE+5] = rssi;
		}
	}

	if(NRF_RADIO->EVENTS_RSSIEND == 1){
		NRF_RADIO->EVENTS_RSSIEND = 0;
		NRF_RADIO->TASKS_RSSISTOP = 1;
		rssi = (NRF_RADIO->RSSISAMPLE);
	}
}

static void collect_packet(void){
	/* Collect a max of 10 packets, otherwise reduce scan window */
	if(count < 10){
		memcpy(*(packets + count),pdu, pdu[1]+2);
		uint32_t time = read_time_us();
		memcpy(*(packets + count) + MAX_PDU_SIZE,&time, 4);	//sizeof(time)=4
		packets[count][MAX_PDU_SIZE+4] = NRF_RADIO->DATAWHITEIV - 64;
		if(NRF_RADIO->CRCSTATUS == 1){
			packets[count][MAX_PDU_SIZE+4] |= 0x80;
		}
		packets[count][MAX_PDU_SIZE+5] = rssi;
		packets[count][MAX_PDU_SIZE+6] = (uint8_t) NRF_RADIO->STATE & 0xF;
		packets[count][MAX_PDU_SIZE+7] = (uint8_t) scan_state;
		memcpy(*(packets + count) + MAX_PDU_SIZE + 8,&adrs_time, 4);
		count++;
	}
}

static void print_collected(uint8_t * packet_to_print){
	uint32_t packet_time;
	memcpy(&packet_time,packet_to_print + MAX_PDU_SIZE,4);
	printfcomma(packet_time);
	printf("us ");
	memcpy(&packet_time,packet_to_print + MAX_PDU_SIZE + 8,4);
	printfcomma(packet_time);
	printf("us ");
	printf("C%d ", (int) packet_to_print[MAX_PDU_SIZE+4]&0x7F);
	printf("R:%d ", (int) packet_to_print[MAX_PDU_SIZE+5]);
	printf("%s ",	(packet_to_print[MAX_PDU_SIZE+4]&0x80) ? "Y" : "N");

	printf("S%d%01X ",packet_to_print[MAX_PDU_SIZE+7],packet_to_print[MAX_PDU_SIZE+6]);


	/*  0000 		ADV_IND
		0001 		ADV_DIRECT_IND
		0010 		ADV_NONCONN_IND
		0011 		SCAN_REQ
		0100 		SCAN_RSP
		0101 		CONNECT_REQ
		0110 		ADV_SCAN_IND
		0111-1111 	Reserved 	*/
	printf("P%d ",(int) (packet_to_print[0]) & 0xF);

	/* 1: Random Tx address, 0: Public Tx address */
	printf("T%d ",(int) (packet_to_print[0] >> 6) & 0x1);
	/* 1: Random Rx address, 0: Public Rx address */
	printf("R%d ",(int) (packet_to_print[0] >> 7) & 0x1);
	/*	Length of the data packet */
	printf("L%d ",(int) packet_to_print[1] & 0x3F);
//	printf("\n");

	uint8_t i;
	uint8_t f = packet_to_print[1] + 2;

	for(i = 7; i>2; i--){
		printf("%02X:", packet_to_print[i]);
	}printf("%02X", packet_to_print[2]);
	printf("\n");


	printf("D:");
	for (i = 0; i < f; i++) {
		printf("%02X ", packet_to_print[i]);
	}
	printf("\n");

//	printf("CRC:");
//	printf("0x%02X ", NRF_RADIO->RXCRC && 0xFF);
//	printf("0x%02X ", (NRF_RADIO->RXCRC>>8) && 0xFF);
//	printf("0x%02X ", (NRF_RADIO->RXCRC>>16) && 0xFF);
//	printf("\n");
}

static void scan_start(){

	uint32_t temp = NRF_RADIO->FREQUENCY;
	temp = (temp == 2)?26 : ((temp == 26)?80:2);
	NRF_RADIO->FREQUENCY = temp;
	NRF_RADIO->DATAWHITEIV = (temp == 2)?37 : ((temp == 26)?38:39);

	if(temp == 26){
//		scan_state = SCAN_REQ_TO_BE_SENT;
//
//		NRF_RADIO->SHORTS = RADIO_SHORTS_READY_START_Msk |
//							RADIO_SHORTS_END_DISABLE_Msk;
//
//		NRF_RADIO->TIFS = 150;
//
//		NRF_RADIO->INTENSET = RADIO_INTENSET_ADDRESS_Msk;
//PRINT_TIME;
	}

	NRF_RADIO->EVENTS_READY = 0UL;
	NRF_RADIO->TASKS_RXEN = 1UL;
}

static void scan_end(){
	if(scan_state == SCAN_RSP_RECD){
		scan_state = NOT_SCANNING;

		printf("SCAN RESPONSE:\n");

		print_collected(scan_res_packet);
	}

	NRF_RADIO->EVENTS_DISABLED = 0UL;
	NRF_RADIO->TASKS_DISABLE = 1UL;
	while (NRF_RADIO->EVENTS_DISABLED == 0UL);

	for(uint32_t i = 0; i< count; i++){
		print_collected(*(packets + i));
	}
	count = 0;

}

void scan_radio_init(){

	/* Refresh all the registers in RADIO */
	NRF_RADIO->POWER = 0;
	NRF_RADIO->POWER = 1;

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
     * RxAdd(1b) | TxAdd(1b) | RFU(2b) | PDU Type(4b) | RFU(2b) | Length(6b)
     *
     * And the nRF51822 RADIO packet has the following format
     * (directly editable fields):
     * S0 (0/1 bytes) | LENGTH ([0, 8] bits) | S1 ([0, 8] bits)
     *
     * We can match those fields with the Link Layer fields:
     * S0 (1 byte)      --> PDU Type(4bits)|RFU(2bits)|TxAdd(1bit)|RxAdd(1bit)
     * LENGTH (6 bits)  --> Length(6bits)
     * S1 (0 bits)      --> S1(0bits)
     */
    NRF_RADIO->PCNF0 |= (1 << RADIO_PCNF0_S0LEN_Pos) |  /* 1 byte */
                        (8 << RADIO_PCNF0_LFLEN_Pos) |  /* 6 bits */
                        (0 << RADIO_PCNF0_S1LEN_Pos);   /* 2 bits */

    /* Set the pointer to write the incoming packet. */
    NRF_RADIO->PACKETPTR = (uint32_t) pdu;

    /* Copy the BLE override registers from FICR */
    NRF_RADIO->OVERRIDE0 = 	NRF_FICR->BLE_1MBIT[0];
    NRF_RADIO->OVERRIDE1 = 	NRF_FICR->BLE_1MBIT[1];
    NRF_RADIO->OVERRIDE2 = 	NRF_FICR->BLE_1MBIT[2];
    NRF_RADIO->OVERRIDE3 = 	NRF_FICR->BLE_1MBIT[3];
    NRF_RADIO->OVERRIDE4 = 	NRF_FICR->BLE_1MBIT[4];

    /* Set the initial freq to scan as channel 37 */
	NRF_RADIO->FREQUENCY = 80;//ADV_CHANNEL[3];

    /* Configure the shorts for scanning
     * READY event and START task
     * ADDRESS event and RSSISTART task
     * END event and START task
     * */
    NRF_RADIO->SHORTS = RADIO_SHORTS_READY_START_Msk |
    					RADIO_SHORTS_ADDRESS_RSSISTART_Msk |
    					RADIO_SHORTS_END_START_Msk;

    NRF_RADIO->INTENSET = RADIO_INTENSET_ADDRESS_Msk |
    					  RADIO_INTENSET_RSSIEND_Msk |
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

static void scan_req(uint8_t * ptr){
	int32_t val = strncmp((const char *)ptr,"S_REQ",5);

	if (val == 0){
		scan_state = SCAN_NEEDED;
		/* XXX Get the address from serial input */
		memcpy(scan_address, (uint8_t [6]) {0x0D,0x3B,0xCC,0x72,0x02,0x00}, 6);
	}//00:02:72:CC:3B:0D
}

void scan_begin(){
	set_rx_handler(&scan_req);
	scan_state = NOT_SCANNING;
	start_ms_timer(MS_TIMER0, REPEATED_CALL, RTC_TICKS(SCAN_WINDOW), &scan_window);
}
