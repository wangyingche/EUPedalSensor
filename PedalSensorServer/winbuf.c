#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "winbuf.h"

#include "nrf_delay.h"

#include "nrf_log.h"

//#define ASSERT(v)	if((v)){while(1);}

int win_w_init (win_w_buf_t *wb)
{
	int i;
	if ( wb->buf == NULL || wb->buf_size == 0 || wb->win_size == 0 || wb->win_size > wb->buf_size)
		return -1;
	for (i=0;i<wb->win_size;i++)
		wb->buf[i] = 0;
	wb->index = 0;
	return 0;
}

int16_t win_w_add (win_w_buf_t *wb, int16_t object)
{
	int16_t lst;
	ASSERT (wb->buf);
	lst = wb->buf[wb->index];
	wb->buf[wb->index++] = object;
	if (wb->index>=wb->win_size)
		wb->index = 0;
#if 0
	NRF_LOG_INFO ("\n%-3d: obj=%-3d lst=%-3d\n", wb->index, object, lst);
	nrf_delay_ms(5);
#endif	

	return lst;
}

#if 0
#define MAX_BUF_SIZE;
static uint16_t buffer[MAX_BUFFER_SIZE];
//#define ASSERT(v)	if((v)){while(1);}
void demo (void)
{
	win_w_buf_t wb;
	wb.buf = buffer;
	wb.buf_size = MAX_BUFFER_SIZE;
	wb.win_size = 10;
	ASSERT(0==win_buf_init(&wb))
		;//fail;
	
	lst = win_w_add (wb, now);
	diff = now-lst;s
}
#endif
