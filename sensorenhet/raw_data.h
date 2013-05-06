/*
 * FILNAMN:       raw_data.h
 * PROJEKT:       Mazeter
 * PROGRAMMERARE: Fredrik Stenmark och Herman Ekwall
 *                
 * DATUM:         2013-04-09
 *
 * BESKRIVNING:
 *
 */

#ifndef RAW_DATA_H_
#define RAW_DATA_H_

#include <stdint.h>

typedef struct  
{
	uint8_t sensor_type;
	uint8_t prev_value;
	uint8_t value;
	uint16_t accumulator;
	uint8_t is_converted;
} RawData;

typedef struct
{
	uint8_t value[11];
	uint8_t is_converted;
} RawLineData;


#endif /* RAW_DATA_H_ */