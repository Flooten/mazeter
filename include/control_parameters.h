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
	uint16_t dist_kp;
	uint16_t dist_kd;
	uint16_t line_kp;
	uint16_t line_kd;
} ControlParameters;

#endif /* CONTROL_PARAMETERS_H_ */