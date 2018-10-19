#include <stdbool.h>
#include <stdint.h>
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "boards.h"
#include "app_error.h"
#include "sdk_errors.h"
#include "app_timer.h"
#include "nrf_drv_clock.h"
#include "app_util_platform.h"
#include "low_power_pwm.h"
#include "nordic_common.h"

#define NRF_LOG_LEVEL	3
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_delay.h"
#include "indicator.h"
#include "sadcmain.h"

//#include "lppwm_led.h"
//#include "app_trace.h"


#define POWER_ON_LED_BIT		0
#define BATTERY_LOW_LED_BIT		1


static low_power_pwm_t low_power_pwm_0;
static low_power_pwm_t low_power_pwm_1;

void pwm_handler(void * p_context)
{
	;
}

/*
	low power pwm led init.
*/
#define ENABLE_LPP_TIMER0 1		//power
#define ENABLE_LPP_TIEMR1 1		//bat_low
void indicator_start (void)
{
    uint32_t err_code;
	//if (start) 
	{
#if ENABLE_LPP_TIMER0		
		err_code = low_power_pwm_start((&low_power_pwm_0), low_power_pwm_0.bit_mask);
		APP_ERROR_CHECK(err_code);
#endif
#if ENABLE_LPP_TIEMR1		
		err_code = low_power_pwm_start((&low_power_pwm_1), low_power_pwm_1.bit_mask);
		APP_ERROR_CHECK(err_code);
#endif		
	}	
}

void indicator_stop (void)
{
    uint32_t err_code;
	err_code = low_power_pwm_stop ((&low_power_pwm_0));
	APP_ERROR_CHECK(err_code);
	err_code = low_power_pwm_stop ((&low_power_pwm_1));
	APP_ERROR_CHECK(err_code);
}

void lppwm_led_init(int start)
{
    uint32_t err_code;
    low_power_pwm_config_t low_power_pwm_config;
	
	//test ();
	
#if ENABLE_LPP_TIMER0
	/* POWER_ON_LED */
    APP_TIMER_DEF(lpp_timer_0);
    low_power_pwm_config.active_high = true;
    low_power_pwm_config.period = APP_TIMER_TICKS(DEFAULT_PERIOD_MS);
    low_power_pwm_config.bit_mask = POWER_ON_LED_MASK; //POWER_ON_LED; //BSP_LED_0_MASK;
    low_power_pwm_config.p_timer_id = &lpp_timer_0;
    low_power_pwm_config.p_port         = NRF_GPIO;
	
    err_code = low_power_pwm_init((&low_power_pwm_0), &low_power_pwm_config, pwm_handler);
    APP_ERROR_CHECK(err_code);
    err_code = low_power_pwm_duty_set(&low_power_pwm_0, DUTY(1000, 100) /*38*/);
    APP_ERROR_CHECK(err_code);
	//NRF_LOG_INFO ("duty(2000,300)=%d\n", DUTY(2000,300));
#endif
	
#if ENABLE_LPP_TIEMR1
    /* BATTERY_LOW_LED */
    APP_TIMER_DEF(lpp_timer_1);
    low_power_pwm_config.active_high = true;
    low_power_pwm_config.period = APP_TIMER_TICKS(DEFAULT_PERIOD_MS);
    low_power_pwm_config.bit_mask = BATTERY_LOW_LED_MASK; //BATTERY_LOW_LED; //BSP_LED_1_MASK;
    low_power_pwm_config.p_timer_id = &lpp_timer_1;
	low_power_pwm_config.p_port         = NRF_GPIO;
    
    err_code = low_power_pwm_init((&low_power_pwm_1), &low_power_pwm_config, pwm_handler);
    APP_ERROR_CHECK(err_code);
    err_code = low_power_pwm_duty_set(&low_power_pwm_1, DUTY(1000, 500) /*64*/);
    APP_ERROR_CHECK(err_code);
#endif

	/* launch pwm */
	if (start) {
#if ENABLE_LPP_TIMER0		
		err_code = low_power_pwm_start((&low_power_pwm_0), low_power_pwm_0.bit_mask);
		APP_ERROR_CHECK(err_code);
#endif
#if ENABLE_LPP_TIEMR1		
		err_code = low_power_pwm_start((&low_power_pwm_1), low_power_pwm_1.bit_mask);
		APP_ERROR_CHECK(err_code);
#endif		
	}	
	
}

/*	low power pwm led duty set:  */
void lppwm_led_cfg_set (ledcfg_t *cfg)
{
	low_power_pwm_0.period = cfg->period[0];
	low_power_pwm_0.duty_cycle = cfg->duty[0];
	low_power_pwm_1.period = cfg->period[1];
	low_power_pwm_1.duty_cycle = cfg->duty[1];
}

void lppwm_led_test (void)
{
	ledcfg_t cfg;
	int period;
	static int ontime=0;
	//for (ontime=0;ontime<=1000;ontime+=100) 
	{
		period = 2000;
		cfg.period[0] = APP_TIMER_TICKS(period);
		cfg.duty[0] = 128; //DUTY(period, ontime); 
		cfg.period[1] = APP_TIMER_TICKS(period);
		cfg.duty[1] = 128; //DUTY(period, ontime); 
		lppwm_led_cfg_set (&cfg);
		NRF_LOG_INFO ("period=%d ontime=%d duty=%d\n", period, ontime, cfg.duty[0]);
		//idle_state_handle ();
		//nrf_delay_ms (4000);
		ontime+=100;
	}
	//nrf_delay_ms (10000);
}

void indicator_init (void)
{
	lppwm_led_init (1);
	//lppwm_led_test ();
	SetIndicator (-1);
}

static batlvl_t batlvl;
/* read battery level and set indicator with corresponding pattern */
void SetIndicator (int reading)
{
	uint32_t period;
	ledcfg_t cfg;
	static batlvl_t batlvl_copy = BATTERY_LEVEL_END;

	if ( reading < 0 ) {
		batlvl = BATTERY_LEVEL_UNDEFINE;
	}
	else
	if ( reading > Volt2Digit(3.75) ) {
		batlvl = BATTERY_LEVEL_NORMAL;
	}
	else 
	if ( reading > Volt2Digit(3.6) ) {
		batlvl = BATTERY_LEVEL_LOW;
	}
	else  
	if ( reading > Volt2Digit(3.0) ) 
	{
		batlvl = BATTERY_LEVEL_WEAK;
	}
	/* this is not necessay since power will shutdown while battery level is less than 3.0 volt. */	
	else {
		batlvl = BATTERY_LEVEL_EMPTY;
	}

	if ( batlvl != batlvl_copy ) {
		switch (batlvl ) {
		case BATTERY_LEVEL_NORMAL:
			period = APP_TIMER_TICKS(2000);
			cfg.period[POWER_ON_LED_BIT] = period;
			cfg.duty[POWER_ON_LED_BIT] = DUTY(2000, 300);
			period = APP_TIMER_TICKS(2000);
			cfg.period[BATTERY_LOW_LED_BIT] = period;
			cfg.duty[BATTERY_LOW_LED_BIT] = DUTY(2000, 0);
			NRF_LOG_INFO ("BATTERY_LEVEL_NORMAL");
			break;
		case BATTERY_LEVEL_LOW:
			period = APP_TIMER_TICKS(2000);
			cfg.period[POWER_ON_LED_BIT] = period;
			cfg.duty[POWER_ON_LED_BIT] = DUTY(2000, 300);
			period = APP_TIMER_TICKS(5000);
			cfg.period[BATTERY_LOW_LED_BIT] = period;
			cfg.duty[BATTERY_LOW_LED_BIT] = DUTY(5000, 500);
			NRF_LOG_INFO ("BATTERY_LEVEL_LOW");
			break;
		case BATTERY_LEVEL_WEAK:
			period = APP_TIMER_TICKS(2000);
			cfg.period[POWER_ON_LED_BIT] = period;
			cfg.duty[POWER_ON_LED_BIT] = DUTY(2000, 0);
			period = APP_TIMER_TICKS(2000);
			cfg.period[BATTERY_LOW_LED_BIT] = period;
			cfg.duty[BATTERY_LOW_LED_BIT] = DUTY(2000, 1000);
			NRF_LOG_INFO ("BATTERY_LEVEL_WEAK");
			break;
		case BATTERY_LEVEL_EMPTY:
			period = APP_TIMER_TICKS(2000);
			cfg.period[POWER_ON_LED_BIT] = period;
			cfg.duty[POWER_ON_LED_BIT] = DUTY(2000, 0);
			period = APP_TIMER_TICKS(2000);
			cfg.period[BATTERY_LOW_LED_BIT] = period;
			cfg.duty[BATTERY_LOW_LED_BIT] = DUTY(2000, 0);
			NRF_LOG_INFO ("BATTERY_LEVEL_EMPTY");
			break;
		case BATTERY_LEVEL_UNDEFINE:
		default:
			period = APP_TIMER_TICKS(200);
			cfg.period[POWER_ON_LED_BIT] = period;
			cfg.duty[POWER_ON_LED_BIT] = DUTY(20, 200);
			period = APP_TIMER_TICKS(200);
			cfg.period[BATTERY_LOW_LED_BIT] = period;
			cfg.duty[BATTERY_LOW_LED_BIT] = DUTY(20, 200);
			NRF_LOG_INFO ("BATTERY_LEVEL_STATE_UNDEFINE");
			break;
		}
#if 0	//turn on all indicatos.
		if(1)	//test. 
		{
			cfg.duty[POWER_ON_LED_BIT] = DUTY(2000, 2000);
			cfg.duty[BATTERY_LOW_LED_BIT] = DUTY(2000, 2000);
		}
#endif		
		batlvl_copy = batlvl;
		lppwm_led_cfg_set (&cfg);
	}
}



