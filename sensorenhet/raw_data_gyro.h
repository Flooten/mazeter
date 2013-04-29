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

#ifndef RAW_DATA_GYRO_H_
#define RAW_DATA_GYRO_H_

#include <stdint.h>

typedef struct  
{
	uint8_t sensor_type;
	uint8_t value;
	uint8_t is_converted;
	uint16_t time; // Antal klockcykler från förra konverteringen
} RawDataGyro;


#endif /* RAW_DATA_GYRO_H_ */





