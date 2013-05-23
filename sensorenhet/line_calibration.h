/*
 * FILNAMN:       line_calibration.h
 * PROJEKT:       Mazeter
 * PROGRAMMERARE: Herman Ekwall
 *                Mattias Fransson
 *
 * DATUM:         2013-05-17
 *
 * BESKRIVNING: Funktioner för att kalibrera linjesensorns tröskelvärden för korrekt detektering av tejp
 *
 */

#ifndef LINE_CALIBRATION_H
#define LINE_CALIBRATION_H

#include "raw_data.h"

extern volatile uint8_t calibrate_line_sensor;

uint8_t calibrateLineSensorTape(const RawLineData* sensor_data);
uint8_t calibrateLineSensorFloor(const RawLineData* sensor_data);
void waitFiveSeconds();

#endif /* LINE_CALIBRATION_H */
