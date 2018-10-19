#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "winbuf.h"

#include "nrf_delay.h"

#include "nrf_log.h"

//#define ASSERT(v)	if((v)){while(1);}
#define MAX_BUFFER_SIZE	50
static int16_t buffer[MAX_BUFFER_SIZE];
static win_w_buf_t wb;
static int16_t sum;
void posbuf_test (void);
void posbuf_init (void)
{
	sum = 0;
	wb.buf = buffer;
	wb.buf_size = MAX_BUFFER_SIZE;
	wb.win_size = 10;
	ASSERT(0==win_w_init(&wb));
}

int16_t posbuf_add (int16_t obj)
{
	int16_t lst;
	lst = win_w_add (&wb, obj);
	sum = sum + obj - lst;
#if 0
	NRF_LOG_INFO ("\nobj=%d - lst=%d sum=%d", obj, lst, sum);
	nrf_delay_ms (5); /* delay is needed otherwise uart error is present. */
#endif	
	return sum;
}
#if 0
void posbuf_test (void)
{
	//int16_t pattern[] = {1,1,1,0,0,0,-1,-1,-1,0,0,0,-1,-1,-1};
	//int16_t pattern[] = {1,1,1,1,1,1,-1,-1,-1,0,0,0,-1,-1,-1};
	int16_t pattern[] = {1,1,1,1,1,1,0,0,0,0,0,0,-1,-1,-1,0,0,0,0};
	int i, pindex;
	int16_t winpos;
	posbuf_init ();
	pindex = 0;
	for (i=0; i<100;i++) {
		int16_t v;
		v = pattern[pindex++];
		if (pindex>=sizeof(pattern)/sizeof(int16_t)) pindex=0;
		winpos = posbuf_add(v);
		//NRF_LOG_INFO ("\npi=%d, %d", pindex, v);
		//NRF_LOG_INFO("\nwp=%d", winpos);
	}
}
#endif
