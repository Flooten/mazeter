/*
 * FILNAMN:       styrenhet.c
 * PROJEKT:       Mazeter
 * PROGRAMMERARE: Fredrik Stenmark
 *                Herman Ekwall
 *                Mattias Fransson
 * DATUM:         2013-05-02
 *
 * BESKRIVNING: Styrenhetens huvudloop.
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
volatile uint8_t reciving_sensor_data_flag = 0;
volatile uint8_t abort_flag = 0;
volatile uint8_t algo_mode_flag = ALGO_IN;
volatile uint8_t turn_stack_top = 0x11;

volatile ControlSignals control_signals;
volatile SensorData current_sensor_data;
volatile SensorData previous_sensor_data;
volatile ControlParameters control_parameters;

volatile uint8_t turn_done_flag = 0;
volatile uint8_t turn_done_flag_copy;

volatile const char* str = "Hello, world!";

volatile TurnStack turn_stack;

void parseCommand(uint8_t cmd);

ISR(SPI_STC_vect)
{
    uint8_t received = SPDR;
    
    if (spi_status == SPI_RECEIVING_DATA)
    {
        if (buffer == NULL || current_byte >= buffer_size)
        {
            SPDR = ERROR_SPI;
            return;
        }
        
        buffer[current_byte++] = received;
        
        if (current_byte == buffer_size)
        {
            spi_status = SPI_READY;
            buffer = NULL;
            buffer_size = 0;
            current_byte = 0;
			
			if(reciving_sensor_data_flag == 1)
			{
				new_sensor_data_flag = 1;
				reciving_sensor_data_flag = 0;
			}
			
        }
    }
    else
    {
        if (received == 0)
        {
            if (buffer == NULL || current_byte == buffer_size)
            {
                SPDR = ERROR_SPI;
                return;
            }
            
            SPDR = buffer[current_byte++];
			
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
            parseCommand(received);
        }
    }
}

void parseCommand(uint8_t cmd)
{
    switch (cmd)
    {
        case SENSOR_DATA_ALL:
            SPDR = SENSOR_DATA_ALL;
            buffer = &current_sensor_data.distance1;
            buffer_size = sizeof(current_sensor_data);
            current_byte = 0;
            spi_status = SPI_RECEIVING_DATA;
			new_sensor_data_flag = 0;
			reciving_sensor_data_flag = 1;
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

		case 0x95:
			SPDR = strlen((const char*)str);
			buffer = (uint8_t*)str;
			buffer_size = strlen((const char*)str);
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
			control_signals.left_value = throttle / 2;
			control_signals.right_direction = 1;
			control_signals.left_direction = 1;
			break;
		
		case STEER_STRAIGHT_RIGHT:
			control_signals.right_value = throttle /2;
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
			else if (control_mode_flag == FLAG_AUTO)
			{
				if (new_sensor_data_flag == 1)
				{
					handleTape(&turn_stack, current_sensor_data.line_type);
					if (algo_mode_flag == ALGO_IN)
					{
						detectTurn(&turn_stack);
						straightRegulator((const SensorData*)&current_sensor_data, (const SensorData*)&previous_sensor_data);
					}
					else if (algo_mode_flag == ALGO_GOAL)
					{
						lineRegulator(current_sensor_data.line_deviation, previous_sensor_data.line_deviation);
					}
					else if (algo_mode_flag == ALGO_GOAL_REVERSE)
					{
						jamesBondTurn(&turn_stack);
					}
					else if (algo_mode_flag ==	ALGO_OUT)
					{
						detectTurnOut(&turn_stack);
						straightRegulator((const SensorData*)&current_sensor_data, (const SensorData*)&previous_sensor_data);
					}
					
					new_sensor_data_flag = 0;
				}
			}				
			/*
			else if (control_mode_flag == FLAG_AUTO)
			{
				if (new_sensor_data_flag == 1)
				{
					if (current_sensor_data.distance1 < THRESHOLD_ABORT || current_sensor_data.distance2 < THRESHOLD_ABORT)
					{
						// Stanna roboten om vi är på väg in i något
						commandToControlSignal(STEER_STOP);
						pwmWheels(control_signals);
					}
					
					if (current_sensor_data.line_type == LINE_GOAL)
					{
						lineRegulator(current_sensor_data.line_deviation, previous_sensor_data.line_deviation);
					}
					else if (current_sensor_data.line_type == LINE_GOAL_STOP)
					{
						commandToControlSignal(STEER_STOP);
					}
					else
					{
						straightRegulator((const SensorData*)&current_sensor_data, (const SensorData*)&previous_sensor_data);
						//detectTurn(&turn_stack);
					}
					new_sensor_data_flag = 0;
				}
			}
			*/
			
			pwmWheels(control_signals);
			pwmClaw(control_signals);
		}	
    }
}

///////////////////////////// TA INTE BORT /////////////////////////////////
// Detta är labyrint algoritmen

/*
else if (control_mode_flag == FLAG_AUTO)
{
	if (new_sensor_data_flag == 1)
	{
		handleTape(turn_stack, current_sensor_data.line_type);
		
		if (algo_mode_flag == ALGO_IN)
		{
			detectTurn(turn_stack);
			sensorDataToControlSignal(current_sensor_data, previous_sensor_data);
		}
		else if (algo_mode_flag == ALGO_GOAL)
		{
			lineRegulator(current_sensor_data.line_deviation, previous_sensor_data.line_deviation);
		}
		else if (algo_mode_flag == ALGO_GOAL_REVERSE)
		{
			jamesBondTurn(turn_stack);
		}
		else if (algo_mode_flag ==	ALGO_OUT)
		{
			detectTurnOut(turn_stack);
			sensorDataToControlSignal(current_sensor_data, previous_sensor_data);
		}
		
		new_sensor_data_flag = 0;
	}		
		
*/
