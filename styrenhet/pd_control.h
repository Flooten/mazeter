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

#define DEGREES_90 5800 // 5000 i 90% hastiget 
#define DEGREES_180 14000
#define DISTANCE_TAPE_TURN 35
#define DISTANCE_DETECT_TURN 40 

#include "pd_control.h"
#include "control_parameters.h"
#include "styrenhet.h"
#include "spi_commands.h"
#include "turn_detection.h"

void straightRegulator(const SensorData* current, const SensorData* previous);
void makeTurn(uint8_t turn);
void makeTurnTest(uint8_t turn);
void makeTurn180();
void handleTape(TurnStack* turn_stack, uint8_t tape);
void lineRegulator(int8_t current_deviation, int8_t previous_deviation);
void driveStraight(uint8_t cm);
void driveStraightBack(uint8_t cm);
void jamesBondTurn(volatile TurnStack* turn_stack);
uint16_t TIM16_ReadTCNT3();

#endif /* PD_CONTROL_H_ */