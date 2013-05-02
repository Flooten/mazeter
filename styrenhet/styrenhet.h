/*
 * FILNAMN:       styrenhet.h
 * PROJEKT:       Mazeter
 * PROGRAMMERARE: Mattias Fransson
 *				  Herman Ekwall
 * DATUM:         2013-04-18
 *
 * BESKRIVNING: 
 *
 */


#ifndef STYRENHET_H_
#define STYRENHET_H_

#include "controlsignals.h"
#include "sensor_data.h"
#include "control_parameters.h"
#include "turn_detection.h"
#include "turn_stack.h"

extern volatile uint8_t control_mode_flag;
extern volatile uint8_t current_command;
extern volatile uint8_t throttle;
extern volatile uint8_t abort_flag;

extern volatile ControlSignals control_signals;
extern volatile SensorData current_sensor_data;
extern volatile SensorData previous_sensor_data;
extern volatile ControlParameters control_parameters;

// Funktioner
void parseCommand(uint8_t cmd);
void commandToControlSignal(uint8_t cmd);

#endif /* STYRENHET_H_ */