/*
 * FILNAMN:       turn_detection.c
 * PROJEKT:       Mazeter
 * PROGRAMMERARE: Martin Andersson
 *				  Joel Davidsson
 *				  Fredrik Stenmark
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
#include "PWM.h"

uint8_t min(uint8_t x, uint8_t y)
{
	if (x < y)
	{
		return x;
	}
	else
	{
		return y;
	}
}

uint8_t max(uint8_t x, uint8_t y)
{
	if (x > y)
	{
		return x;
	}
	else
	{
		return y;
	}
}

// Högersväng upptäckt
void right_detected(TurnStack* turn_stack)
{
	if (max(current_sensor_data.distance1, current_sensor_data.distance2) <= 80)
	{
		driveStraight(15);
	}
	//else
	//{
		//driveStraight(10); // för att undvika moment fr reglering precis innan sväng
	//}
	pushTurnStack(turn_stack, newTurnNode(LEFT_TURN));
	makeTurn(RIGHT_TURN);	
}

// Vänstersväng upptäckt
void left_detected(TurnStack* turn_stack)
{
	if (max(current_sensor_data.distance1, current_sensor_data.distance2) <= 80)
	{
		driveStraight(15);
	}
	//else
	//{
		//driveStraight(10); // för att undvika moment fr reglering precis innan sväng
	//}
	pushTurnStack(turn_stack, newTurnNode(RIGHT_TURN));
	makeTurn(LEFT_TURN);
}

/* Upptäcker svängar på väg in i labyrinten */
void detectTurnTest(TurnStack* turn_stack)
{
	if (lockDetectTurn == 1)
		return;
	
	if (max(current_sensor_data.distance1, current_sensor_data.distance2) <= 130)
	{
		if (current_sensor_data.distance4 == 255 && current_sensor_data.distance3 != 255 && current_sensor_data.distance6 == 255)
		{ 
			right_detected(turn_stack);
		}
		else if (current_sensor_data.distance3 == 255 && current_sensor_data.distance4 != 255 && current_sensor_data.distance5 == 255)
		{
			left_detected(turn_stack);
		}
	}
}

/* Upptäcker svängar på väg in i labyrinten */
void detectTurn(TurnStack* turn_stack)
{	
	if (current_sensor_data.distance3 > THRESHOLD_CONTACT_SIDE)
	{
		// Vänster ej kontakt
		if (min(current_sensor_data.distance1, current_sensor_data.distance2) < THRESHOLD_STOP)
		{
			// Fall 1, 3 eller 7
			if (current_sensor_data.distance4 > current_sensor_data.distance3 + THRESHOLD_CONTACT_SIDE)
			{
				// Fall 7
				pushTurnStack(turn_stack, newTurnNode(LEFT_TURN));
				makeTurn(RIGHT_TURN);
			}
			else if (current_sensor_data.distance3 > current_sensor_data.distance4 + THRESHOLD_CONTACT_SIDE)
			{
				// Fall 1 eller 3
				pushTurnStack(turn_stack, newTurnNode(RIGHT_TURN));
				makeTurn(LEFT_TURN);
			}
			else
			{
				// Bör kanske stanna för vi är på väg in i något
				return;
			}
		}	
		else if (((previous_sensor_data.distance1 + previous_sensor_data.distance2) / 2  >= THRESHOLD_STOP_DEAD_END) &&
				 ((current_sensor_data.distance1 + current_sensor_data.distance2) / 2  < THRESHOLD_STOP_DEAD_END))
		{
			// Fall 2
			pushTurnStack(turn_stack, newTurnNode(RIGHT_TURN));
			makeTurn(LEFT_TURN);
		}
		else if (current_sensor_data.distance3 < THRESHOLD_STOP_DEAD_END)
		{
			// Fall 4
			pushTurnStack(turn_stack, newTurnNode(STRAIGHT)); // Pusha att åka rakt fram
			makeTurn(STRAIGHT);
		}
		else
		{
			return;
		}
	} 
	else if (current_sensor_data.distance4 > THRESHOLD_CONTACT_SIDE)
	{
		// Höger ej kontakt men vänster har
		if (min(current_sensor_data.distance1, current_sensor_data.distance2)  < THRESHOLD_STOP)
		{
			// Fall 5
			pushTurnStack(turn_stack, newTurnNode(LEFT_TURN));
			makeTurn(RIGHT_TURN);
		}
		else if (((previous_sensor_data.distance1 + previous_sensor_data.distance2) / 2  >= THRESHOLD_STOP_DEAD_END) &&
				((current_sensor_data.distance1 + current_sensor_data.distance2) / 2  < THRESHOLD_STOP_DEAD_END))
		{
			// Fall 6
			pushTurnStack(turn_stack, newTurnNode(LEFT_TURN));
			makeTurn(RIGHT_TURN);
		}
		else if (current_sensor_data.distance4 < THRESHOLD_STOP_DEAD_END)
		{
			// Fall 8
			pushTurnStack(turn_stack, newTurnNode(STRAIGHT)); // Pusha att åka rakt fram
			makeTurn(STRAIGHT);
		}
		else
		{
			return; 
		}
	}
	else
	{
		// Kontakt på båda sidor
		return;
	}
}

/* Upptäcker svängar (även rakt fram) på väg ut ur labyrinten */
void detectTurnOut(volatile TurnStack* turn_stack)
{
	if (lockDetectTurn == 1)
		return;
	
	if (max(current_sensor_data.distance1, current_sensor_data.distance2) <= 70)
	{
		if (current_sensor_data.distance3 >= 100 || current_sensor_data.distance4 >= 100)
		{
			makeTurn(popTurnStack(turn_stack));
		}
	}
	else
	{
		if (current_sensor_data.distance3 == 255 || current_sensor_data.distance4 == 255)
		{
			driveStraight(15);
			makeTurn(popTurnStack(turn_stack));
		}
	}
	
	//if ((current_sensor_data.distance3 == 255 && current_sensor_data.distance5 == 255 &&
	       //previous_sensor_data.distance3 == 255 && previous_sensor_data.distance5 == 255) ||
	    //(current_sensor_data.distance4 == 255 && current_sensor_data.distance6 == 255 && 
		   //previous_sensor_data.distance4 == 255 && previous_sensor_data.distance6 == 255))
	//{
		//// Kör fram till mitten av svängen.
		//driveStraight(15);
		//makeTurn(popTurnStack(turn_stack));
	//}
	//else if (max(current_sensor_data.distance1, current_sensor_data.distance2) <= 40)
	//{
		//if ((current_sensor_data.distance3 >= 100 && current_sensor_data.distance5 == 255 &&
		      //previous_sensor_data.distance3 >= 100 && previous_sensor_data.distance5 == 255) ||
			//(current_sensor_data.distance4 >= 100 && current_sensor_data.distance6 == 255 &&
			  //previous_sensor_data.distance4 >= 100 && previous_sensor_data.distance6 == 255))
		//// Kör fram till mitten av svängen.
		//driveStraight(15);
		//makeTurn(popTurnStack(turn_stack));
	//}
}