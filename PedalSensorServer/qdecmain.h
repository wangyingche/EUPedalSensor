#ifndef __QDECMAIN_H
#define __QDECMAIN_H
#include <stdbool.h>
#include <stdint.h>

typedef struct _qdec_info {
	int16_t raw_pos;
	int16_t win_pos;
	//uint16_t accdblread;
	int8_t rssi;
	int8_t battery_level;
}qdec_info;



//static void qdec_event_handler(nrf_drv_qdec_event_t event)
void qdec_main(void);
void qdec_set (qdec_info *info);
int qdec_get_raw_pos (void);
int qdec_get_win_pos (void);
int get_qdec_event (void);
void qdec_event_clear (void);


#endif	
/* #ifndef __QDECMAIN_H */

