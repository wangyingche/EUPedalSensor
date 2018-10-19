#ifndef __NRFPARSER_H
#define __NRFPARSER_H
#include <stdint.h>
#include <stdbool.h>
#include "nrfpf.h"

#define NRFNotifyPosInfo(data,data_len)	NRFSendEventPacket(NRFEVT_SERVEREVT_POS_NOTIFY,data,data_len)


/* a5 5a seq cmd xx xx xx xx*/
int NRFPacketParser (uint8_t ch);
int NRFSendEventPacket (uint8_t event, uint8_t *data, uint8_t data_len);
int NRFSendReplyPacket (uint8_t cmd, uint8_t *data, uint8_t data_len);
int IsEnableNotify (void);

#endif //__NRFPARSER_H
