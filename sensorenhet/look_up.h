/*
 * FILNAMN:       look_up.h
 * PROJEKT:       Mazeter
 * PROGRAMMERARE: Mattias Fransson
 *				  Herman Ekwall
 * DATUM:         2013-05-17
 *
 * BESKRIVNING:   I denna headerfil finns tabellerna för sensorerna.
 *
 */


#ifndef LOOK_UP_TABLE_H_
#define LOOK_UP_TABLE_H_

#include <stdint.h>
#include <avr/pgmspace.h>

extern const uint8_t distance1_table[37][2];
extern const uint8_t distance2_table[37][2];
extern const uint8_t distance3_table[37][2];
extern const uint8_t distance4_table[37][2];
extern const uint8_t distance5_table[27][2];
extern const uint8_t distance6_table[27][2];
extern const uint8_t distance7_table[37][2];

// Omvandlar sensordatan för en sensor
uint8_t lookUp(uint8_t raw_value, uint8_t size, const uint8_t table[][2]);

// Tar reda på vilken sensor som ska omvandlas
uint8_t lookUpDistance(uint8_t raw_value, uint8_t sensor);

#endif /* LOOK_UP_TABLE_H_ */