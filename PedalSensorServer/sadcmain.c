/**
 * Copyright (c) 2014 - 2018, Nordic Semiconductor ASA
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 * 
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 * 
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 * 
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 * 
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
/** @file
 * @defgroup nrf_adc_example main.c
 * @{
 * @ingroup nrf_adc_example
 * @brief ADC Example Application main file.
 *
 * This file contains the source code for a sample application using ADC.
 *
 * @image html example_board_setup_a.jpg "Use board setup A for this example."
 */

#include <Stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "nrf.h"
#include "nrf_drv_saadc.h"
#include "nrf_drv_ppi.h"
#include "nrf_drv_timer.h"
#include "boards.h"
#include "app_error.h"
#include "nrf_delay.h"
#include "app_util_platform.h"
#include "nrf_pwr_mgmt.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "sadcmain.h"
#include "indicator.h"


#define SAMPLES_IN_BUFFER (5+2)
volatile uint8_t state = 1;

static const nrf_drv_timer_t m_timer = NRF_DRV_TIMER_INSTANCE(1); //NRF_TIMER0 will be used if SOFTDEVICE is enabled. .
static nrf_saadc_value_t     m_buffer_pool[2][SAMPLES_IN_BUFFER];
static nrf_ppi_channel_t     m_ppi_channel;
static uint32_t              m_adc_evt_counter;
static nrf_saadc_value_t adc_reading_aved;

void timer_handler(nrf_timer_event_t event_type, void * p_context)
{

}


void saadc_sampling_event_init(void)
{
    ret_code_t err_code;

    err_code = nrf_drv_ppi_init();
    APP_ERROR_CHECK(err_code);

    nrf_drv_timer_config_t timer_cfg = NRF_DRV_TIMER_DEFAULT_CONFIG;
    timer_cfg.bit_width = NRF_TIMER_BIT_WIDTH_32;
    err_code = nrf_drv_timer_init(&m_timer, &timer_cfg, timer_handler);
    APP_ERROR_CHECK(err_code);

    /* setup m_timer for compare event every 500 ms */
    uint32_t ticks = nrf_drv_timer_ms_to_ticks(&m_timer, 500);
    nrf_drv_timer_extended_compare(&m_timer,
                                   NRF_TIMER_CC_CHANNEL0,
                                   ticks,
                                   NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK,
                                   false);
    nrf_drv_timer_enable(&m_timer);

    uint32_t timer_compare_event_addr = nrf_drv_timer_compare_event_address_get(&m_timer,
                                                                                NRF_TIMER_CC_CHANNEL0);
    uint32_t saadc_sample_task_addr   = nrf_drv_saadc_sample_task_get();

    /* setup ppi channel so that timer compare event is triggering sample task in SAADC */
    err_code = nrf_drv_ppi_channel_alloc(&m_ppi_channel);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_ppi_channel_assign(m_ppi_channel,
                                          timer_compare_event_addr,
                                          saadc_sample_task_addr);
    APP_ERROR_CHECK(err_code);
}


void saadc_sampling_event_enable(void)
{
    ret_code_t err_code = nrf_drv_ppi_channel_enable(m_ppi_channel);

    APP_ERROR_CHECK(err_code);
}

int a2d_read (void)
{
	return (int)adc_reading_aved;
}

#define MAX_BATTERY_VOLT	4.1
#define MIN_BATTERY_VOLT	2.6
static uint8_t battery_level;
uint8_t get_battery_level (void)
{
	return battery_level;
}
uint8_t battery_level_read (int a2d_reading)
{
	uint8_t bat_level;
	double bat_volt;
	bat_volt = VCC_IN(a2d_reading);
	if (bat_volt>MAX_BATTERY_VOLT)
		bat_level = 15;
	else if (bat_volt<=2.6)
		bat_level = 0;
	else {
		bat_level = (int)((bat_volt - MIN_BATTERY_VOLT)*10);
	}
	battery_level = bat_level;
	return bat_level;
}

void PowerManage(int reading)
{
	uint32_t err_code;
	if (reading < Volt2Digit(3.59)) {
		
		//turn off indicators;
		indicator_stop ();
		
		// Go to system-off mode (this function will not return; wakeup will cause a reset).
		err_code = sd_power_system_off();
		APP_ERROR_CHECK(err_code);
	}

}

void saadc_callback(nrf_drv_saadc_evt_t const * p_event)
{
    if (p_event->type == NRF_DRV_SAADC_EVT_DONE)
    {
        ret_code_t err_code;
		nrf_saadc_value_t min, max, reading;
        err_code = nrf_drv_saadc_buffer_convert(p_event->data.done.p_buffer, SAMPLES_IN_BUFFER);
        APP_ERROR_CHECK(err_code);

        int i;
		nrf_saadc_value_t sum_of_adc_reading;
        //NRF_LOG_INFO("ADC event number: %d", (int)m_adc_evt_counter);
		min = 0x7fff;
		max = 0;
        for (i = 0, sum_of_adc_reading=0; i < SAMPLES_IN_BUFFER; i++)
        {
			reading = p_event->data.done.p_buffer[i];
			sum_of_adc_reading += p_event->data.done.p_buffer[i];
			if (reading>max) max = reading;
			if (reading<min) min = reading;
            //NRF_LOG_INFO("adc %d", p_event->data.done.p_buffer[i]);
        }
		sum_of_adc_reading -= (max+min);
		adc_reading_aved = (sum_of_adc_reading + (SAMPLES_IN_BUFFER-2)/2)/(SAMPLES_IN_BUFFER-2);
		//NRF_LOG_INFO("min=%d max=%d, adc_read: %d",min, max, adc_reading_aved);
		//NRF_LOG_INFO("adc=%d %5.2f %5.2f volt %d\n", a2d_read(), VADCIN(a2d_read()), VCC_IN(a2d_read()), Volt2Digit(VCC_IN(a2d_read())));
		m_adc_evt_counter++;
		
		/* set indicator */
		SetIndicator(adc_reading_aved);
		/* see if sleep is necessary */
		PowerManage(adc_reading_aved);
    }
}


void saadc_init(void)
{
    ret_code_t err_code;
    nrf_saadc_channel_config_t channel_config =
        NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN0);

    err_code = nrf_drv_saadc_init(NULL, saadc_callback);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_channel_init(0, &channel_config);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_buffer_convert(m_buffer_pool[0], SAMPLES_IN_BUFFER);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_buffer_convert(m_buffer_pool[1], SAMPLES_IN_BUFFER);
    APP_ERROR_CHECK(err_code);

}


extern void idle_state_handle(void);
/**
 * @brief Function for main application entry.
 */
void saadc_main(void)
{
    //uint32_t err_code = NRF_LOG_INIT(NULL);
    //APP_ERROR_CHECK(err_code);

    //NRF_LOG_DEFAULT_BACKENDS_INIT();

    //ret_code_t ret_code = nrf_pwr_mgmt_init();
    //APP_ERROR_CHECK(ret_code);

    saadc_init();
    saadc_sampling_event_init();
    saadc_sampling_event_enable();
    NRF_LOG_INFO("SAADC HAL simple example started.");

    while (0)
    {
        //nrf_pwr_mgmt_run();
        //NRF_LOG_FLUSH();
		nrf_delay_ms (500);
		NRF_LOG_RAW_INFO("*");
		//printf ("*");
		idle_state_handle();
    }
}


/** @} */
