/*
 * FILNAMN:       BT-AVR.c
 * PROJEKT:       Mazeter
 * PROGRAMMERARE: Fredrik Stenmark
 *				  Joel Davidsson
 *				  Marcus Eriksson
 * 				  Mattias Fransson
 * DATUM:         2013-04-08
 *
 * BESKRIVNING:   Innehåller kommunikationsenhetens funktionalitet.
 *          
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include "spi.h"
#include "bt.h"
#include "timer.h"
#include "sensor_data.h"
#include "controlsignals.h"

volatile uint8_t control_mode_flag;
volatile uint8_t control_command;
volatile SensorData sensor_data;
volatile ControlSignals control_signals;

volatile uint16_t timer_signals[1];

// Funktioner som initierar kommunikationsenheten
void ioInit()
{
	// Sätter upp in och ut för BT, samt en bit som har med LCD:n att göra
	//DDRD = 0x22;
	DDRD = (1 << DDD4);
	
	// Interupt init
	MCUCR = 0x03;
	EICRA = 0x03;
	EIMSK = 0x01;
	sei();
}

// Avbrottsrutiner
ISR (USART0_RX_vect)
{
	btReadData();
}

ISR (INT0_vect)
{
}

ISR(TIMER1_COMPA_vect)
{
	timer_internal_ready = 1; // Sätter flaggan att det är dags att begära data
}

ISR(TIMER3_COMPA_vect)
{
	timer_external_ready = 1; // Sätter flaggan att det är dags att skicka data till datorn
}

int main(void)
{
	ioInit();
	spiMasterInit();
	btInit();
	timerInit();
	
	// Initiera kön som håller kommandon från PC.
	rx_queue = createQueue();
	
	// Sätt roboten i manuell körning.
	control_mode_flag = FLAG_MANUAL;
	control_command = STEER_STOP;
	
    while(1)
    {
		if (timer_internal_ready)
		{
			// Läs sensordata
			// retval = spiReadData(SENSOR_DATA_ALL, SENSOR_ENHET, &sensor_data.distance1, sizeof(sensor_data));
			
			if (control_mode_flag == FLAG_MANUAL)
			{
				if (spiSendCommand(control_command, STYR_ENHET) == ERROR_UNKNOWN_SPI_COMMAND)
				{
					btSendString("Kunde inte skicka styrkommandot till styrenheten");
				}
			}
			else
			{
				if (spiSendData(SENSOR_DATA_ALL, SENSOR_ENHET, (const uint8_t*)&sensor_data.distance1, sizeof(sensor_data)) != SENSOR_DATA_ALL)
				{
					btSendString("Kunde skicka sensordata till styrenheten");
				}
			}
			
			spiReadData(CONTROL_SIGNALS, STYR_ENHET, (uint8_t*)&control_signals.right_value, sizeof(control_signals));
			timer_internal_ready = 0;
		}		
		
		if (timer_external_ready)
		{
			//btSendData(CONTROL_SIGNALS, &control_signals.right_value, sizeof(control_signals));
			//btSendData(SENSOR_DATA_ALL, &sensor_data.distance1, sizeof(sensor_data));
			timer_external_ready = 0;
		}
		
		cli();
		while (empty((const Queue*)&rx_queue) == 0)
		{
			Node* tmp = top((const Queue*)&rx_queue);
			
			switch (tmp->command)
			{
				case STEER_STRAIGHT:
					control_command = STEER_STRAIGHT;
					break;
					
				case STEER_STRAIGHT_LEFT:
					control_command = STEER_STRAIGHT_LEFT;
					break;
				
				case STEER_STRAIGHT_RIGHT:
					control_command = STEER_STRAIGHT_RIGHT;
					break;
				
				case STEER_BACK:
					control_command = STEER_BACK;
					break;
				
				case STEER_STOP:
					control_command = STEER_STOP;
					break;
				
				case STEER_ROTATE_LEFT:
					control_command = STEER_ROTATE_LEFT;
					break;
				
				case STEER_ROTATE_RIGHT:
					control_command = STEER_ROTATE_RIGHT;
					break;
				
				case CLAW_OPEN:
					control_command = CLAW_OPEN;
					break;
				
				case CLAW_CLOSE:
					control_command = CLAW_CLOSE;
					break;

				default:
					btSendString("Unknown command");
					break;
			}

			btSendNode(tmp);
			pop((Queue*)&rx_queue);
		}
		sei();
	}
	
	return 0;
}