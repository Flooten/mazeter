/*
 * FILNAMN:       BT-AVR.c
 * PROJEKT:       Mazeter
 * PROGRAMMERARE: Fredrik Stenmark
 *				  Joel Davidsson
 *				  Marcus Eriksson
 * 				  Mattias Fransson
 * DATUM:         2013-05-06
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
#include "sensor_parameters.h"
#include "control_parameters.h"

// Definierar tiden ??? ANV�NDS INTE
#define DELTA_T 2 // i ms 

// Variabler
volatile uint8_t control_mode_flag;
volatile uint8_t control_command;
volatile uint8_t sensor_command;
volatile SensorData sensor_data;
volatile ControlSignals control_signals;
volatile SensorParameters sensor_parameters;
volatile ControlParameters control_parameters;
uint8_t new_control_parameters = 0;
volatile uint8_t throttle;
volatile uint8_t start;

// Funktioner som initierar kommunikationsenheten
void ioInit()
{
	// S�tter upp in och ut f�r BT
	DDRD = (1 << DDD4);
	
	// S�tter vilket mode vi befinner oss i
	if (((1 << PIND2) & PIND) == 0x04)
		control_mode_flag = FLAG_AUTO;
	else
		control_mode_flag = FLAG_MANUAL;
		
	// Ser till att roboten inte �ker iv�g vid uppstarten
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

ISR(INT0_vect)
{
	// Skjutomkopplaren som st�ller in autonomt/manuellt l�ge
	if (((1 << PIND2) & PIND) == 0x04)
	{
		control_mode_flag = FLAG_AUTO;
	}		
	else
	{
		control_mode_flag = FLAG_MANUAL;
		start = 0;
		control_command = STEER_STOP;
	}
}

ISR(INT1_vect)
{
	// Initierar start om roboten �r i autonomt l�ge
	if (control_mode_flag == FLAG_AUTO)
	{
		start = 1;
		btSendData(RUN_START, NULL, 0);
	}		
}

ISR(TIMER1_COMPA_vect)
{
	// S�tter flaggan att det �r dags att beg�ra data
	timer_internal_ready = 1; 
}

ISR(TIMER3_COMPA_vect)
{
	// S�tter flaggan att det �r dags att skicka data till datorn
	timer_external_ready = 1;
}

int main(void)
{
	cli();
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
	throttle = 80;
	
	uint8_t turn_stack_top = 0;
	uint8_t algo_state = 0;
	//uint8_t turn_done_flag = 0;
	uint8_t new_node = 0;
	uint8_t reset_gyro = 0;
	
	sei();
	
    while(1)
    {
		// Om den interna timern �r klar p�b�rjas ny SPI-cykel
		if (timer_internal_ready)
		{
			// L�s sensordata fr�n sensorenheten via bussen
			if (spiReadData(SENSOR_DATA_ALL, SENSOR_ENHET, (uint8_t*)&sensor_data.distance1, sizeof(sensor_data)) != SENSOR_DATA_ALL)
			{
				btSendString("Failed to fetch the sensor data from the sensor device.");
			}
			
			// Skicka vilket mode roboten �r i till styrenheten
			spiSendData(control_mode_flag, STYR_ENHET, NULL, 0);
			
			if (control_mode_flag == FLAG_MANUAL)
			{
				// Skicka gasp�drag till styrenheten
				spiSendData(CONTROL_THROTTLE, STYR_ENHET, (const uint8_t*)&throttle, 1);
				
				// Skicka styrkommandon fr�n datorn till styrenheten
				if (spiSendCommand(control_command, STYR_ENHET) == ERROR_UNKNOWN_SPI_COMMAND)
				{
					btSendString("Failed to send control commands to the control device.");
				}
				
				// Kalibrera linjesensor om det ska g�ras
				if(sensor_command != 0)
				{
					spiSendCommand(sensor_command, SENSOR_ENHET);
					sensor_command = 0;
				}
				
				// Skicka parametrar fr�n datorn till styrenheten
				if (new_control_parameters)
				{
					if (spiSendData(CONTROL_PARAMETERS_ALL, STYR_ENHET, (const uint8_t*)&control_parameters.dist_kp, sizeof(control_parameters)) != CONTROL_PARAMETERS_ALL)
					{
						btSendString("Failed to send the control parameters to the control device.");
					}
					new_control_parameters = 0;
				}				
			}
			else
			{
				// Skicka abort till styrenheten om det ska g�ras
				if (control_command == ABORT)
				{
					spiSendCommand(control_command, STYR_ENHET);
				}
				
				// Skicka sensordata till styrenheten om roboten k�r i autonomt l�ge
				if (start == 1)
				{
					if (spiSendData(SENSOR_DATA_ALL, STYR_ENHET, (const uint8_t*)&sensor_data.distance1, sizeof(sensor_data)) != SENSOR_DATA_ALL)
					{
						btSendString("Failed to send the sensor data to the control device.");
					}
				}
				
				// S�tter gyro till 120 grader om reset_gyro �r 1
				spiReadData(RESET_GYRO, STYR_ENHET, &reset_gyro, 1);
				spiSendData(RESET_GYRO, SENSOR_ENHET, &reset_gyro, 1);
				
				// Skickar senaste sv�ngen till datorn
				spiReadData(CHECK_STACK, STYR_ENHET, &new_node, 1);
				if (new_node == 1)
				{
					spiReadData(TURN_STACK_TOP, STYR_ENHET, &turn_stack_top, 1);
					btSendData(TURN_STACK_TOP, &turn_stack_top, 1);
				}
				
				// Fr�gar styrenheten vilken algoritm roboten �r i
				spiReadData(ALGO_STATE, STYR_ENHET, &algo_state, 1);
			}
			
			// L�ser kontrollsignaler fr�n styrenheten
			spiReadData(CONTROL_SIGNALS, STYR_ENHET, (uint8_t*)&control_signals.right_value, sizeof(control_signals));		
			// Nollst�ller flaggan som den interna timern uppdaterar
			timer_internal_ready = 0;
		}		
		
		// Om den externa timern �r klar p�b�rjas ny bl�tands-cykel
		if (timer_external_ready)
		{			
			// Skickar till datorn vilken algoritm roboten �r i om den �r i autonom k�rning 
			if (control_mode_flag == FLAG_AUTO)
			{
				btSendData(ALGO_STATE, &algo_state, 1);
			}
			
			// S�nder kontrollsignaler emottagna fr�n styrenheten till datorn
			btSendData(CONTROL_SIGNALS, (const uint8_t*)&control_signals.right_value, sizeof(control_signals));
			// S�nder sensordata emottagna fr�n sensorenheten till datorn
			btSendData(SENSOR_DATA_ALL, (const uint8_t*)&sensor_data.distance1, sizeof(sensor_data));
			// S�nder vilken mode roboten �r i till datorn
			btSendData(control_mode_flag, NULL, 0);
			// Nollst�ller flaggan som den externa timern uppdaterar
			timer_external_ready = 0;
		}

		// H�r hanteras k�n f�r bl�tandskommunikation
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
						
					case ABORT:
						control_command = ABORT;
						start = 0;
						break;
					
					case CALIBRATE_LINE_SENSOR:
						sensor_command = CALIBRATE_LINE_SENSOR;
						break;
						
					case CONTROL_PARAMETERS_ALL:
						control_parameters.dist_kp = tmp->data[0];
						control_parameters.dist_kd = tmp->data[1];
						control_parameters.line_kp = tmp->data[2];
						control_parameters.line_kd = tmp->data[3];
						break;
					
					case PARA_DIST_KD:
						control_parameters.dist_kd = tmp->data[0];
						new_control_parameters = 1;
						break;
						
					case PARA_DIST_KP:
						control_parameters.dist_kp = tmp->data[0];
						new_control_parameters = 1;
						break;
					
					case PARA_LINE_KD:
						control_parameters.line_kd = tmp->data[0];
						new_control_parameters = 1;
						break;
						
					case PARA_LINE_KP:
						control_parameters.line_kp = tmp->data[0];
						new_control_parameters = 1;
						break;
						

					default:
						btSendString("Unknown command");
						break;
				}

				btSendNode(tmp);
				pop((Queue*)&rx_queue);
			}
		} // END ATOMIC_BLOCK
	}
	
	return 0;
}
