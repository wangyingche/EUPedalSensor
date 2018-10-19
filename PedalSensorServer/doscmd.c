/*----------------------------------------------------------------------------
 *      
 *----------------------------------------------------------------------------
 *      Name: doscmd.c   
 *      Purpose: main entry
 *----------------------------------------------------------------------------
 *      Copyright (c)  BELONENGINEERING
 * 		Author: wyn
 *      DATE: 10/02/2012 
 *---------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "app_timer.h"

#include "nrf_delay.h"
#include "nvram.h"
#include "doscmd.h"
#include "nrf_log.h"
#include "qenc.h"

/*************** DEFINITION **********************/
typedef unsigned int U32;
typedef unsigned char U8;	
#define MAX_NUM_OF_COMMAND			5
#define CONSUME						1
#define NOTCONSUME					0

typedef enum
{
	CMD_NORMAL_RUN = 0,
	CMD_GPIO,
	CMD_DUMP,
	CMD_NVRAM,
	CMD_SIM,
	CMD_UNDEFINE,
	CMD_NONE = 0xff
}eCmdId;
typedef enum
{
	CPS_IDLE = 0,
   	CPS_PARSER_COMMAND = 1,
	CPS_CMDQUE_REQUEST = 2,
	CPS_PARSER_DONE = 3
}eCPState;

/*************** VARIABLES ***********************/
#define MAX_RECE_BUF_SIZE			80
static char aReceBuf[MAX_RECE_BUF_SIZE], NumOfByteReceived;
static char *gpCharNow;
//static int fRunCommandParser;
static eCmdId parser_id;
//static eCPState cps_state;
uint8_t data_buffer[1024];

/*************** EXTERN **************************/
//void WHLPARM_FindWithUpdata_Test (void);
/*************** START OF FUNCTION ***************/
void hexdump (char *buf, int size, int inverse)
{
	int i;
	unsigned char ch;
	for (i=0; i<size; i++)
	{
		if ( inverse == 0 )
			ch = buf[i];
		else
			ch = buf[size-1-i];
		NRF_LOG_INFO ("%02X ", ch);
	}
	printf ("\n");
}

void hexdump2 (char *buf, int size, int disp)
{
	int i;
	if (disp) {
	 	for (i=0;i<size;i++){
			if (i%16==0)
				NRF_LOG_INFO ("\n%-3x:", i);
		 	NRF_LOG_INFO ("%02X ", buf[i]);
			if (i%16==7)
				NRF_LOG_INFO ("-");
		}
		NRF_LOG_INFO ("\n");
	}
}

void dumpmem (U32 addr, U32 size)
{
	U32 i;
	int ch;
	for (i=addr; i<addr+size; i++)
	{
		if ( (i % 1000) == 0 )
			nrf_delay_ms(1);
	 	ch = *((U8 *)i);
		NRF_LOG_INFO ("%c",ch);
	}		
}

char *FindWord (char *dst, U8 maxbytes, U8 consume)
{
	U8 idx, len;
	char ch, *pch, *pend;
	pch = gpCharNow;
	pend = aReceBuf + NumOfByteReceived;
	for (idx=0, len=0; len<maxbytes-1 && pch < pend; idx++, pch++ )
	{
		ch = *pch;
		if ( ch == NULL || (ch == ' ' && len != 0) )
		{
				break; /* a symbol is found.*/
		}
		else if ( ch >= 0x20 && ch <= 0x7f)
		{
			dst[len] = ch;
			len++;
		}
	}	
	dst[len] = 0;
	if ( consume )
		gpCharNow += idx+1;
#if 1
	printf ("FindWord:%s..%c\n",dst, dst[0]);
#endif
	return dst;
}

eCmdId FindCmdID (char *str)
{
	eCmdId id = CMD_UNDEFINE;

	if ( 0 == strcmp ("NVRAM", str)
		|| 0 == strcmp ("nvram", str)		
		)
	{
		id = CMD_NVRAM;
	}
	else
	if ( 0 == strcmp ("SIM", str)
		|| 0 == strcmp ("sim", str)		
		)
	{
		id = CMD_SIM;
	}
	else
	if ( 0 == strcmp ("DUMP", str)
		|| 0 == strcmp ("dump", str)		
		)
	{
		id = CMD_DUMP;
	}

	return id;
}
void DOSCommandParser (char ch)
{
	char str[48];
	if ( ch>=0x20 && ch<=0x7e ) {
		if ( NumOfByteReceived < sizeof(aReceBuf)-1 )
			aReceBuf[NumOfByteReceived++] = ch;		
	}
	else if (ch == 0x0d) {
		if ( NumOfByteReceived < sizeof(aReceBuf)-1 )
			aReceBuf[NumOfByteReceived] = 0;
		aReceBuf[sizeof(aReceBuf)-1] = 0;			
		gpCharNow = aReceBuf;
		//fRunCommandParser = 1;
		printf ("<-\n");
		//printf ("...%s One command string is got\n\n", aReceBuf);
		
		/* command parser */
		FindWord (str, sizeof(str), CONSUME);
		if ( 0 == strcmp ("PEDAL", str) 
			|| 0 == strcmp ("pedal", str)
			)
		{
			uint32_t t1, t2, tconsume;
			t1 = app_timer_cnt_get();	

			FindWord (str, sizeof(str), CONSUME);
			parser_id = FindCmdID (str);

			switch (parser_id)
			{
			case CMD_UNDEFINE:
				hexdump (str, 20, 0);
				break;
			case CMD_NORMAL_RUN:
				break;
				/* gpio */
			case CMD_GPIO:
				{
					;
				}
				break;
			case CMD_NVRAM:	  /* pedal nvram dump/write/program name value */
				/* 
					pedal nvram dump 
					pedal nvram write name data
					ebike nvram prog
				*/
				{
					FindWord(str, sizeof(str), CONSUME);
					if (0 == strcmp (str, "dump"))	{
						//erase the last sector, the address is 0x3f000
						nvram_dump();
					}
					/* pedal nvram write name data */
					else
					if (0 == strcmp (str, "write"))	{
						char name[16];
						char data[16];
						FindWord(name, sizeof(name), CONSUME);		
						FindWord(data, sizeof(data), CONSUME);
						NRF_LOG_INFO("name=%s data=%s\n", name, data);
						nvram_write ((uint8_t *)name, (uint8_t *)data);				
					}
					//ebike flash prog sector offset size 	:program flash from data_buffer
					else
					if (0 == strcmp (str, "program"))	{
						nvram_prog ();
					}
					//ebike flash comp sector offset size	:compare sector with buffer. 									else
					else
					if (0 == strcmp (str, "factory"))	{
						nvram_init (1);
					}
				}
				break;
			case CMD_SIM:
				/* pedal sim start/stop minisec */
				{
					uint32_t minisec;
					int start_or_stop = 0;
					FindWord(str, sizeof(str), CONSUME);
					if (0 == strcmp (str, "start") || 0 == strcmp(str, "START") ) {
						//FindWord(str, sizeof(str), CONSUME);
						//minisec = atoi(str);
						minisec = 20;
						start_or_stop = 1;
					} 
					qdec_period_read_timer_init ();
					qdec_period_read_ctrl (start_or_stop, minisec);
				}
				break;
			default:
				printf("Not defined\n");
				break;
			}
			t2 = app_timer_cnt_get(); 
			tconsume = app_timer_cnt_diff_compute (t2, t1);
			NRF_LOG_INFO ("time consumption: %f us\n\n", tconsume );	

		}
		else
		{
			printf ("Wrong title\n");
			hexdump (str, 32, 0);
			printf ("\n");
			//hexdump (aReceBuf,32,0);
			//printf ("\n\nNumOfByteReceived=%d aReceBuf=%x gpCharNow=%x ", NumOfByteReceived, aReceBuf, gpCharNow);
		}
		NumOfByteReceived = 0; 
		//fRunCommandParser = 0;
		
		/* end of command parser */
	}else { //it neither text nor 0x0d.
		NumOfByteReceived = 0; 
		//fRunCommandParser = 0;
	}
}
