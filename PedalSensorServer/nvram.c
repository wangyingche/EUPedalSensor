/*----------------------------------------------------------------------------
 *      
 *----------------------------------------------------------------------------
 *      Name: nvram.c   
 *      Purpose: main entry
 *----------------------------------------------------------------------------
 *      Copyright (c)  BELONENGINEERING
 * 		Author: wyn
 *      DATE: 1/31/2018 
 *---------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "nrf_delay.h"
#include "nvram.h"

#include "nrf_log.h"


#define ELMT_NAME_SIZE	16
#define ELMT_DATA_SIZE	16
typedef struct __element_t{
	uint8_t name[ELMT_NAME_SIZE];
	uint8_t data[ELMT_DATA_SIZE];
}element_t;

#define MAX_ELEMENTS	5
typedef struct __nvinfo_t {
	element_t ver;
	element_t sn1;
	element_t sn2;
	element_t reserve[MAX_ELEMENTS-2];
}nvinfo_t;
nvinfo_t nvram;

const nvinfo_t nvram_default = 
{
	{"ver", "NVRAM0000"},
	{"sn1","0418030145"},
	{"sn2","0000000002"},
	{"sn3","0000000003"}
};

#define NVRAM_ADDRESS	0x7f000
//#define SIM					

uint8_t *nvram_getSN(void)
{
	return nvram.sn1.data;
}

uint8_t *nvram_getVer(void)
{
	return nvram.ver.data;
}

static void flash_page_erase(uint32_t * page_address)
{
#ifdef SIM
	NRF_LOG_INFO("flash_page_erase address=%x\n", page_address);
#else
    // Turn on flash erase enable and wait until the NVMC is ready:
    NRF_NVMC->CONFIG = (NVMC_CONFIG_WEN_Een << NVMC_CONFIG_WEN_Pos);

    while (NRF_NVMC->READY == NVMC_READY_READY_Busy)
    {
        // Do nothing.
    }

    // Erase page:
    NRF_NVMC->ERASEPAGE = (uint32_t)page_address;

    while (NRF_NVMC->READY == NVMC_READY_READY_Busy)
    {
        // Do nothing.
    }

    // Turn off flash erase enable and wait until the NVMC is ready:
    NRF_NVMC->CONFIG = (NVMC_CONFIG_WEN_Ren << NVMC_CONFIG_WEN_Pos);

    while (NRF_NVMC->READY == NVMC_READY_READY_Busy)
    {
        // Do nothing.
    }
#endif	
}


/** @brief Function for filling a page in flash with a value.
 *
 * @param[in] address Address of the first word in the page to be filled.
 * @param[in] value Value to be written to flash.
 */
static void flash_word_write(uint32_t * address, uint32_t value)
{
#ifdef SIM
	NRF_LOG_INFO("flash_word_write addr=%x value=%x\n", address, value);
	nrf_delay_ms(100);
#else	
    // Turn on flash write enable and wait until the NVMC is ready:
    NRF_NVMC->CONFIG = (NVMC_CONFIG_WEN_Wen << NVMC_CONFIG_WEN_Pos);

    while (NRF_NVMC->READY == NVMC_READY_READY_Busy)
    {
        // Do nothing.
    }

    *address = value;

    while (NRF_NVMC->READY == NVMC_READY_READY_Busy)
    {
        // Do nothing.
    }

    // Turn off flash write enable and wait until the NVMC is ready:
    NRF_NVMC->CONFIG = (NVMC_CONFIG_WEN_Ren << NVMC_CONFIG_WEN_Pos);

    while (NRF_NVMC->READY == NVMC_READY_READY_Busy)
    {
        // Do nothing.
    }
#endif	
}


void nv_dump (nvinfo_t *nv)
{
	int i;
	char name[ELMT_NAME_SIZE];
	char data[ELMT_NAME_SIZE];
	element_t *elmt = (element_t *)nv;
	printf("dump address=h%x\n", nv);
	for (i=0;i<sizeof(nvinfo_t)/sizeof(element_t);i++, elmt++) {
		memcpy(name, elmt->name, ELMT_NAME_SIZE); 
		name[ELMT_NAME_SIZE-1]=0;	
		memcpy(data, elmt->data, ELMT_NAME_SIZE); 
		data[ELMT_NAME_SIZE-1]=0;		
		printf ("%s %s\n", name, data);
		nrf_delay_ms(10);
	}
	NRF_LOG_INFO("\n");
}

void nvram_dump (void)
{
	nv_dump (&nvram);
	nv_dump ((nvinfo_t *)NVRAM_ADDRESS);
}


void nvram_init (int factory)
{
	nvinfo_t *p_nvram_code = (nvinfo_t *)NVRAM_ADDRESS;
#if 0
	if (factory)
		flash_page_erase ((uint32_t *)NVRAM_ADDRESS);
	if ((0 != strcmp((char *)p_nvram_code->ver.name, "ver") )) {
		NRF_LOG_INFO ("program default data into nvram\n");
		memcpy (&nvram, &nvram_default, sizeof(nvram));
		nvram_prog ();
	} else {
		NRF_LOG_INFO ("read data from nvram\n");
		memcpy (&nvram, p_nvram_code, sizeof(nvram)); 
	}
#else
	memcpy (&nvram, &nvram_default, sizeof(nvram));
#endif	
	nvram_dump ();
}

element_t *element_find(nvinfo_t *nv, uint8_t *name)
{
	int i;
	element_t *elmt = (element_t *)nv;
	for (i=0;i<sizeof(nvinfo_t)/sizeof(element_t);i++, elmt++) {
		if ( 0 == strcmp((char *)elmt->name, (char *)name) ) {
			NRF_LOG_INFO("elmt %s is found\n", elmt->name);
			return elmt;
		}	
	}
	NRF_LOG_INFO("element not found\n");
	return (element_t *)0;
}

int nvram_write (uint8_t *name, uint8_t *data)
{
	element_t *elmt;
	elmt = element_find(&nvram, name);
	if (elmt != 0) {
		memcpy(elmt->data, data, ELMT_DATA_SIZE);
		return 0; //successful
	} else {
		return -1;//fail
	}
}

int nvram_prog (void)
{
	int i;
	uint32_t *src, *dst;
	src = (uint32_t *)&nvram;
	dst = (uint32_t *)NVRAM_ADDRESS;
	NRF_LOG_INFO("\nnvram_prog\n");
	flash_page_erase ((uint32_t *)NVRAM_ADDRESS);
	for (i=0;i<sizeof(nvinfo_t)/sizeof(uint32_t);i++) {
		//flash_word_write (dst++, *src++); /*will fail and don't know why */
		flash_word_write (dst, *src);
		dst++;
		src++;
	}
	return 0;
}
