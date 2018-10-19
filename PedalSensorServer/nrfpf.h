#ifndef __NRFPF_H
#define __NRFPF_H
#include <stdint.h>
#include <stdbool.h>

/* nrf packet format: A5 C0|tid<<2|c|r|e len remaining */
#define NRFPF_1TH_HEADBYTE		0xA5
#define NRFPF_2TH_HEADBYTE		0xC0
#define NRFPF_2TH_HEADBYTE_MASK	0xF0

#define NRFPF_MAX_PACKET_SIZE	32	/* change is allowable */
#define NRFPF_NUMBYTE_HEAD		2
#define NRFPF_NUMBYTE_LENGTH	1
#define NRFPF_NUMBYTE_COMMAND	1
#define NRFPF_NUMBYTE_LENGTH_REMAINING	(NRFPF_MAX_PACKET_SIZE-NRFPF_NUMBYTE_HEAD)
#define NRFPF_NUMBYTE_REMAINING	(NRFPF_MAX_PACKET_SIZE-NRFPF_NUMBYTE_HEAD-NRFPF_NUMBYTE_LENGTH)
#define NRFPF_NUMBYTE_DATA		(NRFPF_NUMBYTE_REMAINING-NRFPF_NUMBYTE_COMMAND)

/* ptype definition */
#define PTYPE_MASK				3
#define PTYPE_COMMAND			0
#define PTYPE_REPLY				1
#define PTYPE_EVENT				2
#define PTYPE_UNKNOWN			3
#define PACKET_READY			0x80

#define REPLY_PACKET_READY				(PACKET_READY|PTYPE_REPLY)
#define EVENT_PACKET_READY				(PACKET_READY|PTYPE_EVENT)
#define COMMAND_PACKET_READY			(PACKET_READY|PTYPE_COMMAND)
#define UNKNOWN_PACKET_READY			(PACKET_READY|PTYPE_UNKNOWN)

/* support commands */
#define NRFCMD_MIDGATE_LOOPTEST				0x10
#define NRFCMD_MIDGATE_SET_SERIALNUMBER		0x11
#define NRFCMD_SERVER_LOOPTEST				0x20
#define NRFCMD_SERVER_ENABLE_NOTIFY			0x21
/* support event */
#define NRFEVT_MIDGATEEVT_SERVER_ONLINE		0x40
#define NRFEVT_SERVEREVT_POS_NOTIFY			0x41



#define NRFCMD_NOBLOCK			0

#endif //__NRFPF_H
