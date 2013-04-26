﻿/*
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

const uint8_t distance1_table[34][2] PROGMEM =
{
	//Fram vänster
{15,136},{16,134},{17,132},{18,130},{19,128},{20,125},{21,122},{22,120},{23,117},{24,115},{25,112},
{26,109},{27,106},{28,103},{29,101},{30,98},{31,96},{32,94},{33,91},{34,89},{35,87},{37,83},{40,77},
{45,69},{50,61},{55,56},{60,52},{65,48},{70,44},{75,41},{80,40},{90,36},{100,32},{110,25}
};

const uint8_t distance2_table[34][2] PROGMEM =
{
	//Fram höger
{15,136},{16,135},{17,132},{18,130},{19,128},{20,126},{21,123},{22,120},{23,118},{24,115},{25,112},
{26,109},{27,105},{28,102},{29,100},{30,97},{31,95},{32,92},{33,89},{34,87},{35,84},{37,80},{40,74},
{45,66},{50,59},{55,53},{60,49},{65,45},{70,42},{75,40},{80,37},{90,33},{100,29},{110,26}
};
const uint8_t distance3_table[34][2] PROGMEM =
{
	//Lång vänster
{15,140},{16,137},{17,133},{18,132},{19,129},{20,128},{21,125},{22,122},{23,121},{24,119},{25,116},
{26,113},{27,110},{28,105},{29,103},{30,100},{31,95},{32,93},{33,91},{34,88},{35,84},{37,80},{40,74},
{45,66},{50,59},{55,53},{60,50},{65,46},{70,43},{75,39},{80,38},{90,33},{100,24},{110,0xFF}
};
const uint8_t distance4_table[34][2] PROGMEM =
{
	//Lång höger
{15,139},{16,138},{17,136},{18,135},{19,136},{20,130},{21,126},{22,122},{23,120},{24,117},{25,113},
{26,110},{27,107},{28,104},{29,102},{30,99},{31,96},{32,94},{33,92},{34,89},{35,87},{37,83},
{40,76},{45,69},{50,58},{55,56},{60,47},{65,43},{70,42},{75,38},{80,38},{90,33},{100,31},{110,29}
};
const uint8_t distance5_table[27][2] PROGMEM =
{
	//Kort vänster
{4,137},{5,117},{6,99},{7,88},{8,77},{9,67},{10,63},{11,57},{12,53},{13,49},{14,46},{15,43},{16,38},
{17,36},{18,34},{19,33},{20,31},{21,29},{22,28},{23,27},{24,26},{25,25},{26,23},{27,21},{28,20},{29,19},{30,18}
};
const uint8_t distance6_table[27][2] PROGMEM =
{
	//Kort höger
{4,120},{5,106},{6,90},{7,81},{8,73},{9,64},{10,59},{11,54},{12,51},{13,49},{14,47},{15,45},{16,44}, //13
{17,43},{18,42},{19,41},{20,39},{21,38},{22,37},{23,36},{24,35},{25,35},{26,34},{27,33},{28,32},{29,32},{30,31} //14
};
const uint8_t distance7_table[34][2] PROGMEM =
{
	//Bak
{15,133},{16,131},{17,128},{18,128},{19,125},{20,122},{21,121},{22,118},{23,114},{24,113},{25,110},
{26,106},{27,104},{28,101},{29,98},{30,96},{31,94},{32,91},{33,89},{34,86},{35,84},{37,80},{40,74},
{45,66},{50,59},{55,54},{60,49},{65,45},{70,42},{75,40},{80,37},{90,33},{100,29},{110,27}
};

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
			return lookUp(raw_value, ELEM_CNT(distance3_table), distance3_table);
			break;
		
		case DISTANCE_4:
			return lookUp(raw_value, ELEM_CNT(distance4_table), distance4_table);
			break;
		
		case DISTANCE_5:
			return lookUp(raw_value, ELEM_CNT(distance5_table), distance5_table);
			break;
			
		case DISTANCE_6:
			return lookUp(raw_value, ELEM_CNT(distance6_table), distance6_table);
			break;
					
		case DISTANCE_7:
			return lookUp(raw_value, ELEM_CNT(distance7_table), distance7_table);
			break;
			
		default:
			break;
	}
	
	return 0;
}


uint8_t lookUp(uint8_t raw_value, uint8_t size, const uint8_t table[][2])
{
	volatile uint8_t i= 0;
	
	if (raw_value > pgm_read_byte_near(&table[0][1]))
	{
		return 0x00; // För nära
	}
	
	
	while(raw_value < pgm_read_byte_near(&table[i][1]))
	{
		i++;
	}
	
	if (i > size)
	{
		return 0xFF; // För stort 
	}
	else if (pgm_read_byte_near(&table[i][1]) == raw_value)
	{
		return pgm_read_byte_near(&table[i][0]);
	}
	else
	{
		uint8_t previous_voltage = pgm_read_byte_near(&table[i - 1][1]);
		uint8_t previous_distance = pgm_read_byte_near(&table[i - 1][0]);
		
		uint8_t next_voltage = pgm_read_byte_near(&table[i][1]);
		uint8_t next_distance = pgm_read_byte_near(&table[i][0]);
		
		volatile float slope = ((float)previous_distance - (float)next_distance)/((float)previous_voltage - (float)next_voltage);
		
		return previous_distance + slope*(raw_value - previous_voltage);
	}
}