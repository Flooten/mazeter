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
	if (x < y)
	{
		return y;
	}
	else
	{
		return x;
	}
}

/* Upptäcker svängar på väg in i labyrinten */
void detectTurn(volatile TurnStack* turn_stack)
{	
	//if (current_sensor_data.distance3 > THRESHOLD_CONTACT_SIDE)
	//{
		//// Vänster ej kontakt
		//if ((current_sensor_data.distance1 + current_sensor_data.distance2) / 2 < THRESHOLD_STOP)
		//{
			//// Fall 1, 3 eller 7
			//if (current_sensor_data.distance4 > current_sensor_data.distance3 + THRESHOLD_CONTACT_SIDE / 2)
			//{
				//// Fall 7
				//pushTurnStack(turn_stack, newTurnNode(LEFT_TURN));
				//makeTurn(RIGHT_TURN);
			//}
			//else if (current_sensor_data.distance3 > current_sensor_data.distance4 + THRESHOLD_CONTACT_SIDE / 2)
			//{
				//// Fall 1 eller 3
				//pushTurnStack(turn_stack, newTurnNode(RIGHT_TURN));
				//makeTurn(LEFT_TURN);
			//}
		//}	
		//else if (((previous_sensor_data.distance1 + previous_sensor_data.distance2) / 2 >= THRESHOLD_STOP_DEAD_END) &&
				 //((current_sensor_data.distance1 + current_sensor_data.distance2) / 2 < THRESHOLD_STOP_DEAD_END))
		//{
			//// Fall 2
			//pushTurnStack(turn_stack, newTurnNode(RIGHT_TURN));
			//makeTurn(LEFT_TURN);
		//}
		//else if (previous_sensor_data.distance3 > current_sensor_data.distance3 + 40)
		//{
			//// Fall 4
			//pushTurnStack(turn_stack, newTurnNode(STRAIGHT)); // Pusha att åka rakt fram
		//}
	//} 
	//else if (current_sensor_data.distance4 > THRESHOLD_CONTACT_SIDE)
	//{
		//// Höger ej kontakt men vänster har
		//if ((current_sensor_data.distance1 + current_sensor_data.distance2) / 2 < THRESHOLD_STOP)
		//{
			//// Fall 5
			//pushTurnStack(turn_stack, newTurnNode(LEFT_TURN));
			//makeTurn(RIGHT_TURN);
		//}
		//else if  (((previous_sensor_data.distance1 + previous_sensor_data.distance2) / 2 >= THRESHOLD_STOP_DEAD_END) &&
				  //((current_sensor_data.distance1 + current_sensor_data.distance2) / 2 < THRESHOLD_STOP_DEAD_END))
		//{
			//// Fall 6
			//pushTurnStack(turn_stack, newTurnNode(LEFT_TURN));
			//makeTurn(RIGHT_TURN);
		//}
		//else if (previous_sensor_data.distance4 > current_sensor_data.distance4 + 40)
		//{
			//// Fall 8
			//pushTurnStack(turn_stack, newTurnNode(STRAIGHT)); // Pusha att åka rakt fram
		//}
	//}
	if (current_sensor_data.distance3 > THRESHOLD_CONTACT_SIDE)
	{
		// Vänster ej kontakt
		if (current_sensor_data.distance2 < THRESHOLD_STOP)
		{
			// Fall 1, 3 eller 7
			if (current_sensor_data.distance4 > current_sensor_data.distance3 + THRESHOLD_CONTACT_SIDE / 2)
			{
				// Fall 7
				makeTurn(RIGHT_TURN);
			}
			else if (current_sensor_data.distance3 > current_sensor_data.distance4 + THRESHOLD_CONTACT_SIDE / 2)
			{
				// Fall 1 eller 3
				makeTurn(LEFT_TURN);
			}
		}
		else if (((previous_sensor_data.distance1 + previous_sensor_data.distance2) / 2 >= THRESHOLD_STOP_DEAD_END) &&
				((current_sensor_data.distance1 + current_sensor_data.distance2) / 2 < THRESHOLD_STOP_DEAD_END))
		{
			// Fall 2
			makeTurn(LEFT_TURN);
		}
		else
		{
			// Fall 4, rakt fram
			makeTurn(STRAIGHT);
		}
	}
	else if (current_sensor_data.distance4 > THRESHOLD_CONTACT_SIDE)
	{
		// Höger ej kontakt men vänster har
		if (current_sensor_data.distance2 < THRESHOLD_STOP)
		{
			// Fall 5
			makeTurn(RIGHT_TURN);
		}
		else if  (((previous_sensor_data.distance1 + previous_sensor_data.distance2) / 2 >= THRESHOLD_STOP_DEAD_END) &&
		((current_sensor_data.distance1 + current_sensor_data.distance2) / 2 < THRESHOLD_STOP_DEAD_END))
		{
			// Fall 6
			makeTurn(RIGHT_TURN);
		}
		else
		{
			// Fall 8
			makeTurn(STRAIGHT);
		}
	}

}

/* Upptäcker svängar (även rakt fram) på väg ut ur labyrinten */
void detectTurnOut(volatile TurnStack* turn_stack)
{
	//if (current_sensor_data.distance3 > THRESHOLD_CONTACT_SIDE || current_sensor_data.distance4 > THRESHOLD_CONTACT_SIDE)
	//{
		//// Kör fram till mitten av svängen.
		//driveStraight(DISTANCE_DETECT_TURN);
		//makeTurn(popTurnStack(turn_stack));
	//}
	
	if (current_sensor_data.distance3 > THRESHOLD_CONTACT_SIDE)
	{
		// Vänster ej kontakt
		if ((current_sensor_data.distance1 + current_sensor_data.distance2) / 2 < THRESHOLD_STOP)
		{
			// Fall 1, 3 eller 7
			if (current_sensor_data.distance4 > current_sensor_data.distance3 + THRESHOLD_CONTACT_SIDE / 2)
			{
				// Fall 7
				makeTurn(RIGHT_TURN);
			}
			else if (current_sensor_data.distance3 > current_sensor_data.distance4 + THRESHOLD_CONTACT_SIDE / 2)
			{
				// Fall 1 eller 3
				makeTurn(LEFT_TURN);
			}
		}
		else if (((previous_sensor_data.distance1 + previous_sensor_data.distance2) / 2 >= THRESHOLD_STOP_DEAD_END) &&
		((current_sensor_data.distance1 + current_sensor_data.distance2) / 2 < THRESHOLD_STOP_DEAD_END))
		{
			// Fall 2
			makeTurn(LEFT_TURN);
		}
		else if (current_sensor_data.distance3 > THRESHOLD_NO_CONTACT && 
					((previous_sensor_data.distance1 + previous_sensor_data.distance2) / 2 > THRESHOLD_NO_CONTACT ||
					current_sensor_data.distance4 > THRESHOLD_NO_CONTACT))
		{
			// Tejpfall
			driveStraight(DISTANCE_DETECT_TURN);
			makeTurn(popTurnStack(turn_stack));
		}
		else
		{
			// Fall 4, Kör rakt fram
			makeTurn(STRAIGHT);			
		}
	}
	else if (current_sensor_data.distance4 > THRESHOLD_CONTACT_SIDE)
	{
		// Höger ej kontakt men vänster har
		if ((current_sensor_data.distance1 + current_sensor_data.distance2) / 2 < THRESHOLD_STOP)
		{
			// Fall 5
			makeTurn(RIGHT_TURN);
		}
		else if  (((previous_sensor_data.distance1 + previous_sensor_data.distance2) / 2 >= THRESHOLD_STOP_DEAD_END) &&
		((current_sensor_data.distance1 + current_sensor_data.distance2) / 2 < THRESHOLD_STOP_DEAD_END))
		{
			// Fall 6
			makeTurn(RIGHT_TURN);
		}
		else if (current_sensor_data.distance4 > THRESHOLD_NO_CONTACT &&
					((previous_sensor_data.distance1 + previous_sensor_data.distance2) / 2 > THRESHOLD_NO_CONTACT ||
					current_sensor_data.distance3 > THRESHOLD_NO_CONTACT))
		{
			// Tejpfall
			driveStraight(DISTANCE_DETECT_TURN);
			makeTurn(popTurnStack(turn_stack));
		}
		else
		{
			// Fall 8, kör rakt fram
			makeTurn(STRAIGHT);
		}
	}	
}

/* Upptäcker svängar (även rakt fram) på väg ut ur labyrinten, kör baklänges */
void detectTurnBack(volatile TurnStack* turn_stack)
{
	commandToControlSignal(STEER_BACK);
	pwmWheels(control_signals);
	
	if (current_sensor_data.distance3 > THRESHOLD_CONTACT_SIDE)
	{
		// Vänster ej kontakt
		if (current_sensor_data.distance7 < THRESHOLD_STOP)
		{
			// Fall 1, 3 eller 7
			if (current_sensor_data.distance4 > current_sensor_data.distance3 + THRESHOLD_CONTACT_SIDE / 2)
			{
				// Fall 7
				makeTurn(LEFT_TURN);
			}
			else if (current_sensor_data.distance3 > current_sensor_data.distance4 + THRESHOLD_CONTACT_SIDE / 2)
			{
				// Fall 1 eller 3
				makeTurn(RIGHT_TURN);
			}
		}
		else if ((previous_sensor_data.distance7 >= THRESHOLD_STOP_DEAD_END) &&
		(current_sensor_data.distance7 < THRESHOLD_STOP_DEAD_END))
		{
			// Fall 2
			makeTurn(RIGHT_TURN);
		}
		else if (current_sensor_data.distance3 > THRESHOLD_NO_CONTACT &&
				(current_sensor_data.distance7 > THRESHOLD_NO_CONTACT ||
				 current_sensor_data.distance4 > THRESHOLD_NO_CONTACT))
		{
			// Tejpfall
			driveStraight(DISTANCE_DETECT_TURN);
			uint8_t tmp = popTurnStack(turn_stack);
			
			if (tmp == LEFT_TURN)
				makeTurn(RIGHT_TURN);
			else if (tmp == RIGHT_TURN)
				makeTurn(LEFT_TURN);
			else if (tmp == STRAIGHT)
			{
				while ((current_sensor_data.distance3 > THRESHOLD_CONTACT_SIDE - 5 || current_sensor_data.distance4 > THRESHOLD_CONTACT_SIDE - 5) && !abort_flag)
				{
					straightRegulator((const SensorData*)&current_sensor_data, (const SensorData*)&previous_sensor_data);
				}
				return;
			}
			algo_mode_flag = ALGO_OUT;
		}
		else
		{
			// Fall 4, Kör rakt fram
			makeTurn(STRAIGHT);
		}
	}
	else if (current_sensor_data.distance4 > THRESHOLD_CONTACT_SIDE)
	{
		// Höger ej kontakt men vänster har
		if (current_sensor_data.distance7 < THRESHOLD_STOP)
		{
			// Fall 5
			makeTurn(LEFT_TURN);
		}
		else if  ((previous_sensor_data.distance7 >= THRESHOLD_STOP_DEAD_END) &&
		(current_sensor_data.distance7 < THRESHOLD_STOP_DEAD_END))
		{
			// Fall 6
			makeTurn(LEFT_TURN);
		}
		else if (current_sensor_data.distance4 > THRESHOLD_NO_CONTACT &&
		(current_sensor_data.distance7 > THRESHOLD_NO_CONTACT ||
		current_sensor_data.distance3 > THRESHOLD_NO_CONTACT))
		{
			// Tejpfall
			driveStraight(DISTANCE_DETECT_TURN);
			uint8_t tmp = popTurnStack(turn_stack);
			
			if (tmp == LEFT_TURN)
				makeTurn(RIGHT_TURN);
			else if (tmp == RIGHT_TURN)
				makeTurn(LEFT_TURN);
			else if (tmp == STRAIGHT)
			{
				while ((current_sensor_data.distance3 > THRESHOLD_CONTACT_SIDE - 5 || current_sensor_data.distance4 > THRESHOLD_CONTACT_SIDE - 5) && !abort_flag)
				{
					straightRegulator((const SensorData*)&current_sensor_data, (const SensorData*)&previous_sensor_data);
				}
				return;
			}
			algo_mode_flag = ALGO_OUT;
		}
		else
		{
			// Fall 8, kör rakt fram
			makeTurn(STRAIGHT);
		}
	}
}