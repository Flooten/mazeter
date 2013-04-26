/*
 * FILNAMN:       turn_detection.c
 * PROJEKT:       Mazeter
 * PROGRAMMERARE: Martin Andersson
 *				  Joel Davidsson
 * DATUM:         2013-04-25
 *
 * BESKRIVNING: 
 *
 */

#include "sensor_data.h"
#include "turn_detection.h"
#include "turn_stack.h"
#include "styrenhet.h"
#include "pd_control.h"

void detectTurn(volatile TurnStack* turn_stack)
{
	
	if (current_sensor_data.distance3 > THRESHOLD_CONTACT)
	{
		// Vänster ej kontakt
		if (current_sensor_data.distance1 > THRESHOLD_CONTACT)
		{
			// Fall 2 eller 4
			if (current_sensor_data.distance1 < THRESHOLD_STOP_DEAD_END)
			{
				// Fall 2
				pushTurnStack(turn_stack, newTurnNode(RIGHT_TURN));
				makeTurn(LEFT_TURN);
			}
			// else fortsätt PD-reglera rakt fram (fall 4)
		}
		else if (current_sensor_data.distance1 < THRESHOLD_STOP)
		{
			// Fall 1, 3 eller 7
			if (current_sensor_data.distance4 > current_sensor_data.distance3)
			{
				// Fall 7
				pushTurnStack(turn_stack, newTurnNode(LEFT_TURN));
				makeTurn(RIGHT_TURN);
			}
			else
			{
				// Fall 1 eller 3
				pushTurnStack(turn_stack, newTurnNode(RIGHT_TURN));
				makeTurn(LEFT_TURN);
			}
		}
	} 
	else if (current_sensor_data.distance4 > THRESHOLD_CONTACT)
	{
		// Höger ej kontakt men vänster har
		if (current_sensor_data.distance1 > THRESHOLD_CONTACT)
		{
			// Fall 6 eller 8
			if (current_sensor_data.distance1 < THRESHOLD_STOP_DEAD_END)
			{
				// Fall 6
				pushTurnStack(turn_stack, newTurnNode(LEFT_TURN));
				makeTurn(RIGHT_TURN);
			}
			// else fortsätt PD-reglera rakt fram (fall 8)
		}
		else
		{
			// Fall 5
			pushTurnStack(turn_stack, newTurnNode(LEFT_TURN));
			makeTurn(RIGHT_TURN);
		}
	}
		
}