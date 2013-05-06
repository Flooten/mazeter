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
	TCCR3B = (1 << CS10) | (0 << CS11) | (1 << CS12); // Prescaler 1024, ändra i pd_control.c i handleTape om prescalern ändras
}

void stopTimer()
{
	TCCR3B = 0x00;
}

void resetTimer()
{
	TCCR3B = 0x00;
	TCNT3 = 0x0000;
	TIFR3 |= (1 << TOV3);
}

void straightRegulator(const SensorData* current, const SensorData* previous)
{
	ATOMIC_BLOCK(ATOMIC_FORCEON)
	{
		static int8_t regulator_value = 0;
		
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
			//regulator_value = -20;
			
			int16_t delta = current->distance5 - current->distance4;
			int16_t delta_previous = previous->distance5 - previous->distance4;
			
			regulator_value = (float)control_parameters.dist_kp / 10 * delta + (float)control_parameters.dist_kd / 10 * (delta - delta_previous);
		}
		else if (current->distance3 >= 45 && current->distance5 == 255)
		{
			// Kör vänster mot mitten
			//regulator_value = 20;
			
			int16_t delta = current->distance3 - current->distance6;
			int16_t delta_previous = previous->distance3 - previous->distance6;
			
			regulator_value = (float)control_parameters.dist_kp / 10 * delta + (float)control_parameters.dist_kd / 10 * (delta - delta_previous);
		}
		else if (current->distance3 <= 42 && current->distance4 <= 42)
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
		//control_signals.left_direction = 1;
		//control_signals.right_direction = 1;
	}
}

void makeTurn(uint8_t turn)
{
	uint16_t angle_end = current_sensor_data.angle;
	uint16_t angle_start = angle_end;
	
	commandToControlSignal(CLAW_CLOSE);
	pwmClaw(control_signals);
		
	switch(turn)
	{
		case LEFT_TURN:
			angle_end += DEGREES_90;
			commandToControlSignal(STEER_ROTATE_LEFT);
			pwmWheels(control_signals);
			if (angle_end >= 36000)
			{
				angle_end -= 36000;
				while ((current_sensor_data.angle < angle_end || current_sensor_data.angle >= angle_start) && !abort_flag)
				{}	
			}
			else
			{
				while (current_sensor_data.angle < angle_end && !abort_flag)
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
				while ((current_sensor_data.angle > angle_end || current_sensor_data.angle <= angle_start) && !abort_flag)
				{}		
			}
			else
			{
				while (current_sensor_data.angle > angle_end && !abort_flag)
				{}
			}
			break;
		
		case STRAIGHT:
			commandToControlSignal(STEER_STRAIGHT);
			pwmWheels(control_signals);
			while ((current_sensor_data.distance3 > THRESHOLD_CONTACT_SIDE || current_sensor_data.distance4 > THRESHOLD_CONTACT_SIDE) && !abort_flag)
			{}
			break;
		
		default:
			break;
	}
	
	commandToControlSignal(STEER_STRAIGHT);
	pwmWheels(control_signals);
	
	
	// Ser till att vi inte lämnar svängen för PD-reglering förrän vi har något vettigt att PD-reglera på.
	while ((current_sensor_data.distance3 > THRESHOLD_CONTACT_SIDE || current_sensor_data.distance4 > THRESHOLD_CONTACT_SIDE) && !abort_flag)
	{
		commandToControlSignal(STEER_STRAIGHT);
		pwmWheels(control_signals);
		
		// Stannar roboten om vi är på väg att köra in i något.
		if (current_sensor_data.distance1 < THRESHOLD_ABORT || current_sensor_data.distance2 < THRESHOLD_ABORT)
		{
			commandToControlSignal(STEER_STOP);
			pwmWheels(control_signals);
			return;
		}
	}
	commandToControlSignal(CLAW_OPEN);
	pwmClaw(control_signals);
	
	driveStraight(10);
	
	commandToControlSignal(CLAW_CLOSE);
	pwmClaw(control_signals);
	
}

void handleTape(volatile TurnStack* turn_stack, uint8_t tape)
{
	switch(tape)
	{
		case LINE_GOAL:
			algo_mode_flag = ALGO_GOAL;
			break;
		
		case LINE_GOAL_STOP:
			// stanna och plocka upp muggen
			commandToControlSignal(STEER_STOP);
			pwmWheels(control_signals);
			commandToControlSignal(CLAW_CLOSE);
			pwmClaw(control_signals);
			algo_mode_flag = ALGO_GOAL_REVERSE;
			break;
			
		case LINE_TURN_LEFT:
			if (algo_mode_flag == ALGO_OUT)
				break;
			driveStraight(DISTANCE_TAPE_TURN);
			pushTurnStack(turn_stack, newTurnNode(RIGHT_TURN));
			makeTurn(LEFT_TURN);
			break;
			
		case LINE_TURN_RIGHT:
			if (algo_mode_flag == ALGO_OUT)
				break;
			driveStraight(DISTANCE_TAPE_TURN);
			pushTurnStack(turn_stack, newTurnNode(LEFT_TURN));
			makeTurn(RIGHT_TURN);
			break;
			
		case LINE_STRAIGHT:
			if (algo_mode_flag == ALGO_OUT)
				break;
			driveStraight(DISTANCE_TAPE_TURN);
			pushTurnStack(turn_stack, newTurnNode(STRAIGHT));
			makeTurn(STRAIGHT);
			break;
			
		case LINE_START_STOP:
			if (algo_mode_flag == ALGO_OUT)
			{
				commandToControlSignal(STEER_STOP);
				pwmWheels(control_signals);
				algo_mode_flag = ALGO_DONE;
			}
			break;
				
		default:
			break;
	}
}

void lineRegulator(int8_t current_deviation, int8_t previous_deviation)
{
	const int8_t speed = 60;
	int8_t regulator_value = (float)control_parameters.line_kp / 10 * current_deviation + (float)control_parameters.line_kd / 10 * (current_deviation - previous_deviation);
	
	if (regulator_value > speed)
	{
		regulator_value = speed;
	}
	else if (regulator_value < -speed)
	{
		regulator_value = -speed;
	}
	
	control_signals.right_value = speed + regulator_value;
	control_signals.left_value = speed - regulator_value;
	
	if (control_signals.right_value > 100)
		control_signals.right_value = 100;
	
	if (control_signals.left_value > 100)
		control_signals.left_value = 100;
	
	// Kör frammåt
	control_signals.left_direction = 1;
	control_signals.right_direction = 1;
}

void driveStraight(uint8_t cm)
{
	resetTimer();
	
	uint32_t tmp = (uint32_t)cm*2*F_CPU/(1024 * ((uint32_t)control_signals.left_value + (uint32_t)control_signals.right_value)); // Prescaler 1024
	
	uint16_t timer_count = (uint16_t)tmp; 
	
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
	
	while((TCNT3 < timer_count) && !abort_flag)
	{}
}

void jamesBondTurn(volatile TurnStack* turn_stack)
{
	// behöver pd regleras istället
	commandToControlSignal(STEER_BACK);
	pwmWheels(control_signals);
	
	while (current_sensor_data.distance7 > THRESHOLD_STOP && !abort_flag)
	{}	
		
	uint8_t tmp = popTurnStack(turn_stack);
	
	if (tmp == LEFT_TURN)
		makeTurn(RIGHT_TURN);
	else if (tmp == RIGHT_TURN)
		makeTurn(LEFT_TURN);
		
	algo_mode_flag = ALGO_OUT;
}
