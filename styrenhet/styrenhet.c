/*
 * FILNAMN:       styrenhet.c
 * PROJEKT:       Mazeter
 * PROGRAMMERARE: Fredrik Stenmark
 * DATUM:         2013-04-08
 *
 * BESKRIVNING:
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

/* TEST  ---------------- */
#ifndef F_CPU
#define F_CPU 8000000UL
#endif
#include <util/delay.h> 
/* TEST ----------------- */


#define SPI_RECEIVING_SENSOR_DATA 0x03

// SPI-variabler
volatile uint8_t* buffer;
volatile uint8_t buffer_size;
volatile uint8_t current_byte;
volatile uint8_t spi_status;

volatile uint8_t control_mode_flag;
volatile uint8_t current_command;
volatile uint8_t throttle;
volatile uint8_t new_sensor_data = 0;

volatile ControlSignals control_signals;
volatile SensorData current_sensor_data;
volatile SensorData previous_sensor_data;
volatile ControlParameters control_parameters;

volatile TurnStack* turn_stack;

volatile uint8_t mah_2nd_const = 129;

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
			new_sensor_data = 0;
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
			buffer = &control_parameters.right_kp;
			buffer_size = sizeof(control_parameters);
			current_byte = 0;
			spi_status = SPI_RECEIVING_DATA;
			break;
			
		case 0x96:
			SPDR = 0x96;
			buffer = &mah_2nd_const;
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

int main()
{
	memset((void*)&control_signals, 0, sizeof(control_signals));
	memset((void*)&control_parameters, 0, sizeof(control_parameters));
	memset((void*)&current_sensor_data, 0, sizeof(current_sensor_data));
	memset((void*)&previous_sensor_data, 0, sizeof(previous_sensor_data));
	
    spi_status = SPI_READY;
    buffer = NULL;
    buffer_size = 0;
    current_byte = 0;
	
	current_command = STEER_STOP;
	throttle = 0;
	
    pwmInit();
    spiSlaveInit();
    sei();
	
	commandToControlSignal(current_command);
	commandToControlSignal(CLAW_OPEN);
	
	pwmWheels(control_signals);
	pwmClaw(control_signals);
	
	
	control_parameters.left_kd = 1;
	control_parameters.left_kp = 1;
	control_parameters.right_kd = 1;
	control_parameters.right_kp = 1;
	
    while (1)
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
			if (new_sensor_data == 1)
			{
				sensorDataToControlSignal((const SensorData*)&current_sensor_data, (const SensorData*)&previous_sensor_data);
				new_sensor_data = 0;
			}
		}
		
		pwmWheels(control_signals);
		pwmClaw(control_signals);
    }
}