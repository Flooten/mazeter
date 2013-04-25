/*
 * FILNAMN:       turn_detection.h
 * PROJEKT:       Mazeter
 * PROGRAMMERARE: Martin Andersson
 *				  Joel Davidsson
 * DATUM:         2013-04-25
 *
 * BESKRIVNING: 
 *
 */


#ifndef TURN_DETECTION_H_
#define TURN_DETECTION_H_

#include "sensor_data.h"
#include "turn_stack.h"


#define LEFT_TURN 1
#define RIGHT_TURN 2
#define STRAIGHT 3

/* Avståndsvärde då man inte längre har kontakt med väggen,
	dvs sväng eller återvändsgränd upptäckt */
#define THRESHOLD_CONTACT 80

/* Avståndsvärde när man ska rotera */
#define THRESHOLD_STOP 30

/* Avståndsvärde när man ska rotera  om det är en återvändsgränd man mäter mot*/
#define THRESHOLD_STOP_DEAD_END 110

void detect_turn(TurnStack* turn_stack);



#endif /* TURN_DETECTION_H_ */