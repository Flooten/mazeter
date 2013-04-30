/*
 * FILNAMN:       styrenhet.c
 * PROJEKT:       Mazeter
 * PROGRAMMERARE: Mattias Fransson
 *				  Herman Ekwall
 * DATUM:         2013-04-18
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

typedef struct  
{
	int8_t left_value;
	int8_t right_value;
} RegulatorSignals;

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

RegulatorSignals regulatorSignalDeltaLeft(const int16_t* delta_left, const int16_t* delta_left_previous)
{
	RegulatorSignals ret;
	ret.left_value = control_parameters.left_kp * *delta_left + control_parameters.left_kd * (*delta_left - *delta_left_previous);
	ret.right_value = -(control_parameters.right_kp * *delta_left + control_parameters.right_kd * (*delta_left - *delta_left_previous));
	return ret;
}

RegulatorSignals regulatorSignalDeltaRight(const int16_t* delta_right, const int16_t* delta_right_previous)
{
	RegulatorSignals ret;
	ret.left_value = -(control_parameters.left_kp * *delta_right + control_parameters.left_kd * (*delta_right - *delta_right_previous));
	ret.right_value = control_parameters.right_kp * *delta_right + control_parameters.right_kd * (*delta_right - *delta_right_previous);
	return ret;
}

RegulatorSignals regulatorSignalDeltaFront(const int16_t* delta_front, const int16_t* delta_front_previous)
{
	RegulatorSignals ret;
	ret.left_value = -(control_parameters.left_kp * *delta_front + control_parameters.left_kd * (*delta_front - *delta_front_previous));
	ret.right_value = control_parameters.right_kp * *delta_front + control_parameters.right_kd * (*delta_front - *delta_front_previous);
	return ret;
}

RegulatorSignals regulatorSignalDeltaBack(const int16_t* delta_back, const int16_t* delta_back_previous)
{
	RegulatorSignals ret;
	ret.left_value = control_parameters.left_kp * *delta_back + control_parameters.left_kd * (*delta_back - *delta_back_previous);
	ret.right_value = -(control_parameters.right_kp * *delta_back + control_parameters.right_kd * (*delta_back - *delta_back_previous));
	return ret;
}

void sensorDataToControlSignal(const SensorData* current, const SensorData* previous)
{
	ATOMIC_BLOCK(ATOMIC_FORCEON)
	{
		RegulatorSignals regulator_signals;
		
		// switch
		
		//int16_t delta_front = current->distance3 - current->distance4;
		//int16_t delta_front_previous = previous->distance3 - previous->distance4;
		
		int16_t delta_left = current->distance3 - current->distance5;
		int16_t delta_left_previous = previous->distance3 - previous->distance5;
		
		regulator_signals = regulatorSignalDeltaLeft(&delta_left, &delta_left_previous);
		
		if (regulator_signals.left_value + (int8_t)control_signals.left_value < 0)
		{
			control_signals.left_value = 0;
		}
		else
		{
			control_signals.left_value += regulator_signals.left_value;
			if (control_signals.left_value > 60)
			{
				control_signals.left_value = 60;
			}
		}
		
		if (regulator_signals.right_value +  (int8_t)control_signals.right_value < 0)
		{
			control_signals.right_value = 0;
		}
		else
		{
			control_signals.right_value += regulator_signals.right_value;
			if (control_signals.right_value > 60)
			{
				control_signals.right_value = 60;
			}
		}
	}
}

void makeTurn(uint8_t turn)
{
	uint16_t angle_end = current_sensor_data.angle;
	uint16_t angle_start = angle_end;
	
	switch(turn)
	{
		case LEFT_TURN:
			angle_end += 9000;
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
			angle_end -= 9000;
			commandToControlSignal(STEER_ROTATE_RIGHT);
			pwmWheels(control_signals);
			if (angle_end >= 36000)
			{
				angle_end = 36000 - (9000 - angle_start);
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
			break;
		
		default:
			break;
	}
	
	commandToControlSignal(STEER_STOP); // för test ska vara: commandToControlSignal(STEER_STRAIGHT);
	pwmWheels(control_signals);
	
	
	// Ser till att vi inte lämnar svängen för PD-reglering förrän vi har något vettigt att PD-reglera på.
	while (current_sensor_data.distance3 > THRESHOLD_CONTACT && current_sensor_data.distance4 > THRESHOLD_CONTACT)
	{
		// Stannar roboten om vi är på väg att köra in i något.
		if (current_sensor_data.distance1 < THRESHOLD_STOP - 5 || current_sensor_data.distance2 < THRESHOLD_STOP -5)
		{
			commandToControlSignal(STEER_STOP);
			pwmWheels(control_signals);
			return;
		}
	}
}

void handleTape(volatile TurnStack* turn_stack, uint8_t turn)
{
	uint16_t timer_count = 800000000/(1024*(control_signals.left_value + control_signals.right_value)); // Prescaler 1024
		
	switch(turn)
	{
		case LINE_GOAL:
			// algo mål
			break;
			
		case LINE_TURN_LEFT:
			startTimer();
		
			while(TCNT1 < timer_count)
			{}
			pushTurnStack(turn_stack, newTurnNode(RIGHT_TURN));
			makeTurn(LEFT_TURN);
			break;
			
		case LINE_TURN_RIGHT:
			startTimer();
		
			while(TCNT1 < timer_count)
			{}	
			pushTurnStack(turn_stack, newTurnNode(LEFT_TURN));
			makeTurn(RIGHT_TURN);
			break;
			
		case LINE_STRAIGHT:
			//startTimer();
		//
			//while(TCNT1 < 2*timer_count)
			//{}
			pushTurnStack(turn_stack, newTurnNode(STRAIGHT));
			makeTurn(STRAIGHT);
			break;
			
		default:
			break;
	}
	resetTimer();
}