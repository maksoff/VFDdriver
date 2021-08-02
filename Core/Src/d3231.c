/*
 * d3231.c
 *
 *  Created on: Aug 1, 2021
 *      Author: makso
 */

#include "d3231.h"
#include "i2c.h"
#include "main.h"
#include "microrl_cmd.h"
#include "freertos_inc.h"
#include "semphr.h"

#define D3231_ADDRESS (0b1101000 << 1)
uint8_t d3231_mem[19];

uint8_t * d3231_get_time(void)
{
	xSemaphoreTake(muI2CHandle, portMAX_DELAY);
	HAL_I2C_Mem_Read(&hi2c1, D3231_ADDRESS, 0, 1, d3231_mem, 3, 10);
	xSemaphoreGive(muI2CHandle);
	return d3231_mem;
}

uint8_t * d3231_get_temp(void)
{
	xSemaphoreTake(muI2CHandle, portMAX_DELAY);
	HAL_I2C_Mem_Read(&hi2c1, D3231_ADDRESS, 0x11, 1, d3231_mem+0x11, 2, 10);
	xSemaphoreGive(muI2CHandle);
	return d3231_mem+0x11;
}

uint8_t * d3231_get_all(void)
{
	xSemaphoreTake(muI2CHandle, portMAX_DELAY);
	HAL_I2C_Mem_Read(&hi2c1, D3231_ADDRESS, 0, 1, d3231_mem, 19, 100);
	xSemaphoreGive(muI2CHandle);
	return d3231_mem;
}

uint8_t d3231_get_A2M2(void)
{
	return d3231_get_all()[0xB];
}

void d3231_set_A2M2(uint8_t data)
{
	xSemaphoreTake(muI2CHandle, portMAX_DELAY);
	HAL_I2C_Mem_Write(&hi2c1, D3231_ADDRESS, 0xB, 1, &data, 1, 100);
	xSemaphoreGive(muI2CHandle);
}

void d3231_set(uint8_t * arr, bool date)
{
	xSemaphoreTake(muI2CHandle, portMAX_DELAY);
	HAL_I2C_Mem_Write(&hi2c1, D3231_ADDRESS, date<<2, 1, arr, 3, 100);
	xSemaphoreGive(muI2CHandle);
}

