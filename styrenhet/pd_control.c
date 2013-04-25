﻿/*
 * FILNAMN:       styrenhet.c
 * PROJEKT:       Mazeter
 * PROGRAMMERARE: Mattias Fransson
 *				  Herman Ekwall
 * DATUM:         2013-04-18
 *
 */

#include "pd_control.h"
#include "control_parameters.h"
//#include "styrenhet.h"
#include <util/atomic.h>
#include <stdint.h>

typedef struct  
{
	int8_t left_value;
	int8_t right_value;
} RegulatorSignals;

//RegulatorSignals regulatorSignalDeltaLeft(const int16_t* delta_left, const int16_t* delta_left_previous)
//{
	//RegulatorSignals ret;
	//ret.left_value = control_parameters.left_kp * delta_left + control_parameters.left_kd * (delta_left - delta_left_previous));
	//ret.right_value = -(control_parameters.right_kp * delta_left + control_parameters.right_kd * (delta_left - delta_left_previous)));
	//return ret;
//}
//
//RegulatorSignals regulatorSignalDeltaRight(const int16_t* delta_right, const int16_t* delta_right_previous)
//{
	//RegulatorSignals ret;
	//ret.left_value = -(control_parameters.left_kp * delta_right + control_parameters.left_kd * (delta_right - delta_right_previous)));
	//ret.right_value = control_parameters.right_kp * delta_right + control_parameters.right_kd * (delta_right - delta_right_previous));
	//return ret;
//}
//
//RegulatorSignals regulatorSignalDeltaFront(const int16_t* delta_front, const int16_t* delta_front_previous)
//{
	//RegulatorSignals ret;
	//ret.left_value = -(control_parameters.left_kp * delta_front + control_parameters.left_kd * (delta_front - delta_front_previous)));
	//ret.right_value = control_parameters.right_kp * delta_front + control_parameters.right_kd * (delta_front - delta_front_previous)));
	//return ret;
//}
//
//RegulatorSignals regulatorSignalDeltaBack(const int16_t* delta_back, const int16_t* delta_back_previous)
//{
	//RegulatorSignals ret;
	//ret.left_value = control_parameters.left_kp * delta_back + control_parameters.left_kd * (delta_back - delta_back_previous));
	//ret.right_value = -(control_parameters.right_kp * delta_back + control_parameters.right_kd * (delta_back - delta_back_previous)));
	//return ret;
//}

void sensorDataToControlSignal(const SensorData* current, const SensorData* previous)
{
	//ATOMIC_BLOCK(ATOMIC_FORCEON)
	//{
		//RegulatorSignals regulator_signals;
		//
		//// switch
		//
		//regulator_signals = regulatorSignalDeltaFront(current->distance3 - current.distance4, previous.distance3 - previous.distance4);
		//
		//if (regulator_signals.left_value > control_signals.left_value)
		//{
			//control_signals.left_value = 0;
		//}
		//else
		//{
			//control_signals.left_value += regulator_signals.left_value;
			//if (control_signals.left_value > 100)
			//{
				//control_signals.left_value = 100;
			//}
		//}
		//
		//if (regulator_signals.right_value > control_signals.right_value)
		//{
			//control_signals.left_value = 0;
		//}
		//else
		//{
			//control_signals.right_value += regulator_signals.right_value;
			//if (control_signals.right_value > 100)
			//{
				//control_signals.right_value = 100;
			//}
		//}
	//}
}

void makeTurn(uint8_t turn)
{
	// Kör tills mitten av korsningen
	// while (angle_diff < 90)
	//   rotate(turn)
	// (finjustera med rätt avståndssensorer)
	// Kör framåt ut ur korsningen
	// Gå över i straightRegulation
}