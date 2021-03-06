﻿ /*
 * FILNAMN:       look_up.c
 * PROJEKT:       Mazeter
 * PROGRAMMERARE: Mattias Fransson
 *				  Herman Ekwall
 *				  Joel Davidsson
 *				  Fredrik Stenmark
 *
 * DATUM:         2013-05-17
 *
 */

#include "look_up.h"
#include "sensor_names.h"
#include <avr/pgmspace.h>

// Funktion som räknar ut antal rader.
#define ELEM_CNT(x)  (sizeof(x) / sizeof(x[0]))

// Look-up-tables för avståndsensorerna. Sparade i programminnet
const uint8_t distance1_table[37][2] PROGMEM =
{
	//Fram vänster
{15,136},{16,134},{17,132},{18,130},{19,128},{20,125},{21,122},{22,120},{23,117},{24,115},{25,112},
{26,109},{27,106},{28,103},{29,101},{30,98},{31,96},{32,94},{33,91},{34,89},{35,87},{37,83},{40,77},
{45,69},{50,61},{55,56},{60,52},{65,48},{70,44},{75,41},{80,40},{90,36},{100,32},{110,29},{120,27},{130,24},{140,22}
};
const uint8_t distance2_table[37][2] PROGMEM =
{
	//Fram höger
{15,136},{16,135},{17,132},{18,130},{19,128},{20,126},{21,123},{22,120},{23,118},{24,115},{25,112},
{26,109},{27,105},{28,102},{29,100},{30,97},{31,95},{32,92},{33,89},{34,87},{35,84},{37,80},{40,74},
{45,66},{50,59},{55,53},{60,49},{65,45},{70,42},{75,40},{80,37},{90,33},{100,29},{110,26},{120,24},{130,22},{140,20}
};
const uint8_t distance3_table[37][2] PROGMEM =
{
	//Lång vänster
{15,141},{16,138},{17,135},{18,133},{19,130},{20,127},{21,125},{22,121},{23,118},{24,115},{25,111},
{26,108},{27,105},{28,102},{29,99},{30,95},{31,93},{32,90},{33,88},{34,85},{35,83},{37,78},{40,73},
{45,65},{50,58},{55,53},{60,49},{65,45},{70,42},{75,39},{80,36},{90,32},{100,28},{110,26},{120,24},{130,22},{140,20}
};
const uint8_t distance4_table[37][2] PROGMEM =
{
	//Lång höger
{15,139},{16,138},{17,136},{18,135},{19,136},{20,130},{21,126},{22,122},{23,120},{24,117},{25,113},
{26,110},{27,107},{28,104},{29,102},{30,99},{31,96},{32,94},{33,92},{34,89},{35,87},{37,83},
{40,76},{45,69},{50,58},{55,56},{60,47},{65,43},{70,42},{75,38},{80,38},{90,33},{100,31},{110,28},{120,25},{130,23},{140,21}
};
const uint8_t distance5_table[27][2] PROGMEM =
{
	//Kort vänster
{4,126},{5,106},{6,92},{7,80},{8,71},{9,64},{10,58},{11,53},{12,48},{13,46},{14,42},{15,39},{16,37},
{17,35},{18,33},{19,31},{20,29},{21,27},{22,26},{23,25},{24,24},{25,23},{26,22},{27,21},{28,20},{29,19},{30,18}
};
const uint8_t distance6_table[27][2] PROGMEM =
{
	//Kort höger
{4,133},{5,112},{6,99},{7,88},{8,77},{9,69},{10,63},{11,57},{12,53},{13,49},{14,46},{15,43},{16,39}, //13
{17,37},{18,34},{19,32},{20,30},{21,29},{22,27},{23,26},{24,24},{25,23},{26,22},{27,21},{28,20},{29,19},{30,18} //14
};
const uint8_t distance7_table[37][2] PROGMEM =
{
	//Bak
{15,133},{16,131},{17,128},{18,128},{19,125},{20,122},{21,121},{22,118},{23,114},{24,113},{25,110},
{26,106},{27,104},{28,101},{29,98},{30,96},{31,94},{32,91},{33,89},{34,86},{35,84},{37,80},{40,74},
{45,66},{50,59},{55,54},{60,49},{65,45},{70,42},{75,40},{80,37},{90,33},{100,29},{110,27},{120,23},{130,21},{140,19}
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
	
	// Täcker fallet om roboten är för nära
	if (raw_value > pgm_read_byte_near(&table[0][1]))
	{
		return 0x00; // För nära
	}
	
	// Letar upp vilken värde det är
	while(raw_value < pgm_read_byte_near(&table[i][1]))
	{
		// Kollar att man inte går utanför tabellen
		if (i > size)
		{
			return 0xFF; // För stort
		}
		
		i++;
	}
	
	if (i > size)
	{
		return 0xFF; // För stort 
	}
	else if (pgm_read_byte_near(&table[i][1]) == raw_value)
	{
		// returnera den värde som är tabellen
		return pgm_read_byte_near(&table[i][0]);
	}
	else
	{
		// räknar ut ett värde mellan två värden i tabellen
		uint8_t previous_voltage = pgm_read_byte_near(&table[i - 1][1]);
		uint8_t previous_distance = pgm_read_byte_near(&table[i - 1][0]);
		
		uint8_t next_voltage = pgm_read_byte_near(&table[i][1]);
		uint8_t next_distance = pgm_read_byte_near(&table[i][0]);
		
		volatile float slope = ((float)previous_distance - (float)next_distance)/((float)previous_voltage - (float)next_voltage);
		
		return previous_distance + slope*(raw_value - previous_voltage);
	}
}
