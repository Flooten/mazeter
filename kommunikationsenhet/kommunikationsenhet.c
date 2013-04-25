/*
 * FILNAMN:       BT-AVR.c
 * PROJEKT:       Mazeter
 * PROGRAMMERARE: Fredrik Stenmark
 *				  Joel Davidsson
 *				  Marcus Eriksson
 * 				  Mattias Fransson
 * DATUM:         2013-04-08
 *
 * BESKRIVNING:   Inneh�ller kommunikationsenhetens funktionalitet.
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
volatile uint8_t throttle;
volatile uint8_t start;

// Funktioner som initierar kommunikationsenheten
void ioInit()
{
	// S�tter upp in och ut f�r BT, samt en bit som har med LCD:n att g�ra
	//DDRD = 0x22;
	DDRD = (1 << DDD4);
	
	// S�tter vilket mode vi befinner oss i
	if (((1 << PIND2) & PIND) == 0x04)
		control_mode_flag = FLAG_AUTO;
	else
		control_mode_flag = FLAG_MANUAL;
		
	start = 0;
	
	// Interupt init
	MCUCR = 0x03;
	EICRA = 0x0D;
	EIMSK = 0x03;
	sei();
}

// Avbrottsrutiner
ISR (USART0_RX_vect)
{
	btReadData();
}

ISR (INT0_vect)
{
	if (((1 << PIND2) & PIND) == 0x04)
		control_mode_flag = FLAG_AUTO;
	else
		{
			control_mode_flag = FLAG_MANUAL;
			start = 0;
		}
}

ISR (INT1_vect)
{
	start = 1;
}

ISR(TIMER1_COMPA_vect)
{
	timer_internal_ready = 1; // S�tter flaggan att det �r dags att beg�ra data
}

ISR(TIMER3_COMPA_vect)
{
	timer_external_ready = 1; // S�tter flaggan att det �r dags att skicka data till datorn
}

int main(void)
{
	ioInit();
	spiMasterInit();
	btInit();
	timerInit();
	
	// Initiera k�n som h�ller kommandon fr�n PC.
	rx_queue = createQueue();
	bt_connected = 0;
	
	// S�tt roboten i manuell k�rning.
	control_mode_flag = FLAG_MANUAL;
	control_command = STEER_STOP;
	throttle = 62;
	
	//! Ta bort, test
	DDRA = 0xFF;
	
    while(1)
    {
		PORTA = sizeof(sensor_data);
		
		if (timer_internal_ready)
		{
			// L�s sensordata
			//if (spiReadData(SENSOR_DATA_ALL, SENSOR_ENHET, (uint8_t*)&sensor_data.distance1, sizeof(sensor_data)) != SENSOR_DATA_ALL)
			//{
				////btSendString("Failed to fetch the sensor data from the sensor device.");
			//}
			
			spiSendData(control_mode_flag, STYR_ENHET, NULL, 0);
			
			if (control_mode_flag == FLAG_MANUAL)
			{
				spiSendData(CONTROL_THROTTLE, STYR_ENHET, (const uint8_t*)&throttle, 1);
				
				if (spiSendCommand(control_command, STYR_ENHET) == ERROR_UNKNOWN_SPI_COMMAND)
				{
					btSendString("Failed to send control commands to the control device.");
				}
			}
			else
			{
				if (start == 1)
				{
					if (spiSendData(SENSOR_DATA_ALL, STYR_ENHET, (const uint8_t*)&sensor_data.distance1, sizeof(sensor_data)) != SENSOR_DATA_ALL)
					{
						btSendString("Failed to send the sensor data to the control device.");
					}
				}
			}
			
			spiReadData(SENSOR_DATA_ALL, SENSOR_ENHET, (uint8_t*)&sensor_data.distance1, sizeof(sensor_data));
			spiReadData(CONTROL_SIGNALS, STYR_ENHET, (uint8_t*)&control_signals.right_value, sizeof(control_signals));
			timer_internal_ready = 0;
		}		
		
		if (timer_external_ready)
		{	
			btSendData(CONTROL_SIGNALS, (const uint8_t*)&control_signals.right_value, sizeof(control_signals));
			btSendData(SENSOR_DATA_ALL, (const uint8_t*)&sensor_data.distance1, sizeof(sensor_data));
			btSendData(control_mode_flag, NULL, 0);
			timer_external_ready = 0;
		}
		
		//cli();
		ATOMIC_BLOCK(ATOMIC_FORCEON)
		{
			while (empty((const Queue*)&rx_queue) == 0)
			{
				Node* tmp = top((const Queue*)&rx_queue);
			
				switch (tmp->command)
				{
					case BT_CONNECT:
						bt_connected = 1;
						break;
				
					case BT_DISCONNECT:
						bt_connected = 0;
						break;
				
					case CONTROL_THROTTLE:
						throttle = tmp->data[0];
						break;
				
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
		} // END ATOMIC_BLOCK
		//sei();
	}
	
	return 0;
}