#ifndef __ARMRF_H
#define __ARMRF_H
#include <stdint.h>
#include <stdbool.h>

/* a5 5a seq cmd xx xx xx xx*/
#define ARMRF_PACKET_HEAD_LENGTH 	2
#define ARMRF_PACKET_CMD_LENGTH		1
#define ARMRF_PACKET_SUBCMD_LENGTH	1
#define ARMRF_PACKET_DATA_LEGTH		4
#define ARMRF_PACKET_LENGTH		(ARMRF_PACKET_HEAD_LENGTH+ARMRF_PACKET_SUBCMD_LENGTH+ARMRF_PACKET_CMD_LENGTH+ARMRF_PACKET_DATA_LEGTH)

/* A5 A6 seq cmd xx xx xx xx*/
typedef enum __armrf_cmd_t 
{
ARMRF_CMD_POLLING_ENABLE = 0,
ARMRF_CMD_POLLING_REPLY,
ARMRF_CMD_SET_WINDOW,
ARMRF_CMD_END = 16
}armrf_cmd_t;

int armrf_packetparser (uint8_t byte);
int armrf_pollingsend (uint8_t *data);
int armrf_commandparser (uint8_t *packet);

#endif //__ARMRF_H
