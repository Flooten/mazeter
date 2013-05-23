/*
 * FILNAMN:       sensordata.h
 * PROJEKT:       Mazeter
 * PROGRAMMERARE: Martin Andersson
 *                Mattias Fransson
 * DATUM:         2013-04-02
 *
 * BESKRIVNING: Omvandlad sensordata
 *
 */

#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H

#include <stdint.h>

typedef struct
{
    uint8_t distance1;
	uint8_t distance2;
	uint8_t distance3;
	uint8_t distance4;
	uint8_t distance5;
	uint8_t distance6;
	uint8_t distance7;
    uint16_t angle;
	int8_t line_deviation;
	uint8_t line_type;
} SensorData;

#endif /* SENSOR_DATA_H */
