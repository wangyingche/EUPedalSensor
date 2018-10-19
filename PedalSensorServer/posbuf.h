#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "winbuf.h"
#ifndef __POSBUF_H
#define __POSBUF_H

//#define #define ASSERT(v)	if((v)){while(1);}

void posbuf_init (void);
int16_t posbuf_add (int16_t obj);
void posbuf_test (void);

#endif //__POSBUF_H
