#ifndef __SADCMAIN_H
#define __SADCMAIN_H

#define VOLT2READ_FACTOR		93.57143
#define VADCIN(reading)			((double)(reading)/(VOLT2READ_FACTOR*3))
#define VCC_IN(reading)			((double)(reading)/(VOLT2READ_FACTOR))
#define Volt2Digit(volt)		(int)((volt)*VOLT2READ_FACTOR)	


int a2d_read (void);
uint8_t battery_level_read (int a2d_reading);
uint8_t get_battery_level (void);
void saadc_main(void);
void PowerManage(int reading);


#endif /* __SADCMAIN_H */

