#ifndef __NVRAM_H
#define __NVRAM_H
#include <stdint.h>

void nvram_init (int factory);
void nvram_dump (void);
int nvram_write (uint8_t *name, uint8_t *data);
int nvram_prog (void);
uint8_t *nvram_getSN(void);
uint8_t *nvram_getVer(void);

#endif //__NVRAM_H
