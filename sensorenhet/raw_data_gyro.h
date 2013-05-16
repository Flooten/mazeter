/*
 * FILNAMN:       raw_data.h
 * PROJEKT:       Mazeter
 * PROGRAMMERARE: Fredrik Stenmark och Herman Ekwall
 *                
 * DATUM:         2013-05-17
 *
 * BESKRIVNING: Datastructen som används för gyro datan.
 *
 */

#ifndef RAW_DATA_GYRO_H_
#define RAW_DATA_GYRO_H_

#include <stdint.h>

typedef struct  
{
	uint8_t sensor_type;
	uint8_t value;
	uint8_t is_converted;
	uint16_t time; // Antal millisekunder från förra konverteringen (prescaler 8)
} RawDataGyro;


#endif /* RAW_DATA_GYRO_H_ */





