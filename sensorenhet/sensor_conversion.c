/*
 * FILNAMN:       sensor_conversion.c
 * PROJEKT:       Mazeter
 * PROGRAMMERARE: Martin Andersson
 *                Herman Ekwall
 *                Mattias Fransson
 *
 * DATUM:         2013-05-17
 *
 * BESKRIVNING: Hanterar konvertering av rådatan från samtliga sensorer till enheter som har en mer direkt tolkning.
 *
 */

#include <stdint.h>
#include <stdlib.h>
#include "sensor_conversion.h"
#include "look_up.h"
#include "line_calibration.h"

volatile uint8_t conversion_status;
uint8_t line_detections = 0;
uint8_t no_line_detections = 0;
uint8_t current_line = 0;
uint8_t first_line_val = 0;
uint8_t second_line_val = 0;
uint8_t goal_mode = 0;

/* filter parameters multiplied with 1000 */
void initGYRO()
{
	filter_coeff[0]=50;
	filter_coeff[1]=32;
	filter_coeff[2]=-580;
	filter_coeff[3]=583;
	filter_coeff[4]=4915;
	filter_coeff[5]=4915;
	filter_coeff[6]=583;
	filter_coeff[7]=-580;
	filter_coeff[8]=32;
	filter_coeff[9]=50;
	
	uint8_t i;
	for (i = 0; i < NR_OF_GYRO_SAMPLES; i++)
	{
		gyro_samples[i]=0;
	}
}

void convertAllData()
{
	convertRawDataGyro((RawDataGyro*)&gyro_sample1);
	
	convertRawData((RawData*)&distance1);
	convertRawData((RawData*)&distance2);
	convertRawData((RawData*)&distance3);
	convertRawData((RawData*)&distance4);
	
	convertRawData((RawData*)&distance5);
	convertRawData((RawData*)&distance6);
	convertRawData((RawData*)&distance7);
	
	convertLineData((RawLineData*)&line_sensor);
	
	
}

void convertRawData(RawData* data)
{
	// Konvertering av avståndssensorerna
	if (!data->is_converted)
	{
		switch (data->sensor_type)
		{
			case DISTANCE_1:
				sensor_data.distance1 = lookUpDistance(data->value, data->sensor_type);
				break;
			case DISTANCE_2:
				sensor_data.distance2 = lookUpDistance(data->value, data->sensor_type);
				break;
			case DISTANCE_3:
				sensor_data.distance3 = lookUpDistance(data->value, data->sensor_type);
				break;
			case DISTANCE_4:
				sensor_data.distance4 = lookUpDistance(data->value, data->sensor_type);
				break;
			case DISTANCE_7:
				sensor_data.distance7 = lookUpDistance(data->value, data->sensor_type);
				break;
				
			case DISTANCE_5:
				sensor_data.distance5 = lookUpDistance(data->value, data->sensor_type);
				if (sensor_data.distance5 != 255)
					sensor_data.distance5 += 2;
				break;
			case DISTANCE_6:
				sensor_data.distance6 = lookUpDistance(data->value, data->sensor_type);
				break;
		
			default:
				conversion_status = CONVERSION_ERROR;
				break;
		}
		
		data->is_converted = 1;
	}
}

/* Integrering av vinkelhastigheten för att få fram vinkeln */
void convertRawDataGyro(volatile RawDataGyro* data)
{	
	// Konvertera endast om nödvändigt
	if (!data->is_converted)
	{	
		// Skifta alla värden i gyro_samples ett steg för att göra plats åt ett nytt värde
		uint8_t i;
		for (i = 0; i < NR_OF_GYRO_SAMPLES-1; i++)
		{
			gyro_samples[i] = gyro_samples[i+1];
		}
	
		if (data->value >= GYRO_REF_LEVEL)
		{
			/* positiv ändring */
			gyro_samples[NR_OF_GYRO_SAMPLES - 1] = (data->time * ((long)data->value - gyro_temp.value) * 3 + 5170 + GYRO_COMP) / 10340 ; /* ger antal hundradelsgrader matematiskt avrundat */
		}
		else
		{
			/* negativ ändring */
			gyro_samples[NR_OF_GYRO_SAMPLES - 1] = (data->time * ((long)data->value - gyro_temp.value) * 3 - 5170 - GYRO_COMP) / 10340 ; /* ger antal hundradelsgrader matematiskt avrundat */
		}
	
		/* FIR-filtrering */
		for (i=0; i < NR_OF_GYRO_SAMPLES ; i++)
		{
			long int tmp;
			tmp = 1 * (long int)filter_coeff[i] * (long int)gyro_samples[NR_OF_GYRO_SAMPLES - 1 - i];
			
			if ( tmp >= 0)
			{
				tmp += 5000;
			}
			else
			{
				tmp -= 5000;
			}
							
			gyro_filtered += tmp * 1 / 10000; /* original tmp / 10000, infogat normeringsfaktor 3/5 */
		}
		
		sensor_data.angle += gyro_filtered;
		
		// Korrigera för eventuella överslag
		if (gyro_filtered >= 0)
		{
			if (sensor_data.angle >= 36000)
			{
				sensor_data.angle -= 36000;
			}
		} 
		else
		{
			if (sensor_data.angle >= 36000)
			{
				sensor_data.angle -= 29536;
			}
		}		
		data->is_converted = 1;
		gyro_filtered = 0;		
	}
	
}

int8_t calculateCenter(const uint8_t* data)
{
	// Beräknar offseten från mitt-tejpen

	int line_calc_ = 0;
	int sum_line_ = 0;
	
	// Lägger ihop alla linjesensor sum_line_ och tyngdpunkten line_calc_
	int i;
	for(i=0; i<11; i++)
	{
		if(data[i] >= sensor_parameters.tape_threshold)
		{
			sum_line_ = sum_line_ + data[i];
			line_calc_ = line_calc_ + (i - 5) * data[i];
		}

	}
	
	// gånger tio för att få med en decimal
	int8_t center_ = (line_calc_ * 10) / sum_line_;
	
	return center_;
}

uint8_t calculateAverage(const uint8_t* data)
{
	int sum_line_ = 0;
	
	uint8_t i;
	for(i=0;i<11;i++)
	{
		if(data[i] >= sensor_parameters.tape_threshold)
		{
			sum_line_ = sum_line_ + data[i];
		}		
	}
	
	uint8_t average = sum_line_/11;
	
	return average;
}

uint8_t getLineType(const uint8_t* data)
{
	// Returnerar vilken linjetyp som linjesenorn ser; vertikal, horizontell eller ingen linje

	static uint8_t previous_val = NO_LINE;
	static uint8_t count = 0;
	
	uint8_t num_detection = 0;
	
	uint8_t i;
	for (i = 0; i < 11; ++i)
	{
		// Hur många av dioderna är över tejp?
		if (data[i] > sensor_parameters.tape_threshold)
		{
			++num_detection;
		}
	}
	
	if (num_detection == 0)
	{
		previous_val = NO_LINE;
		return NO_LINE;
	}
	else if (num_detection < sensor_parameters.horizontal_line_threshold)
	{
		// Om föregående tejp var horisontell krävs en nedkylningsperiod för dioderna, horizontal_to_vertical_threshold
		// innan vertikal tejp kan detekteras
		if (previous_val == HORIZONTAL_LINE && count < sensor_parameters.horizontal_to_vertical_threshold)
		{
			++count;
			return HORIZONTAL_LINE;
		}
		else if (previous_val == NO_LINE)
		{
			count = 0;
			return NO_LINE;
		}
		
		if (previous_val == HORIZONTAL_LINE || previous_val == VERTICAL_LINE)
		{
			previous_val = VERTICAL_LINE;
			count = 0;
			return VERTICAL_LINE;
		}
		
		return NO_LINE;
	}
	else
	{
		previous_val = HORIZONTAL_LINE;
		return HORIZONTAL_LINE;
	}
}

void compareLines(uint8_t first, uint8_t second)
{	

	// Jämför antalet AD-omvandlingar som hinns med över den första respektive den andra tejpen.
	// Är dessa ungefär lika antas det vara två tunna linjer, annars en tunn och en tjock.
	uint8_t diff = abs(first - second);

	if (diff < sensor_parameters.line_diff_threshold)
	{
		sensor_data.line_type = LINE_STRAIGHT;
	}
	else if (first > second)
	{
		sensor_data.line_type = LINE_TURN_RIGHT;
	}
	else
	{
		sensor_data.line_type = LINE_TURN_LEFT;
	}
}

void convertLineData(RawLineData* data)
{
	if (data->is_converted)
	{
		return;
	}
	
	uint8_t line_type = getLineType(data->value);
	
	switch (line_type)
	{
		case HORIZONTAL_LINE:
		{
			if (current_line == 0)
			{
				// Första tejpen detekteras
				current_line = LINE_1;
				line_detections = 0;
			}
			else if (goal_mode && current_line == LINE_FOLLOWING)
			{
				// Andra tejpen detekteras efter vertikal/mål-tejp
				sensor_data.line_type = LINE_GOAL_STOP;
				current_line = 0;
				goal_mode = 0;
				line_detections = 0;
			}
			else if (current_line == SPACE_1)
			{
				// Andra tejpen detekteras efter mellanrum
				current_line = LINE_2;
			}
			else if (current_line == SPACE_2)
			{
				// Tredje tejpen detekteras efter mellanrum
				sensor_data.line_type = LINE_START_STOP;
				current_line = 0;
				line_detections = 0;
			}

			no_line_detections = 0;

			if (current_line == LINE_1 || current_line == LINE_2)
			{
				++line_detections;
			}
			
			break;
		}
		
		case VERTICAL_LINE:
		{
			if (current_line == LINE_1 || current_line == LINE_FOLLOWING)
			{
				if (current_line == LINE_1)
					sensor_data.line_deviation = 0;
				
				goal_mode = 1;
				current_line = LINE_FOLLOWING;
				sensor_data.line_type = LINE_GOAL;
				sensor_data.line_deviation = calculateCenter(data->value);
			}
			
			break;
		}
		
		case NO_LINE:
		{
			if (current_line == LINE_1)
			{
				first_line_val = line_detections;
				current_line = SPACE_1;
				line_detections = 0;
			}
			else if (current_line == LINE_2)
			{
				second_line_val = line_detections;
				current_line = SPACE_2;
				line_detections = 0;
				
				compareLines(first_line_val, second_line_val);
			}
			else
			{
				++no_line_detections;
				
				// Om ingen tejp detekteras under 'no_line_detection_threshold' AD-omvandlingar
				// sätts 'line_type' till ingen tejp
				if (no_line_detections > sensor_parameters.no_line_detection_threshold)
				{
					no_line_detections = 0;
					line_detections = 0;
					current_line = 0;
					sensor_data.line_type = LINE_NONE;
				}
			}
			
			break;
		}
	}
	
	data->is_converted = 1;
}


	
	

