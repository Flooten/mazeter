/*
 * FILNAMN:       control_parameters.h
 * PROJEKT:       Mazeter
 * PROGRAMMERARE: Mattias Fransson
 *				  Herman Ekwall
 * DATUM:         2013-04-18
 *
 */


#ifndef CONTROL_PARAMETERS_H_
#define CONTROL_PARAMETERS_H_

#include <stdint.h>

typedef struct  
{
	uint8_t right_kp;
	uint8_t right_kd;
	uint8_t left_kp;
	uint8_t left_kd;
	uint8_t line_kp;
	uint8_t line_kd;
} ControlParameters;

#endif /* CONTROL_PARAMETERS_H_ */