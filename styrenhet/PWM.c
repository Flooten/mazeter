/*
 * FILNAMN:       PWM.c
 * PROJEKT:       Mazeter
 * PROGRAMMERARE: Herman Ekwall och Joel Davidsson
 * DATUM:         2013-04-04
 *
 * BESKRIVNING:
 *
 */

#include <avr/io.h>
#include "PWM.h"

void pwmInit()
{
	// Initierar utgångar
	DDRA = (1 << DDA0) | (1 << DDA1); // DIR setup
	DDRD = (1 << DDD4) | (1 << DDD5) | (1 << DDD6); // PWM till motorer och klo
		
	// Kontrollregister till hjulen
	TCCR1A = (1 << COM1A1) | (1 << COM1B1) | (1 << WGM11); // Väljer mode 14 i 14-5
	TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11); // Ställer in klockan och mode 14 och delar klockan 8
	
	// Periodtiden 0x4E1F = 19999 klockpulser dvs. 20 ms
	ICR1 = 0x4E1F; 
	
	// Står still
	OCR1A = MIN_PWM_WHEELS;
	OCR2B = MIN_PWM_WHEELS;
	
	// Kontrollregister till Klon
	TCCR2A = (1 << COM2B1) | (1 << WGM20) | (1 << WGM21); // Mode 7 i tabel 15-8
	TCCR2B = (1 << WGM22) | (1 << CS22) | (1 << CS21) | (1 << CS20); // Mode 7 i tabel 15-8
	
	// Periodtiden 0x9B = 155 klockpulser dvs. 20 ms
	OCR2A = 0x9B;  
	
	//Öppnar klon
	OCR2B = MIN_PWM_CLAW + 5;
}

void pwmWheels(ControlSignals ctrlsig)
{
	OCR1A = MIN_PWM_WHEELS + ctrlsig.left_value * INCREASE_PWM_WHEELS;
	OCR1B = MIN_PWM_WHEELS + ctrlsig.right_value * INCREASE_PWM_WHEELS;

	PORTA = (ctrlsig.right_direction << PORTA0) | (ctrlsig.left_direction << PORTA1);
}

void pwmClaw(ControlSignals ctrlsig)
{
	OCR2B = MIN_PWM_CLAW + ctrlsig.claw_value;
}
