/*
 * FILNAMN:       styrenhet.c
 * PROJEKT:       Mazeter
 * PROGRAMMERARE: Fredrik Stenmark
 *                Herman Ekwall
 *                Mattias Fransson
 * DATUM:         2013-05-16
 *
 * BESKRIVNING:   Styrenhetens huvudloop.
 *
 */

#include <stdio.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "styrenhet.h"
#include "spi.h"
#include "PWM.h"
#include "controlsignals.h"
#include "sensor_data.h"
#include "control_parameters.h"
#include "pd_control.h"

// SPI-variabler
volatile uint8_t* buffer = NULL;
volatile uint8_t buffer_size = 0;
volatile uint8_t current_byte = 0;
volatile uint8_t spi_status = SPI_READY;

volatile uint8_t control_mode_flag = FLAG_MANUAL;
volatile uint8_t current_command;
volatile uint8_t throttle;
volatile uint8_t new_sensor_data_flag = 0;
volatile uint8_t receiving_sensor_data_flag = 0;
volatile uint8_t abort_flag = 0;
volatile uint8_t algo_mode_flag = ALGO_IN;
volatile uint8_t turn_stack_top;
volatile uint8_t turn_stack_sent_flag = 0;

volatile ControlSignals control_signals;
volatile SensorData current_sensor_data;
volatile SensorData previous_sensor_data;
volatile ControlParameters control_parameters;

volatile uint8_t turn_done_flag = 0;
volatile uint8_t turn_done_flag_copy;
volatile uint8_t reset_gyro = 1;

volatile TurnStack turn_stack;

void parseCommand(uint8_t cmd);

ISR(SPI_STC_vect)
{
    // Läs in den mottagna datan.
    uint8_t received = SPDR;
    
    if (spi_status == SPI_RECEIVING_DATA)
    {
        // Om vi försöker skriva data till en felaktigt satt buffer, skicka fel.
        // Ska aldrig kunna ske.
        if (buffer == NULL || current_byte >= buffer_size)
        {
            SPDR = ERROR_SPI;
            return;
        }
        
        buffer[current_byte++] = received;
        
        // När vi är klara, återställ SPI-funktionaliteten.
        if (current_byte == buffer_size)
        {
            spi_status = SPI_READY;
            buffer = NULL;
            buffer_size = 0;
            current_byte = 0;
			
            // Specialflaggan för ljust sensordatan.
			if(receiving_sensor_data_flag == 1)
			{
				new_sensor_data_flag = 1;
				receiving_sensor_data_flag = 0;
			}
			
        }
    }
    else
    {
        // Så länge 0 skickas från kommunikationsenheten antar vi att vi ska skicka data.
        if (received == 0)
        {
            if (buffer == NULL || current_byte == buffer_size)
            {
                SPDR = ERROR_SPI;
                return;
            }
            
            SPDR = buffer[current_byte++];
			
            // Återställ SPI när vi är klara
			if (current_byte == buffer_size)
			{
				spi_status = SPI_READY;
				buffer = NULL;
				buffer_size = 0;
				current_byte = 0;
			}			
        }
        else
        {
            // Om den mottagna byten inte var 0, och vi håller inte på att ta emot data så måste
            // det finnas en kommandokod i received.
            parseCommand(received);
        }
    }
}

// Funktion som hanterar SPI-kommandon.
void parseCommand(uint8_t cmd)
{
    // Försätt SPI-variablerna i rätt läge beroende på vilket kommando
    // som inkommer (skicka/ta emot). Om styrenheten behöver vidare information när
    // ett kommando tas emot, försök hantera detta med flaggor.
    switch (cmd)
    {
        case SENSOR_DATA_ALL:
            SPDR = SENSOR_DATA_ALL;
            buffer = &current_sensor_data.distance1;
            buffer_size = sizeof(current_sensor_data);
            current_byte = 0;
            spi_status = SPI_RECEIVING_DATA;
			new_sensor_data_flag = 0;
			receiving_sensor_data_flag = 1;
			previous_sensor_data = current_sensor_data;
            break;

		case CONTROL_SIGNALS:
			SPDR = CONTROL_SIGNALS;
			buffer = &control_signals.right_value;
			buffer_size = sizeof(control_signals);
			current_byte = 0;
			break;

		case CONTROL_THROTTLE:
			SPDR = CONTROL_THROTTLE;
			buffer = &throttle;
			buffer_size = 1;
			current_byte = 0;
			spi_status = SPI_RECEIVING_DATA;
			break;
			
		case FLAG_AUTO:
			if (control_mode_flag == FLAG_MANUAL)
			{
                // Halvreset, återställ labyrintlogiken.
				clear(&turn_stack);
				algo_mode_flag = ALGO_IN; // ALGO_START;
				control_signals.left_direction = 1;
				control_signals.right_direction = 1;
				
				//TEST FÖR ATT PRIMA STACKEN
				//pushTurnStack((TurnStack*)&turn_stack, newTurnNode(LEFT_TURN));
				//pushTurnStack((TurnStack*)&turn_stack, newTurnNode(LEFT_TURN));
				//pushTurnStack((TurnStack*)&turn_stack, newTurnNode(STRAIGHT));
			}
			control_mode_flag = FLAG_AUTO;
			break;
			
		case FLAG_MANUAL:
			control_mode_flag = FLAG_MANUAL;
			abort_flag = 0;
			break;

		case STEER_STRAIGHT:
			SPDR = STEER_STRAIGHT;
			current_command = STEER_STRAIGHT;
			buffer_size = 0;
			current_byte = 0;
			break;

		case STEER_STRAIGHT_LEFT:
			SPDR = STEER_STRAIGHT_LEFT;
			current_command = STEER_STRAIGHT_LEFT;
			buffer_size = 0;
			current_byte = 0;
			break;

		case STEER_STRAIGHT_RIGHT:
			SPDR = STEER_STRAIGHT_RIGHT;
			current_command = STEER_STRAIGHT_RIGHT;
			buffer_size = 0;
			current_byte = 0;
			break;

		case STEER_BACK:
			SPDR = STEER_BACK;
			current_command = STEER_BACK;
			buffer_size = 0;
			current_byte = 0;
			break;

		case STEER_STOP:
			SPDR = STEER_STOP;
			current_command = STEER_STOP;
			buffer_size = 0;
			current_byte = 0;
			break;

		case STEER_ROTATE_LEFT:
			SPDR = STEER_ROTATE_LEFT;
			current_command = STEER_ROTATE_LEFT;
			buffer_size = 0;
			current_byte = 0;
			break;

		case STEER_ROTATE_RIGHT: 
			SPDR = STEER_ROTATE_RIGHT;
			current_command = STEER_ROTATE_RIGHT;
			buffer_size = 0;
			current_byte = 0;
			break;

		case CLAW_OPEN:
			SPDR = CLAW_OPEN;
			current_command = CLAW_OPEN;
			buffer_size = 0;
			current_byte = 0;
			break;

		case CLAW_CLOSE:
			SPDR = CLAW_CLOSE;
			current_command = CLAW_CLOSE;
			buffer_size = 0;
			current_byte = 0;
			break;
			
		case CONTROL_PARAMETERS_ALL:
			SPDR = CONTROL_PARAMETERS_ALL;
			buffer = (uint8_t*)&control_parameters.dist_kp;
			buffer_size = sizeof(control_parameters);
			current_byte = 0;
			spi_status = SPI_RECEIVING_DATA;
			break;
			
		case ABORT:
			SPDR = ABORT;
			abort_flag = 1;
			new_sensor_data_flag = 0;
			break;
			
		case CHECK_STACK:
			SPDR = CHECK_STACK;
			turn_stack_sent_flag = turn_stack.new_node;
			turn_stack.new_node = 0;
			buffer = (uint8_t*)&turn_stack_sent_flag;
			buffer_size = 1;
			current_byte = 0;
			break;
			
		case TURN_STACK_TOP:
			SPDR = TURN_STACK_TOP;
			turn_stack_top = topTurnStack((const TurnStack*)&turn_stack);
			buffer = (uint8_t*)&turn_stack_top;
			buffer_size = 1;
			current_byte = 0;
			break;
			
		case ALGO_STATE:
			SPDR = ALGO_STATE;
			buffer = (uint8_t*)&algo_mode_flag;
			buffer_size = 1;
			current_byte = 0;
			break;
			
		case TURN_DONE:
			SPDR = TURN_DONE;
			turn_done_flag_copy = turn_done_flag;
			turn_done_flag = 0;
			buffer = (uint8_t*)&turn_done_flag_copy;
			buffer_size = 1;
			current_byte = 0;
			break;
			
		case RESET_GYRO:
			SPDR = RESET_GYRO;
			buffer = (uint8_t*)&reset_gyro;
			buffer_size = 1;
			current_byte = 0;
			break;
			
        default:
            SPDR = ERROR_UNKNOWN_SPI_COMMAND;
			buffer_size = 0;
			current_byte = 0;
            buffer = NULL;
            buffer_size = 0;
            current_byte = 0;
            break;
    }
}

// Översätter ett styrkommando till styrsignaler (control_signals).
void commandToControlSignal(uint8_t cmd)
{
	switch (cmd)
	{
		case STEER_STRAIGHT:
			control_signals.right_value = throttle;
			control_signals.left_value = throttle;
			control_signals.right_direction = 1;
			control_signals.left_direction = 1;
			break;
		
		case STEER_STRAIGHT_LEFT:
			control_signals.right_value = throttle;
			control_signals.left_value = throttle / 3;
			control_signals.right_direction = 1;
			control_signals.left_direction = 1;
			break;
		
		case STEER_STRAIGHT_RIGHT:
			control_signals.right_value = throttle / 3;
			control_signals.left_value = throttle;
			control_signals.right_direction = 1;
			control_signals.left_direction = 1;
			break;
		
		case STEER_BACK:
			control_signals.right_value = throttle;
			control_signals.left_value = throttle;
			control_signals.right_direction = 0;
			control_signals.left_direction = 0;
			break;
		
		case STEER_STOP:
			control_signals.right_value = 0;
			control_signals.left_value = 0;
			break;
		
		case STEER_ROTATE_LEFT:
			control_signals.right_value = throttle;
			control_signals.left_value = throttle;
			control_signals.right_direction = 1;
			control_signals.left_direction = 0;
			break;
		
		case STEER_ROTATE_RIGHT:
			control_signals.right_value = throttle;
			control_signals.left_value = throttle;
			control_signals.right_direction = 0;
			control_signals.left_direction = 1;
			break;
		
		case CLAW_OPEN:
			control_signals.claw_value = 5;
			break;
		
		case CLAW_CLOSE:
			control_signals.claw_value = 0;
			break;
		
		default:
			break;
	}
}

void resetData()
{
	memset((void*)&control_signals, 0, sizeof(control_signals));
	memset((void*)&current_sensor_data, 0, sizeof(current_sensor_data));
	memset((void*)&previous_sensor_data, 0, sizeof(previous_sensor_data));
	
	control_signals.left_direction = 1;
	control_signals.right_direction = 1;
	
	current_command = STEER_STOP;
	throttle = 0;
	
	control_parameters.dist_kd = 250;
	control_parameters.dist_kp = 15;
	control_parameters.line_kd = 40;
	control_parameters.line_kp = 4;
}

// Ta bort denna när den inte används längre.
uint8_t maxAwesome(uint8_t x, uint8_t y)
{
	return (x < y) ? y : x;
}

int main()
{
	cli();
	turn_stack = createTurnStack();
	
	memset((void*)&control_parameters, 0, sizeof(control_parameters));
	
	resetData();
    pwmInit();
    spiSlaveInit();
    sei();
	
	commandToControlSignal(current_command);
	commandToControlSignal(CLAW_OPEN);
	
	pwmWheels(control_signals);
	pwmClaw(control_signals);
	
    while (1)
    {
		if (abort_flag) // Återställs när mode -> manual 
		{
			resetData();
			pwmWheels(control_signals);
			continue;
		}
		else
		{
			// Låt inte Joel köra för fort...
			if (throttle > 100)
			{
				throttle = 100;
			}
			
			if (control_mode_flag == FLAG_MANUAL)
			{
				commandToControlSignal(current_command);
			}
			
			/* TEST
			else if (control_mode_flag == FLAG_AUTO)
			{
				if (new_sensor_data_flag == 1)
				{
					makeTurnTest(RIGHT_TURN);
					driveStraight(60);
				}				
			}
			*/
			else if (control_mode_flag == FLAG_AUTO)
			{
                // Labyrintalgoritmen

                // "Nollställ" gyrot så länge vi befinner oss i en korridor.
				reset_gyro = 1;
				
				if (new_sensor_data_flag == 1)
				{
					if (algo_mode_flag == ALGO_IN)// || algo_mode_flag == ALGO_OUT)
					{
						//detectTurnTest((TurnStack*)&turn_stack); // TEST
						
						if (current_sensor_data.line_type == LINE_NONE)
							detectTurnTest((TurnStack*)&turn_stack);
						else
							handleTape((TurnStack*)&turn_stack, current_sensor_data.line_type);
						
						straightRegulator((const SensorData*)&current_sensor_data, (const SensorData*)&previous_sensor_data);
					}
					else if (algo_mode_flag == ALGO_OUT)
					{
						if (current_sensor_data.line_type == LINE_START_STOP)
							handleTape((TurnStack*)&turn_stack, current_sensor_data.line_type);
						//else
						detectTurnOut(&turn_stack);
							
						straightRegulator((const SensorData*)&current_sensor_data, (const SensorData*)&previous_sensor_data);
					}
					else if (algo_mode_flag == ALGO_START)
					{
						handleTape((TurnStack*)&turn_stack, current_sensor_data.line_type);
						commandToControlSignal(STEER_STRAIGHT);
					}
					else if (algo_mode_flag == ALGO_GOAL)
					{
						commandToControlSignal(CLAW_OPEN);
						lineRegulator(current_sensor_data.line_deviation, previous_sensor_data.line_deviation);
						handleTape((TurnStack*)&turn_stack, current_sensor_data.line_type);
					}
					else if (algo_mode_flag == ALGO_GOAL_REVERSE)
					{
						jamesBondTurn(&turn_stack);
					}
					else if (algo_mode_flag == ALGO_FINISH)
					{
						driveStraight(40);
						algo_mode_flag = ALGO_DONE;
					}
					else
					{
						commandToControlSignal(STEER_STOP);
					}
					
					new_sensor_data_flag = 0;
					
				}
			}
			
			pwmWheels(control_signals);
			pwmClaw(control_signals);
		}	
    }
}
