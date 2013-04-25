/*
 * FILNAMN:       look_up.h
 * PROJEKT:       Mazeter
 * PROGRAMMERARE: Mattias Fransson
 *				  Herman Ekwall
 * DATUM:         2013-04-18
 *
 */


#ifndef LOOK_UP_TABLE_H_
#define LOOK_UP_TABLE_H_

#include <stdint.h>

uint8_t lookUp(uint8_t raw_value, uint8_t size, const uint8_t table[][2]);
uint8_t lookUpDistance(uint8_t raw_value, uint8_t sensor);

#endif /* LOOK_UP_TABLE_H_ */