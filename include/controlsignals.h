/*
 * FILNAMN:       controlsignals.h
 * PROJEKT:       Mazeter
 * PROGRAMMERARE: Fredrik Stenmark
 *                Mattias Fransson
 * DATUM:         2013-04-10
 *
 * BESKRIVNING: Styrsignaler till PWM-styrning
 *
 */


#ifndef CONTROLSIGNALS_H_
#define CONTROLSIGNALS_H_

#include <stdint.h>

typedef struct
{
    uint8_t right_value;
	uint8_t left_value;
	uint8_t right_direction;
	uint8_t left_direction;
	uint8_t claw_value; // claw_value måste vara mellan 0 och 5
} ControlSignals;

#endif /* CONTROLSIGNALS_H_ */
