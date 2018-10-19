#include <stdint.h>
#include <string.h>
#include "nrf_error.h"

#include "nrfpf.h"
#include "nrfparser.h"

#include "nrf_log.h"


#define MAX_PACKET_SIZE			NRFPF_MAX_PACKET_SIZE
#define MAX_REMAINING_SIZE		NRFPF_NUMBYTE_REMAINING

#define WAIT_1TH_BYTE			0
#define WAIT_2TH_BYTE			1
#define WAIT_LENGTH_BYTE		2
#define WAIT_REMAINING			3
int g_enable_notify;
uint32_t nrfsendbytes (uint8_t *data, uint16_t data_len);

int IsEnableNotify (void)
{
	return g_enable_notify;
}

//Send Event packet
int NRFSendEventPacket (uint8_t event, uint8_t *data, uint8_t data_len)
{
	int sts;
	uint8_t packet[MAX_PACKET_SIZE];
	packet[0] = NRFPF_1TH_HEADBYTE;
	packet[1] = NRFPF_2TH_HEADBYTE|PTYPE_EVENT;
	packet[2] = NRFPF_NUMBYTE_COMMAND + data_len;
	packet[3] = event;
	memcpy(&packet[4], data, data_len);
	sts = nrfsendbytes(packet, NRFPF_NUMBYTE_HEAD+NRFPF_NUMBYTE_LENGTH+NRFPF_NUMBYTE_COMMAND+data_len);
	return sts;
}

//Send Reply packet
int NRFSendReplyPacket (uint8_t cmd, uint8_t *data, uint8_t data_len)
{
	int sts;
	uint8_t packet[MAX_PACKET_SIZE];
	packet[0] = NRFPF_1TH_HEADBYTE;
	packet[1] = NRFPF_2TH_HEADBYTE|PTYPE_REPLY;
	packet[2] = NRFPF_NUMBYTE_COMMAND + data_len;
	packet[3] = cmd;
	memcpy(&packet[4], data, data_len);
	NRF_LOG_INFO("\nNRFSendReplyPacket:");
	//app_trace_dump(packet, 4+data_len);
	NRF_LOG_HEXDUMP_INFO(packet, 4+data_len);
	sts = nrfsendbytes(packet, NRFPF_NUMBYTE_HEAD+NRFPF_NUMBYTE_LENGTH+NRFPF_NUMBYTE_COMMAND+data_len);
	if (sts != NRF_SUCCESS)
		NRF_LOG_INFO("fail return %x\n", sts);
	return sts;
}


static int NRFCommandParser (uint8_t cmd, uint8_t* data, uint8_t data_len)
{
	switch(cmd) {
	case NRFCMD_SERVER_LOOPTEST:
		NRF_LOG_INFO("\nNRFCMD_SERVER_LOOPTEST\n");
		NRFSendReplyPacket (NRFCMD_SERVER_LOOPTEST, data, data_len);
		break;
	case NRFCMD_SERVER_ENABLE_NOTIFY:
		g_enable_notify = data[0];
		NRF_LOG_INFO("\nNRFCMD_SERVER_ENABLE_NOTIFY %x\n", g_enable_notify);
		NRFSendReplyPacket (NRFCMD_SERVER_ENABLE_NOTIFY, data, data_len);
		break;
	}
	return 0;
}

int NRFPacketParser (uint8_t ch)
{
	static int ss, sscpy;
	static uint8_t length;
	static uint8_t index;
	static uint8_t remaining[MAX_REMAINING_SIZE];
	uint8_t ptype;

	NRF_LOG_INFO(" %x ", ch);
	
	switch(ss) {
	case WAIT_1TH_BYTE:
		if (ch == NRFPF_1TH_HEADBYTE)
			ss = WAIT_2TH_BYTE;
		break;
	case WAIT_2TH_BYTE:
		ptype = ch&PTYPE_MASK;
		if (ptype==PTYPE_COMMAND && (ch&NRFPF_2TH_HEADBYTE_MASK)==NRFPF_2TH_HEADBYTE) {
			ss = WAIT_LENGTH_BYTE;
		}
		else	
			ss = WAIT_1TH_BYTE;
		break;
	case WAIT_LENGTH_BYTE:
		length = ch;
		if (length>MAX_REMAINING_SIZE) 
			length = MAX_REMAINING_SIZE;
		index = 0;
		ss = WAIT_REMAINING;
		break;
	case WAIT_REMAINING:
		remaining[index++] = ch;

		if (index>=length) {
			//app_trace_dump(remaining, length);
			NRFCommandParser(remaining[0], &remaining[1], length-1);
			ss = WAIT_1TH_BYTE;
		}
		break;
	}
	if (sscpy!=ss) 
		NRF_LOG_INFO("\nss:from %x to %x", sscpy, ss);
	sscpy = ss;
	return 0;
}

#if 0
static void nus_data_handler(ble_nus_t * p_nus, uint8_t * p_data, uint16_t length)
{
	for (uint32_t i=0; i<length; i++) {
		nrfpacketparser (p_data[i]);
	}
}
#endif



