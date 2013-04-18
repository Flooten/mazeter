/*
 * FILNAMN:       timer.h
 * PROJEKT:       Mazeter
 * PROGRAMMERARE: Fredrik Stenmark
 * DATUM:         2013-04-08
 *
 * BESKRIVNING:   
 *          
 */

#ifndef TIMER_H_
#define TIMER_H_

#include <stdint.h>

extern volatile uint8_t timer_internal_ready;
extern volatile uint8_t timer_external_ready;

void timerInit();

#endif /* TIMER_H_ */