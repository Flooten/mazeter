/*
 * FILNAMN:       sensor_conversion.c
 * PROJEKT:       Mazeter
 * PROGRAMMERARE: Martin Andersson
 *                Herman Ekwall
 *                Mattias Fransson
 *
 * DATUM:         2013-04-17
 *
 * BESKRIVNING:
 *
 */

#include <stdint.h>
#include <stdlib.h>
#include "sensor_conversion.h"

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
	//convertRawDataGyro(gyro_sample1);
	sensor_data.angle=11;
	/* punktberäkning av linjesensor, kanske bara göra det i linjeföljande läge?  */
	
	convertRawData(distance1);
	convertRawData(distance2);
	convertRawData(distance3);
	convertRawData(distance4);
	
	convertRawData(distance5);
	convertRawData(distance6);
	convertRawData(distance7);
}

void convertRawData(RawData data)
{
	if (1 )//! data.is_converted)
	{
		switch (data.sensor_type)
		{
			case DISTANCE_1:
				sensor_data.distance1 = 01;
				break;
			case DISTANCE_2:
				sensor_data.distance2 = 02;
				break;
			case DISTANCE_3:
				sensor_data.distance3 = 03;
				break;
			case DISTANCE_4:
				sensor_data.distance4 = 04;
				break;
			case DISTANCE_7:
				//convertDistanceLong(data);
				sensor_data.distance7 = 07;
				break;
				
			case DISTANCE_5:
				sensor_data.distance5 = 05;
				break;
			case DISTANCE_6:
				//convertDistanceShort(data);
				sensor_data.distance6 = 06;
				break;
		
			default:
				conversion_status = CONVERSION_ERROR;
				break;
		}	
	} 
	else
	{
		// Datan är redan converterad
	}
	
}

void convertDistanceLong(RawData data)
{	
	if (data.value < 138)
	{
		if (data.value > 79) /* V [1.55,2.7] */
		{
		} 
		else if (data.value > 64) /* V [1.25,1.55] */
		{
		}
		else if (data.value > 46) /* V [0.9,1.25] */
		{
		}
		else if (data.value > 36) /* V [0.7,0.9] */
		{
		}
		else if (data.value > 20) /* V [0.4,0.7] */
		{
		}
		else
		{
			// underflow
		}
	} 
	else
	{
		// overflow
	}
		
}

void convertDistanceShort(RawData data)
{
	
}

void convertRawDataGyro(RawDataGyro data)
{	
		//* --- FIR filter, uint16_t angle ---------- */
	if (! data.is_converted)
	{
		uint8_t i;
		long int time_in_micros = ((long)data.time + 4) / 8; /* tid i mikrosekunder */
	
		for (i = 0; i < NR_OF_GYRO_SAMPLES-1; i++)
		{
			gyro_samples[i] = gyro_samples[i+1];
		}
	
		if (data.value >= GYRO_REF_LEVEL)
		{
			/* positiv ändring */
			gyro_samples[NR_OF_GYRO_SAMPLES -1] = (time_in_micros * ((long)data.value - GYRO_REF_LEVEL) * 3 + 5170) / 10340 ; /* ger antal hundradelsgrader matematiskt avrundat */
		}
		else
		{
			/* negativ ändring */
			gyro_samples[NR_OF_GYRO_SAMPLES -1] = (time_in_micros * ((long)data.value - GYRO_REF_LEVEL) * 3 - 5170) / 10340 ; /* ger antal hundradelsgrader matematiskt avrundat */
		}
	
		for (i=0; i < NR_OF_GYRO_SAMPLES ; i++)
		{
			long int tmp;
			tmp = 3 * (long int)filter_coeff[i] * (long int)gyro_samples[NR_OF_GYRO_SAMPLES - 1 - i];
			
			if ( tmp >= 0)
			{
				tmp += 25000;
			}
			else
			{
				tmp -= 25000;
			}
							
			gyro_filtered += tmp * 1 / 50000; /* original tmp / 10000, infogat normeringsfaktor 3/5 */
		}
		
		sensor_data.angle += gyro_filtered;
		
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
		data.is_converted = 1;
		gyro_filtered = 0;		
	}
	
}

int8_t calculateCenter(const uint8_t* data)
{
	int line_calc_ = 0;
	int sum_line_ = 0;
	
	// lägger ihop alla linjesensor sum_line_ och tyngdpunkten line_calc_
	int i;
	for(i=0;i<11;i++)
	{
		if(data[i] >= sensor_parameters.tape_threshold)
		{
			sum_line_ = sum_line_ + data[i];
			line_calc_ = line_calc_ + (i - 5) * data[i];
		}

	}
	
	// gånger tio för att få med en decimal
	int8_t center_ = (line_calc_ * 10)/sum_line_;
	
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
	static uint8_t previous_val = NO_LINE;
	static uint8_t count = 0;
	
	uint8_t num_detection = 0;
	
	uint8_t i;
	for (i = 0; i < 11; ++i)
	{
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
		
		previous_val = VERTICAL_LINE;
		count = 0;
		return VERTICAL_LINE;
	}
	else
	{
		previous_val = HORIZONTAL_LINE;
		return HORIZONTAL_LINE;
	}
}

void compareLines(uint8_t first, uint8_t second)
{
	uint8_t diff = abs(first - second);
	
	//! Helt godtyckligt
	if (diff < sensor_parameters.line_diff_threshold)
	{
		sensor_data.line_type = LINE_STRIGHT;
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
				sensor_data.line_type = LINE_STOP;
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
			if (current_line == LINE_1)
			{
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
				
				// Godtyckligt
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


	
	

