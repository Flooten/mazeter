/*
 * FILNAMN:       sensorenheten.h
 * PROJEKT:       Mazeter
 * PROGRAMMERARE: Martin Andersson
 *
 * DATUM:         2013-04-xx
 *
 * BESKRIVNING:
 *
 */

#ifndef SENSORENHETEN_H_
#define SENSORENHETEN_H_

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "spi.h"
#include "sensor_data.h"
#include "spi_commands.h"
#include "sensor_names.h"
#include "sensor_parameters.h"
#include "sensor_conversion.h"
#include "raw_data.h"
#include "raw_data_gyro.h"

// SPI-variabler
volatile uint8_t* buffer;
volatile uint8_t buffer_size;
volatile uint8_t current_byte;
volatile uint8_t spi_status;

// Sensorvariabler
volatile uint8_t current_sensor;

volatile RawData distance1;
volatile RawData distance2;
volatile RawData distance3;
volatile RawData distance4;
volatile RawData distance5;
volatile RawData distance6;
volatile RawData distance7;
volatile RawLineData line_sensor;
volatile SensorParameters sensor_parameters;
volatile SensorData sensor_data;
volatile SensorData sensor_data_copy;

volatile RawDataGyro gyro_sample1;
volatile RawDataGyro gyro_sample2;
volatile RawDataGyro gyro_sample3;
volatile RawData gyro_temp;

void ioInit();
void initADC();
void startTimer();
uint16_t restartTimer();
void startADC();
void pauseADC();
void resumeADC();
void readLine(uint8_t diod);
void readGyroData();
void readGyroTemp();
void parseCommand(uint8_t cmd);

#endif /* SENSORENHETEN_H_ */