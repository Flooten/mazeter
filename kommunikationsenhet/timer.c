/*
 * FILNAMN:       timer.h
 * PROJEKT:       Mazeter
 * PROGRAMMERARE: Fredrik Stenmark
 * DATUM:         2013-04-08
 *
 * BESKRIVNING:   
 *          
 */

#include <avr/io.h>
#include "timer.h"

volatile uint8_t timer_internal_ready;
volatile uint8_t timer_external_ready;

void timerInit()
{
	TCCR1B |= (1 << WGM12);
	TCCR3B |= (1 << WGM12); // Sätter upp CTC-mode för timer 1 och 3

	TIMSK1 |= (1 << OCIE1A);
	TIMSK3 |= (1 << OCIE3A); // Tillåter CTC interupt

	OCR1A = 2499; // Sätter vad den ska räkna till. 2499 -> begär data 50 ggr/sekund
	//OCR3A = 24999; // Sätter vad den ska räkna till. 24999 -> skicka data 5 ggr/sekund
	OCR3A = 0x1E78; // 1 avbr / sekundend @ 1024 prescaler

	TCCR1B |= ((1 << CS10) | (1 << CS11)); // Sätter prescaler till 64 för timer 1
	//TCCR3B |= ((1 << CS30) | (1 << CS31)); // Sätter prescaler till 64 för timer 3
	TCCR3B |= ((1 << CS30) | (1 << CS32)); // Sätter prescaler till 1024 för timer 3
}