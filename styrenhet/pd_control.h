/*
 * FILNAMN:       pd_control.h
 * PROJEKT:       Mazeter
 * PROGRAMMERARE: Mattias Fransson
 *				  Herman Ekwall
 * DATUM:         2013-04-22
 *
 */

#ifndef PD_CONTROL_H_
#define PD_CONTROL_H_

#include "sensor_data.h"
void sensorDataToControlSignal(const SensorData* current, const SensorData* previous);

#endif /* PD_CONTROL_H_ */