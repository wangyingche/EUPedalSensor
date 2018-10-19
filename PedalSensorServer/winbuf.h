#ifndef __WINBUF_H
#define __WINBUF_H
#include <stdint.h>
#include <stdbool.h>

typedef struct __win_w_buf_t
{
	int16_t *buf;
	uint16_t buf_size;
	uint16_t win_size;
	uint16_t index;
}win_w_buf_t;


int win_w_init(win_w_buf_t *wb);
int16_t win_w_add (win_w_buf_t *wb, int16_t new);
	
#endif //__WINBUF_H	

