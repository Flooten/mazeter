/*
 * FILNAMN:       raw_data.h
 * PROJEKT:       Mazeter
 * PROGRAMMERARE: Herman Ekwall
 *                Mattias Fransson
 *                
 * DATUM:         2013-04-15
 *
 * BESKRIVNING:
 *
 */

#ifndef SENSOR_PARAMETERS_H
#define SENSOR_PARAMETERS_H

/*
 * tape_threshold:                   Värden över detta tolkas som tejp.
 * line_diff_threshold:              Tjockleksskillnad i antal AD-omvandlingar.
 * no_line_detection_threshold:      Hur länge vanligt golv kan detekteras innan allt nollställs.
 * horizontal_line_threshold:        Hur många linjesensorer måste ge utslag för att en horisontell linje ska detekteras.
 * horizontal_to_vertical_threshold: Hur många AD-omvandlingar det tar innan en vertikal linje verkligen kan urskiljas efter en horisontell.
 */
typedef struct 
{
	uint8_t tape_threshold;
	uint8_t line_diff_threshold;
	uint8_t no_line_detection_threshold;
	uint8_t horizontal_line_threshold;
	uint8_t horizontal_to_vertical_threshold;
} SensorParameters;

#endif /* SENSOR_PARAMETERS_H */