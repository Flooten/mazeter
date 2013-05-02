/*
 * FILNAMN:       styrenhet.c
 * PROJEKT:       Mazeter
 * PROGRAMMERARE: Mattias Fransson
 *				  Herman Ekwall
 * DATUM:         2013-05-02
 *
 */

#include "pd_control.h"
#include "control_parameters.h"
#include "styrenhet.h"
#include "spi_commands.h"
#include "turn_detection.h"
#include "PWM.h"
#include "controlsignals.h"
#include <util/atomic.h>
#include <stdint.h>

#ifndef F_CPU
#define F_CPU 8000000UL
#endif


void startTimer()
{
	TCCR1B = (1 << CS10) | (0 << CS11) | (1 << CS12); // Prescaler 1024, ändra i pd_control.c i handleTape om prescalern ändras
}

void resetTimer()
{
	TCCR1B = 0x00;
	TCNT1 = 0x0000;
	TIFR1 |= (1 << TOV1);
}

void sensorDataToControlSignal(const SensorData* current, const SensorData* previous)
{
	ATOMIC_BLOCK(ATOMIC_FORCEON)
	{
		int8_t regulator_value = 0;
		
		if (current->distance3 >= 80 && current->distance6 != 255)
		{
			// Reglera på högersidan
			int16_t delta_right = current->distance4 - current->distance6;
			int16_t delta_right_previous = previous->distance4 - previous->distance6;
			
			regulator_value = -((float)control_parameters.dist_kp / 10 * delta_right + (float)control_parameters.dist_kd / 10 * (delta_right - delta_right_previous));
		}
		else if (current->distance4 >= 80 && current->distance5 != 255)
		{
			// Reglera på vänstersidan
			int16_t delta_left = current->distance3 - current->distance5;
			int16_t delta_left_previous = previous->distance3 - previous->distance5;
			
			regulator_value = (float)control_parameters.dist_kp / 10 * delta_left + (float)control_parameters.dist_kd / 10 * (delta_left - delta_left_previous);
		}
		else if (current->distance4 >= 45 && current->distance6 == 255)
		{
			// Kör höger mot mitten
			regulator_value = -20;
		}
		else if (current->distance3 >= 45 && current->distance5 == 255)
		{
			// Kör vänster mot mitten
			regulator_value = 20;
		}
		else 
		{		
			int16_t delta_front = current->distance3 - current->distance4;
			int16_t delta_front_previous = previous->distance3 - previous->distance4;
			
			regulator_value = (float)control_parameters.dist_kp / 10 * delta_front + (float)control_parameters.dist_kd / 10 * (delta_front - delta_front_previous);
		}		
		
		if (regulator_value > 100)
		{
			regulator_value = 100;
		}
		else if (regulator_value < -100)
		{
			regulator_value = -100;
		}
		
		if (regulator_value < 0)
		{
			// Sväng åt höger
			control_signals.right_value = 100 + regulator_value;
			control_signals.left_value = 100;
		}
		else
		{
			control_signals.right_value = 100;
			control_signals.left_value = 100 - regulator_value;
		}
		
	
		// Kör frammåt
		control_signals.left_direction = 1;
		control_signals.right_direction = 1;
	}
}

void makeTurn(uint8_t turn)
{
	uint16_t angle_end = current_sensor_data.angle;
	uint16_t angle_start = angle_end;
		
	switch(turn)
	{
		case LEFT_TURN:
			angle_end += DEGREES_90;
			commandToControlSignal(STEER_ROTATE_LEFT);
			pwmWheels(control_signals);
			if (angle_end >= 36000)
			{
				angle_end -= 36000;
				while (current_sensor_data.angle < angle_end || current_sensor_data.angle >= angle_start)
				{}	
			}
			else
			{
				while (current_sensor_data.angle < angle_end)
				{}
			}
			break;
		
		case RIGHT_TURN:
			angle_end -= DEGREES_90;
			commandToControlSignal(STEER_ROTATE_RIGHT);
			pwmWheels(control_signals);
			if (angle_end >= 36000)
			{
				angle_end = 36000 - (DEGREES_90 - angle_start);
				while (current_sensor_data.angle > angle_end || current_sensor_data.angle <= angle_start)
				{}		
			}
			else
			{
				while (current_sensor_data.angle > angle_end)
				{}
			}
			break;
		
		case STRAIGHT:
			commandToControlSignal(STEER_STRAIGHT);
			pwmWheels(control_signals);
			while (current_sensor_data.distance3 > THRESHOLD_CONTACT || current_sensor_data.distance4 > THRESHOLD_CONTACT)
			{}
			break;
		
		default:
			break;
	}
	
	commandToControlSignal(STEER_STRAIGHT);
	pwmWheels(control_signals);
	
	
	// Ser till att vi inte lämnar svängen för PD-reglering förrän vi har något vettigt att PD-reglera på.
	while (current_sensor_data.distance3 > THRESHOLD_CONTACT || current_sensor_data.distance4 > THRESHOLD_CONTACT)
	{
		// Stannar roboten om vi är på väg att köra in i något.
		if (current_sensor_data.distance1 < THRESHOLD_ABORT || current_sensor_data.distance2 < THRESHOLD_ABORT)
		{
			commandToControlSignal(STEER_STOP);
			pwmWheels(control_signals);
			return;
		}
	}
	
}

void handleTape(volatile TurnStack* turn_stack, uint8_t turn)
{
	
	switch(turn)
	{
		case LINE_GOAL:
			// algo mål
			break;
		
		case LINE_GOAL_STOP:
			// stanna och plocka upp muggen
			commandToControlSignal(STEER_STOP);
			pwmWheels(control_signals);
			commandToControlSignal(CLAW_CLOSE);
			pwmClaw(control_signals);
			break;
			
		case LINE_TURN_LEFT:
			driveStraight(DISTANCE_TAPE_TURN);
			pushTurnStack(turn_stack, newTurnNode(RIGHT_TURN));
			makeTurn(LEFT_TURN);
			break;
			
		case LINE_TURN_RIGHT:
			driveStraight(DISTANCE_TAPE_TURN);
			pushTurnStack(turn_stack, newTurnNode(LEFT_TURN));
			makeTurn(RIGHT_TURN);
			break;
			
		case LINE_STRAIGHT:
			driveStraight(DISTANCE_TAPE_TURN);
			pushTurnStack(turn_stack, newTurnNode(STRAIGHT));
			makeTurn(STRAIGHT);
			break;
			
		case LINE_START:
			// linjen vi passerar när vi går in och ut ur labyrinten
				
		default:
			break;
	}
	resetTimer();
}

void lineRegulator(int8_t current_deviation, int8_t previous_deviation)
{
	const int8_t speed = 80;
	int8_t regulator_value = (float)control_parameters.line_kp / 10 * current_deviation + (float)control_parameters.line_kd / 10 * (current_deviation - previous_deviation);
	
	if (regulator_value > speed)
	{
		regulator_value = speed;
	}
	else if (regulator_value < -speed)
	{
		regulator_value = -speed;
	}
	
	control_signals.right_value = speed - regulator_value;
	control_signals.left_value = speed + regulator_value;
	
	if (control_signals.right_value > 100)
		control_signals.right_value = 100;
	
	if (control_signals.left_value > 100)
		control_signals.left_value = 100;
	
	// Kör frammåt
	control_signals.left_direction = 1;
	control_signals.right_direction = 1;
}
}

void driveStraight(uint8_t cm)
{
	uint16_t timer_count = cm*2*F_CPU/(1024*(control_signals.left_value + control_signals.right_value)); // Prescaler 1024
	
	/* Kör rakt fram med den högsta av hjulparshastigheterna */
	if (control_signals.right_value > control_signals.left_value)
	{
		control_signals.left_value = control_signals.right_value;
	} 
	else
	{
		control_signals.right_value = control_signals.left_value;
	}
	pwmWheels(control_signals);
	
	startTimer();
	
	while(TCNT1 < timer_count)
	{}
}