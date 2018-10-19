#ifndef __INDICATOR_H
#define __INDICATOR_H

#define LED_RED					7
#define LED_GREEN				12
#define POWER_ON_LED			LED_RED		//LE_GREEN
#define BATTERY_LOW_LED			LED_GREEN	//LED_RED

#define ON_SENSOR				1
#if ON_SENSOR
#define POWER_ON_LED_MASK		(1<<POWER_ON_LED)
#define BATTERY_LOW_LED_MASK 	(1<<BATTERY_LOW_LED)
#define DUTY(period,on)			(uint16_t)(((on)*256)/(period)) //(uint8_t)(((256-on)*256)/(period)) 	
#else
#define POWER_ON_LED_MASK		(BSP_LED_0_MASK)
#define BATTERY_LOW_LED_MASK 	(BSP_LED_1_MASK)
#define DUTY(period,on)			(uint16_t)(((on)*256)/(period))
#endif
#define DEFAULT_PERIOD_MS		2000

typedef enum _batlvl_t {
	BATTERY_LEVEL_NORMAL,
	BATTERY_LEVEL_LOW,
	BATTERY_LEVEL_WEAK,
	BATTERY_LEVEL_EMPTY,
	BATTERY_LEVEL_UNDEFINE,
	BATTERY_LEVEL_END
}batlvl_t;

/*
	low power pwm led duty set:
*/
typedef struct __ledcfg_t {
	uint16_t duty[2];
	uint32_t period[2];
}ledcfg_t;

//void lppwm_led_cfg_set (ledcfg_t *cfg);
void SetIndicator (int reading);
void indicator_init (void);
//void indicator_start (void);
void indicator_stop (void);
#endif /* __INDICATOR_H */

