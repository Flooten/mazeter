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

#define DEGREES_90 8700
#define DISTANCE_TAPE_TURN 53

#include "pd_control.h"
#include "control_parameters.h"
#include "styrenhet.h"
#include "spi_commands.h"
#include "turn_detection.h"

void sensorDataToControlSignal(const SensorData* current, const SensorData* previous);
void makeTurn(uint8_t turn);
void handleTape(volatile TurnStack* turn_stack, uint8_t turn);
void driveStraight(uint8_t cm);

#endif /* PD_CONTROL_H_ */