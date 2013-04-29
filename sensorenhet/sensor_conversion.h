/*
 * FILNAMN:       sensor_conversion.h
 * PROJEKT:       Mazeter
 * PROGRAMMERARE: Martin Andersson
 *                Herman Ekwall
 *                Mattias Fransson
 *
 * DATUM:         2013-04-17
 *
 * BESKRIVNING:
 *
 */

#ifndef SENSOR_CONVERSION_H_
#define SENSOR_CONVERSION_H_

#include "sensorenhet.h"
#include "raw_data.h"
#include "sensor_names.h"
#include "sensor_data.h"
#include "sensorenhet.h" 
#include "raw_data_gyro.h"

/* ------- Konstanter -------- */
#define NO_LINE 0x01
#define HORIZONTAL_LINE 0x02
#define VERTICAL_LINE 0x03

#define LINE_1 0x01
#define SPACE_1 0x02
#define LINE_2 0x03
#define SPACE_2 0x04
#define LINE_FOLLOWING 0x05

#define NR_OF_GYRO_SAMPLES 10
#define GYRO_REF_LEVEL 128 // 127

#define CONVERSION_ERROR 1;

/* ------- Variabler -------- */

extern uint8_t line_detections;
extern uint8_t no_line_detections;
extern uint8_t current_line;
extern uint8_t first_line_val;
extern uint8_t second_line_val;
extern uint8_t goal_mode;

volatile long int gyro_filtered;
volatile long int gyro_total_time;
volatile long int gyro_samples[10];
volatile int filter_coeff[10];

/* conversion_status */

/* ------ Variabler slut ------- */

void initGYRO();
void convertAllData();
void convertRawData(RawData* data);

void convertDistanceLong(RawData* data);
void convertDistanceShort(RawData* data);
void convertRawDataGyro(volatile RawDataGyro* data);

int8_t calculateCenter(const uint8_t* data);
// calcute_center ger ett värde mellan -5 och 5
uint8_t calculateAverage(const uint8_t* data);
void convertLineData(RawLineData* data);

#endif /* SENSOR_CONVERSION_H_ */