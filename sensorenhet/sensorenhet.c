/*
 * FILNAMN:       sensorenheten.c
 * PROJEKT:       Mazeter
 * PROGRAMMERARE: Fredrik Stenmark och Herman Ekwall
 *                
 * DATUM:         2013-04-02
 *
 * BESKRIVNING:
 *
 */

#include "sensorenhet.h"
#include "line_calibration.h"

#ifndef F_CPU
#define F_CPU 8000000UL
#endif
#include <util/delay.h>

/* för test */
volatile uint8_t tmp = 1;

void ioInit()
{
	DDRD = 0xFF; // Sätter upp så att muxarna kan styras.
	DDRA = 0x00; // Sätter upp så att sensorerna kan avläsas.
}

void test_timer_init()
{
	TCCR3B |= (1 << WGM32); // Sätter upp CTC-mode för timer 3
	TIMSK3 |= (1 << OCIE3A); // Tillåter CTC interupt

	OCR3A = 35000; // Sätter vad den ska räkna till.

	TCCR3B |= ((1 << CS30) | (0 << CS31) | (1 << CS32)); // Sätter prescaler 
}

ISR(TIMER3_COMPA_vect)
{
	tmp = 0; 
}

void initADC()
{
	ADCSRA = (1 << ADEN) | (1 << ADIE) | (1 << ADPS2) | (1 << ADPS1); // Prescaler, ADPS1 kan sättas till 0 om snabbare konvertering krävs 
	ADMUX = (1 << ADLAR);
}

void startTimer()
{	
	TCCR1B = 1 << CS10;
}

void stopTimer()
{
	TCCR1B = 0x00; 
}

uint16_t restartTimer()
{
	uint16_t tmp;
	stopTimer();
	
	tmp = TCNT1;
	

	TCNT1 = 0x0000;
	TIFR1 |= (1 << TOV1); 

	
	startTimer();
	return tmp;
}

void startADC()
{
	readGyroData();
	ADMUX |= (1 << MUX2) | (1 << MUX1) | (1 << MUX0); // Väljer pinne att AD-omvandla
	ADCSRA |= (1 << ADSC);
	startTimer();
	
	current_sensor = GYRO_SAMPLE_1;
	
}

void readLine(uint8_t diod)
{
	if (diod <= 11 && diod >= 0)
	{
		PORTD = (diod << 4) | diod;
	}
	else
	{
		PORTD = 0xFF;
	}		
}

void readGyroData()
{
	PORTD = 0xFB;	
}

void readGyroTemp()
{
	PORTD = 0xFC;
}

ISR(ADC_vect)
{
	switch (current_sensor)
	{
		// --------------- Avståndssensorer ---------------
		
		case DISTANCE_1:
		distance1.value = ADCH;
		distance1.is_converted = 0;
		distance1.sensor_type = DISTANCE_1;
		
		current_sensor = DISTANCE_2;
		ADMUX = 0x21; // Ingång ADC1 
		break;
		
		case DISTANCE_2:
		distance2.value = ADCH;
		distance2.is_converted = 0;
		distance2.sensor_type = DISTANCE_2;
		
		current_sensor = DISTANCE_3;
		ADMUX = 0x22; // Ingång ADC2
		break;
		
		case DISTANCE_3:
		distance3.value = ADCH;
		distance3.is_converted = 0;
		distance3.sensor_type = DISTANCE_3;
		
		current_sensor = DISTANCE_4;
		ADMUX = 0x23; // Ingång ADC3
		break;
		
		case DISTANCE_4:
		distance4.value = ADCH;
		distance4.is_converted = 0;
		distance4.sensor_type = DISTANCE_4;

		current_sensor = DISTANCE_5;
		ADMUX = 0x24; // Ingång ADC5
		readLine(0xFF); // Avaktiverar muxarna
		break;
		
		case DISTANCE_5:
		distance5.value = ADCH;
		distance5.is_converted = 0;
		distance5.sensor_type = DISTANCE_5;
				
		current_sensor = DISTANCE_6;
		ADMUX = 0x25; // Ingång ADC5
		break;
				
		case DISTANCE_6:
		distance6.value = ADCH;
		distance6.is_converted = 0;
		distance6.sensor_type = DISTANCE_6;
				
		current_sensor = DISTANCE_7;
		ADMUX = 0x26; // Ingång ADC6
		break;
				
		case DISTANCE_7:
		distance7.value = ADCH;
		distance7.is_converted = 0;
		distance7.sensor_type = DISTANCE_7;
				
		current_sensor = GYRO_SAMPLE_1;
		ADMUX = 0x27; // Ingång ADC7
		readGyroData();
		break;
		
		// --------------- Linjesensor ---------------
		
		case LINE_SENSOR_0:
		line_sensor.value[0] = ADCH;
		current_sensor = LINE_SENSOR_5;
		ADMUX = 0x27; // Ingång ADC7
		readLine(LINE_SENSOR_5);
		break;
		
		case LINE_SENSOR_1:
		line_sensor.value[1] = ADCH;
		current_sensor = LINE_SENSOR_6;
		ADMUX = 0x27; // Ingång ADC7
		readLine(LINE_SENSOR_6);
		break;
		
		case LINE_SENSOR_2:
		line_sensor.value[2] = ADCH;
		current_sensor = LINE_SENSOR_7;
		ADMUX = 0x27; // Ingång ADC7
		readLine(LINE_SENSOR_7);
		break;
		
		case LINE_SENSOR_3:
		line_sensor.value[3] = ADCH;
		current_sensor = LINE_SENSOR_8;
		ADMUX = 0x27; // Ingång ADC7
		readLine(LINE_SENSOR_8);
		break;
		
		case LINE_SENSOR_4:
		line_sensor.value[4] = ADCH;
		current_sensor = LINE_SENSOR_9;
		ADMUX = 0x27; // Ingång ADC7
		readLine(LINE_SENSOR_9);
		break;
		
		case LINE_SENSOR_5:
		line_sensor.value[5] = ADCH;
		current_sensor = LINE_SENSOR_1;
		ADMUX = 0x27; // Ingång ADC7
		readLine(LINE_SENSOR_1);
		break;		
		
		case LINE_SENSOR_6:
		line_sensor.value[6] = ADCH;
		current_sensor = LINE_SENSOR_2;
		ADMUX = 0x27; // Ingång ADC7
		readLine(LINE_SENSOR_2);
		break;
		
		case LINE_SENSOR_7:
		line_sensor.value[7] = ADCH;
		current_sensor = LINE_SENSOR_3;
		ADMUX = 0x27; // Ingång ADC7
		readLine(LINE_SENSOR_3);
		break;
		
		case LINE_SENSOR_8:
		line_sensor.value[8] = ADCH;
		//line_sensor.is_converted = 0;
		//current_sensor = DISTANCE_1;
		//ADMUX = 0x20; // Ingång ADC0
		//readLine(0xFF); // Avaktiverar muxarna
		current_sensor = LINE_SENSOR_4;
		ADMUX = 0x27;
		readLine(LINE_SENSOR_4);
		break;
		
		case LINE_SENSOR_9:
		line_sensor.value[9] = ADCH;
		line_sensor.is_converted = 0;
		current_sensor = DISTANCE_1;
		ADMUX = 0x20; // Ingång ADC0
		readLine(0xFF); // Avaktiverar muxarna
		//current_sensor = LINE_SENSOR_1;
		//ADMUX = 0x27; // Ingång ADC7
		//readLine(LINE_SENSOR_1);
		break;
		
		case LINE_SENSOR_10:
		line_sensor.value[10] = ADCH;
		current_sensor = LINE_SENSOR_0;
		ADMUX = 0x27; // Ingång ADC7
		readLine(LINE_SENSOR_0);
		break;		
		
		// --------------- Gyro ---------------
		
		case GYRO_SAMPLE_1:
		gyro_sample1.value = ADCH;
		gyro_sample1.is_converted = 0;
		gyro_sample1.sensor_type = GYRO_SAMPLE_1;
		gyro_sample1.time = restartTimer();

		current_sensor = GYRO_TEMP;
		ADMUX = 0x27; // Ingång ADC7
		readGyroTemp();
		break;
		
		case GYRO_TEMP:
		gyro_temp.value = ADCH;
		gyro_temp.is_converted = 0;
		gyro_temp.sensor_type = GYRO_TEMP;
		
		current_sensor = LINE_SENSOR_10;
		ADMUX = 0x27;
		readLine(LINE_SENSOR_10);
		break;
		
	} 
	
	ADCSRA |= (1 << ADSC); // Startar en ny AD-omvandling
}

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
			if (buffer == NULL || current_byte >= buffer_size)
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
			buffer = &sensor_data.distance1;
			buffer_size = sizeof(sensor_data);
			current_byte = 0;
			break;
		
		case SENSOR_DISTANCE1:
			SPDR = SENSOR_DISTANCE1;
			buffer = &sensor_data.distance1;
			buffer_size = 1;
			current_byte = 0;
			break;
		
		case CALIBRATE_LINE_SENSOR:
			SPDR = CALIBRATE_LINE_SENSOR;
			calibrate_line_sensor = 1;
			break;
		
		default:
			SPDR = ERROR_UNKNOWN_SPI_COMMAND;
			buffer = NULL;
			buffer_size = 0;
			current_byte = 0;
			break;
	}
}

int main()
{
	spi_status = SPI_READY;
	buffer = NULL;
	buffer_size = 0;
	current_byte = 0;
	
	sensor_parameters.tape_threshold = 100;
	sensor_parameters.horizontal_line_threshold = 4;
	sensor_parameters.no_line_detection_threshold = 100;
	sensor_parameters.line_diff_threshold = 5;
	sensor_parameters.horizontal_to_vertical_threshold = 30;

	ioInit();
	initADC();
	initGYRO();
	
	//spiSlaveInit();
	sei();
	
	startADC();
	
	while (1)
	{

		if (calibrate_line_sensor)
		{
			uint8_t tape_value = calibrateLineSensorTape((const RawLineData*)&line_sensor);
			waitFiveSeconds();
			uint8_t floor_value = calibrateLineSensorFloor((const RawLineData*)&line_sensor);
			uint16_t sum = tape_value + floor_value;
			sensor_parameters.tape_threshold = sum / 2;
			calibrate_line_sensor = 0;
		}
		
		//convertAllData();
		convertLineData((RawLineData*)&line_sensor);
	}
}