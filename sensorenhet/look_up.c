/*
 * FILNAMN:       look_up.h
 * PROJEKT:       Mazeter
 * PROGRAMMERARE: Mattias Fransson
 *				  Herman Ekwall
 * DATUM:         2013-04-18
 *
 */

#include "look_up.h"
#include "sensor_names.h"
#include <avr/pgmspace.h> 

#define ELEM_CNT(x)  (sizeof(x) / sizeof(x[0]))

const uint8_t distance1_table[][] PROGMEM =
{
	{136.15}, {.}
};

const uint8_t distance2_table[256] PROGMEM =
{0};
const uint8_t distance3_table[256] PROGMEM =
{0};
const uint8_t distance4_table[256] PROGMEM =
{0};
const uint8_t distance5_table[256] PROGMEM =
{0};
const uint8_t distance6_table[256] PROGMEM =
{0};
const uint8_t distance7_table[256] PROGMEM =
{0};

uint8_t lookUpDistance(uint8_t raw_value, uint8_t sensor)
{
	switch(sensor)
	{
		case DISTANCE_1:
			return lookUp(raw_value, ELEM_CNT(distance1_table), distance1_table);
			break;
		
		case DISTANCE_2:
			return lookUp(raw_value, ELEM_CNT(distance2_table), distance2_table);
			break;
		
		case DISTANCE_3:
			return distance3_table[raw_value];
			break;
		
		case DISTANCE_4:
			return distance4_table[raw_value];
			break;
		
		case DISTANCE_5:
			return distance5_table[raw_value];
			break;
			
		case DISTANCE_6:
			return distance6_table[raw_value];
			break;
					
		case DISTANCE_7:
			return distance7_table[raw_value];
			break;
			
		default:
			break;
	}
	
	return 0;
}

uint8_t lookUp(uint8_t raw_value, uint8_t size, const uint8_t table[][2])
{
	volatile uint8_t i= 0;
	
	while(raw_value < pgm_read_byte_near(&table[i][0]))
	{
		i++;
	}
	
	if (i > size)
	{
		return 0xFF;
	}
	else if (pgm_read_byte_near(&table[i][0]) == raw_value)
	{
		return pgm_read_byte_near(&table[i][1]);
	}
	else
	{
		uint8_t previous_voltage = pgm_read_byte_near(&table[i - 1][0]);
		uint8_t previous_distance = pgm_read_byte_near(&table[i - 1][1]);
		
		uint8_t next_voltage = pgm_read_byte_near(&table[i][0]);
		uint8_t next_distance = pgm_read_byte_near(&table[i][1]);
		
		volatile float slope = ((float)previous_distance - (float)next_distance)/((float)previous_voltage - (float)next_voltage);
		
		uint8_t res = previous_distance + slope*(raw_value - previous_voltage);
		return res;
	}
};
