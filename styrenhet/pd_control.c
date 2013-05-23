/*
 * FILNAMN:       styrenhet.c
 * PROJEKT:       Mazeter
 * PROGRAMMERARE: Mattias Fransson
 *				  Herman Ekwall
 * DATUM:         2013-05-16
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
#include <string.h>
#include <stdint.h>

#ifndef F_CPU
#define F_CPU 8000000UL
#endif

// TEST
#include <util/delay.h>

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
	// PD-reglering med avståndssensorerna

	uint8_t speed = throttle;
	
	ATOMIC_BLOCK(ATOMIC_FORCEON)
	{
		int8_t regulator_value = 0;
		
		// Vänster sida har släppt
		if (current->distance3 >= 50)
		{
			int16_t delta = 0;
			int16_t delta_previous = 0;
			
			if (current->distance6 != 255)
			{
				// Reglera på höger kort
				delta = 30 - current->distance6;
				delta_previous = 30 - previous->distance6;
			}
			else
			{
				 //Reglera på höger lång
				delta = 32 - current->distance4;
				delta_previous = 32 - previous->distance4;				
			}
			
			regulator_value = (float)control_parameters.dist_kp / 10 * delta + (float)control_parameters.dist_kd / 10 * (delta - delta_previous);
		}
		else if (current->distance4 >= 50)
		{
			int16_t delta = 0;
			int16_t delta_previous = 0;
			
			if (current->distance5 != 255)
			{
				// Reglera på vänster kort
				delta = current->distance5 - 30;
				delta_previous = previous->distance5 - 30;
			}
			else
			{
				 //Reglera på vänster lång
				delta = current->distance3 - 32;
				delta_previous = previous->distance3 - 32;
			}
			
			regulator_value = (float)control_parameters.dist_kp / 10 * delta + (float)control_parameters.dist_kd / 10 * (delta - delta_previous);
		}
		else if (current->distance4 >= 43)// && current->distance6 == 255)
		{
			// Kör höger mot mitten

			int16_t delta = 32 - current->distance4;
			int16_t delta_previous = 32 - previous->distance4;
			
			regulator_value = (float)control_parameters.dist_kp / 10 * delta + (float)control_parameters.dist_kd / 10 * (delta - delta_previous);
			
		}
		else if (current->distance3 >= 43) // && current->distance5 == 255)
		{
			// Kör vänster mot mitten
	
			int16_t delta = current->distance3 - 32;
			int16_t delta_previous = previous->distance3 - 32;
			
			regulator_value = (float)control_parameters.dist_kp / 10 * delta + (float)control_parameters.dist_kd / 10 * (delta - delta_previous);
		}
		else if (current->distance3 <= 42 && current->distance4 <= 42)
		{	
			int16_t delta_front = current->distance3 - current->distance4;
			int16_t delta_front_previous = previous->distance3 - previous->distance4;

			regulator_value = (float)control_parameters.dist_kp / 10 * delta_front + (float)control_parameters.dist_kd / 10 * (delta_front - delta_front_previous);
		}
		else
		{
			regulator_value = 0;
		}
		
		if (regulator_value > speed)
		{
			regulator_value = speed;
		}
		else if (regulator_value < -speed)
		{
			regulator_value = -speed;
		}
		
		if (regulator_value < 0)
		{
			// Sväng åt höger
			control_signals.right_value = speed + regulator_value;
			
			if ((control_signals.left_value = speed - regulator_value) > 100)
				control_signals.left_value = 100;
		}
		else
		{
			if ((control_signals.right_value = speed + regulator_value) > 100)
				control_signals.right_value = 100;
			
			control_signals.left_value = speed - regulator_value;
		}
	}
}

void stopBeforeTurn()
{
	uint16_t timer_count = 3000;
	
	commandToControlSignal(STEER_STOP);
	pwmWheels(control_signals);
	
	resetTimer();
	startTimer();
	
	while ((TIM16_ReadTCNT3() < timer_count) && !abort_flag)
	{}
	
	stopTimer();
}

void makeTurn(uint8_t turn)
{
	reset_gyro = 0;
	
	uint16_t timer_count = 3500; // 3500 för batteri 16, 3600 för batteri 18
	
	switch(turn)
	{
		case LEFT_TURN:
			stopBeforeTurn();
			commandToControlSignal(STEER_ROTATE_LEFT);
			pwmWheels(control_signals);
			//angle_end += DEGREES_90;
			resetTimer();
			startTimer();
		
			while ((TIM16_ReadTCNT3() < timer_count) && !abort_flag)
			{}
			
			stopTimer();
			break;
		
		case RIGHT_TURN:
			stopBeforeTurn();
			commandToControlSignal(STEER_ROTATE_RIGHT);
			pwmWheels(control_signals);
			//angle_end -= DEGREES_90;
			timer_count -= 200;
			resetTimer();	
			startTimer();

			while ((TIM16_ReadTCNT3() < timer_count) && !abort_flag)
			{}

			stopTimer();
			break;
		
		case STRAIGHT:
			driveStraight(15);			
			lockDetectTurn = 1;
			numberOfSensorTransfers = 0;
			return;
			break;
		
		case 0xEE:
			commandToControlSignal(CLAW_OPEN);
			pwmClaw(control_signals);
			while (!abort_flag)
			{
				commandToControlSignal(STEER_STOP);
				pwmWheels(control_signals);
			}
			break;
		
		default:
			return;
			break;
	}
	
	lockDetectTurn = 1;
	numberOfSensorTransfers = 0;
	
	driveStraight(50);
	turn_done_flag = 1;
	reset_gyro = 1;
}

void makeTurn180()
{
	uint16_t timer_count = 6800; // 7000 för batteri 18, 6800 för batteri 16
	commandToControlSignal(STEER_ROTATE_LEFT);
	pwmWheels(control_signals);
	resetTimer();
	startTimer();
	
	while ((TIM16_ReadTCNT3() < timer_count) && !abort_flag)
	{}
	
	stopTimer();
	numberOfSensorTransfers = 0;
	lockDetectTurn = 1;
}

void handleTape(TurnStack* turn_stack, uint8_t tape)
{
	switch(tape)
	{
		case LINE_GOAL:
			commandToControlSignal(CLAW_OPEN);
			pwmClaw(control_signals);
			algo_mode_flag = ALGO_GOAL;
			break;
		
		case LINE_GOAL_STOP:
			if (algo_mode_flag == ALGO_GOAL)
			{
				// stanna och plocka upp muggen
				commandToControlSignal(STEER_STOP);
				pwmWheels(control_signals);
				commandToControlSignal(CLAW_CLOSE);
				pwmClaw(control_signals);
				algo_mode_flag = ALGO_GOAL_REVERSE;
			}
			break;
			
		case LINE_TURN_LEFT:
			if (algo_mode_flag != ALGO_IN)
				break;
			driveStraight(DISTANCE_TAPE_TURN);
			makeTurn(LEFT_TURN);
			pushTurnStack(turn_stack, newTurnNode(RIGHT_TURN));
			break;
			
		case LINE_TURN_RIGHT:
			if (algo_mode_flag != ALGO_IN)
				break;
			driveStraight(DISTANCE_TAPE_TURN);
			makeTurn(RIGHT_TURN);
			pushTurnStack(turn_stack, newTurnNode(LEFT_TURN));
			break;
			
		case LINE_STRAIGHT:
			if (algo_mode_flag != ALGO_IN)
				break;

			driveStraight(20);
			makeTurn(STRAIGHT);

			current_sensor_data.line_type = LINE_NONE;
			pushTurnStack(turn_stack, newTurnNode(STRAIGHT));
			break;
			
		case LINE_START_STOP:
			if (algo_mode_flag == ALGO_START)
			{
				algo_mode_flag = ALGO_IN;
				driveStraight(20);
			}
			else if (algo_mode_flag == ALGO_OUT)
			{
				algo_mode_flag = ALGO_FINISH;
			}
			break;

		default:
			break;
	}
}

void lineRegulator(int8_t current_deviation, int8_t previous_deviation)
{
	const int8_t speed = 40;
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

// Kör framåt (driveStraight) eller bakåt (driveStraightBack) utan reglering under angiven sträcka.
// Värdet skiljer dock från verkligheten med ungefär 10 cm, men felet
// är alltid det samma.
void driveStraight(uint8_t cm)
{
	resetTimer();
	
	//uint32_t tmp = (uint32_t)cm*2*F_CPU/(1024 * ((uint32_t)control_signals.left_value + (uint32_t)control_signals.right_value)); // Prescaler 1024
	uint32_t tmp = (uint32_t)cm*F_CPU/(1024 * (uint32_t)throttle);
	uint16_t timer_count = (uint16_t)tmp; 
	
	/* Kör rakt fram med den högsta av hjulparshastigheterna */
	commandToControlSignal(STEER_STRAIGHT);
	pwmWheels(control_signals);
	
	startTimer();
	
	while((TIM16_ReadTCNT3() < timer_count) && !abort_flag)
	{}
}

void driveStraightBack(uint8_t cm)
{
	resetTimer();
	
	//uint32_t tmp = (uint32_t)cm*2*F_CPU/(1024 * ((uint32_t)control_signals.left_value + (uint32_t)control_signals.right_value)); // Prescaler 1024
	uint32_t tmp = (uint32_t)cm*F_CPU/(1024 * (uint32_t)throttle);
	uint16_t timer_count = (uint16_t)tmp;
	
	/* Kör rakt fram med den högsta av hjulparshastigheterna */
	commandToControlSignal(STEER_BACK);
	pwmWheels(control_signals);
	
	startTimer();
	
	while((TIM16_ReadTCNT3() < timer_count) && !abort_flag)
    {}
}

// Backar med reglering tills en sväng upptäcks och utför svängen.
void jamesBondTurn(volatile TurnStack* turn_stack)
{

	commandToControlSignal(STEER_BACK);
	pwmWheels(control_signals);
	
	if ((current_sensor_data.distance3 == 255 && current_sensor_data.distance5 == 255) ||
	    (current_sensor_data.distance4 == 255 && current_sensor_data.distance6 == 255))
	{
		uint8_t tmp = popTurnStack(turn_stack);
		
		if (tmp == LEFT_TURN)
		{
			tmp = RIGHT_TURN;
		}			
		else if (tmp == RIGHT_TURN)
		{
			tmp = LEFT_TURN;
		}
		else if (tmp == STRAIGHT)
		{
			makeTurn180();
			driveStraight(70);
			
			algo_mode_flag = ALGO_OUT;
			return;
		}
		
		// Kör fram till mitten av svängen.
		driveStraightBack(10);
		makeTurn(tmp);
		driveStraight(15);
		
		algo_mode_flag = ALGO_OUT;
	}
	
	straightRegulator((const SensorData*)&current_sensor_data, (const SensorData*)&previous_sensor_data);
}

uint16_t TIM16_ReadTCNT3()
{
	unsigned char sreg;
	uint16_t i;
	/* Save global interrupt flag */
	sreg = SREG;
	/* Disable interrupts */
	cli();
	/* Read TCNTn into i */
	i = TCNT3;
	/* Restore global interrupt flag */
	SREG = sreg;
	return i;
}
