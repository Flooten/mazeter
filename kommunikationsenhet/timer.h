/*
 * FILNAMN:       timer.h
 * PROJEKT:       Mazeter
 * PROGRAMMERARE: Fredrik Stenmark
 * DATUM:         2013-04-08
 *
 * BESKRIVNING:   I denna fil 
 *          
 */

#ifndef TIMER_H_
#define TIMER_H_

#include <stdint.h>

// Variabler
extern volatile uint8_t timer_internal_ready;
extern volatile uint8_t timer_external_ready;

// Funktion
void timerInit();

#endif /* TIMER_H_ */