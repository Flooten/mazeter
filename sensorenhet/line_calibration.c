/*
 * FILNAMN:       line_calibration.c
 * PROJEKT:       Mazeter
 * PROGRAMMERARE: Herman Ekwall
 *                Mattias Fransson
 *
 * DATUM:         2013-04-17
 *
 */

#include "line_calibration.h"

#ifndef F_CPU
#define F_CPU 8000000UL
#endif
#include <util/delay.h>

volatile uint8_t calibrate_line_sensor = 0;

uint8_t calibrateLineSensorTape(const RawLineData* sensor_data)
{
	uint8_t min_tape_value = 0xFF;
	uint8_t i;
	for (i = 0; i < 11; ++i)
	{
		if (sensor_data->value[i] < min_tape_value)
		{
			min_tape_value = sensor_data->value[i];
		}
	}
	
	return min_tape_value;
}

uint8_t calibrateLineSensorFloor(const RawLineData* sensor_data)
{
	uint8_t max_floor_value = 0x00;
	uint8_t i;
	for (i = 0; i < 11; ++i)
	{
		if (sensor_data->value[i] > max_floor_value)
		{
			max_floor_value = sensor_data->value[i];
		}
	}
	
	return max_floor_value;
}

void waitFiveSeconds()
{
	uint8_t i;
	for (i = 0; i < 167; ++i)
	{
		_delay_ms(30);
	}
}