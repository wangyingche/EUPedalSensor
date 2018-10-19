#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "app_uart.h"
#include "armRF.h"
#include "nrf_log.h"

#define PACKET_STATE_FIRST_HEADBYTE		0
#define PACKET_STATE_SECOND_HEADBYTE	1
#define PACKET_STATE_DATA_RECEIVE		2

static uint8_t armrf_polling_enable;
//static uint16_t armrf_window;

/* client to arm functions*/
int armrf_sendbytes (uint8_t *packet, uint8_t size)
{
	int i;
	for (i=0;i<size;i++) {
		app_uart_put(*packet++);
	}
	return 0;
}

int armrf_packetparser (uint8_t byte)
{
	static uint8_t state;
	static uint8_t packet[ARMRF_PACKET_LENGTH], index;
	switch (state) {
	case PACKET_STATE_FIRST_HEADBYTE:
		if ( byte == 0xa5 ) {
			index = 0;
			state = PACKET_STATE_SECOND_HEADBYTE; 
			packet[index++] = byte;
		}else {
			;//NRF_LOG_INFO ("*");
		}
		break;
	case PACKET_STATE_SECOND_HEADBYTE:
		if ( byte == 0xa6 ) {
			state = PACKET_STATE_DATA_RECEIVE;
			packet[index++] = byte;
		}
		else
			state = PACKET_STATE_FIRST_HEADBYTE;
		break;
	case PACKET_STATE_DATA_RECEIVE:
		packet[index++] = byte;
		if (index >=ARMRF_PACKET_LENGTH) {
			armrf_commandparser (packet);
			state = PACKET_STATE_FIRST_HEADBYTE;
		}
		break;
	default: break;
	}
	return 0;
}

int armrf_commandparser (uint8_t *packet)
{
	if ( packet[0] == 0xa5 && packet[1] == 0xa6 ) {
		packet[1] |= 1; /* reply packet */
		switch (packet[2]) {
		case ARMRF_CMD_POLLING_ENABLE:
			if (packet[3]&1)
				armrf_polling_enable = 1;
			else	
				armrf_polling_enable = 0;
			//NRF_LOG_INFO ("enable poll %d\n", armrf_polling_enable);
			break;
		case ARMRF_CMD_SET_WINDOW:
			//armrf_window = packet[3];
			break;
		
		default: break;
		}
		armrf_sendbytes (packet, ARMRF_PACKET_LENGTH);
		NRF_LOG_INFO ("\npacket cmd %d\n", packet[2]);
	}
	return 0;
}

int armrf_pollingsend (uint8_t *data)
{
	static uint8_t seq;
	uint8_t packet[ARMRF_PACKET_LENGTH];
	if ( armrf_polling_enable ) {
		packet[0] = 0xa5;
		packet[1] = 0xa6|0x1;
		packet[2] = ARMRF_CMD_POLLING_REPLY;
		packet[3] = seq++;
		memcpy (&packet[4], data, ARMRF_PACKET_DATA_LEGTH);
		armrf_sendbytes (packet, ARMRF_PACKET_LENGTH);
	}
	return 0;
}
