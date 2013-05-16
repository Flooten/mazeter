/*
 * FILNAMN:       raw_data.h
 * PROJEKT:       Mazeter
 * PROGRAMMERARE: Fredrik Stenmark
 *				  Herman Ekwall
 *				  Mattias Fransson
 *                
 * DATUM:         2013-05-17
 *
 * BESKRIVNING: Datastructen som används för avståndssensorerna samt linjesensorn.
 *
 */

#ifndef RAW_DATA_H_
#define RAW_DATA_H_

#include <stdint.h>

typedef struct  
{
	uint8_t sensor_type;
	uint8_t prev_value;
	uint8_t current_value;
	uint8_t value;
	uint16_t accumulator; // Ackumulator för att beräkna medelvärden av sensorerna mellan två SPI-cykler
	uint8_t arr[7];
	uint8_t is_converted;
} RawData;

typedef struct
{
	uint8_t value[11]; // Samtliga linjesensorers värden
	uint8_t is_converted;
} RawLineData;


#endif /* RAW_DATA_H_ */