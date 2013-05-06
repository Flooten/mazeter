/*
 * FILNAMN:       pd_control.h
 * PROJEKT:       Mazeter
 * PROGRAMMERARE: Mattias Fransson
 *				  Herman Ekwall
 * DATUM:         2013-05-02
 *
 */

#ifndef PD_CONTROL_H_
#define PD_CONTROL_H_

#define DEGREES_90 6000
#define DISTANCE_TAPE_TURN 43

#include "pd_control.h"
#include "control_parameters.h"
#include "styrenhet.h"
#include "spi_commands.h"
#include "turn_detection.h"

void straightRegulator(const SensorData* current, const SensorData* previous);
void makeTurn(uint8_t turn);
void handleTape(volatile TurnStack* turn_stack, uint8_t tape);
void lineRegulator(int8_t current_deviation, int8_t previous_deviation);
void driveStraight(uint8_t cm);
void jamesBondTurn(volatile TurnStack* turn_stack);

#endif /* PD_CONTROL_H_ */