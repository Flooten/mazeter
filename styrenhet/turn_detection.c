/*
 * FILNAMN:       turn_detection.c
 * PROJEKT:       Mazeter
 * PROGRAMMERARE: Martin Andersson
 *				  Joel Davidsson
 *				  Fredrik Stenmark
 * DATUM:         2013-04-25
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
	if (max(current_sensor_data.distance1, current_sensor_data.distance2) <= 90)
	{
		driveStraight(15);
	}
	else
	{
		driveStraight(10); // för att undvika moment fr reglering precis innan sväng
	}
	pushTurnStack(turn_stack, newTurnNode(LEFT_TURN));
	makeTurn(RIGHT_TURN);	
}

// Vänstersväng upptäckt
void left_detected(TurnStack* turn_stack)
{
	if (max(current_sensor_data.distance1, current_sensor_data.distance2) <= 90)
	{
		driveStraight(15);
	}
	else
	{
		driveStraight(10); // för att undvika moment fr reglering precis innan sväng
	}
	pushTurnStack(turn_stack, newTurnNode(RIGHT_TURN));
	makeTurn(LEFT_TURN);
}

/* Upptäcker svängar på väg in i labyrinten */
void detectTurn(TurnStack* turn_stack)
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

/* Upptäcker svängar (även rakt fram) på väg ut ur labyrinten */
void detectTurnOut(volatile TurnStack* turn_stack)
{
	if (lockDetectTurn == 1)
		return;
	
	if (max(current_sensor_data.distance1, current_sensor_data.distance2) <= 35)
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
}
