/*
 * FILNAMN:       PWM.h
 * PROJEKT:       Mazeter
 * PROGRAMMERARE: Herman Ekwall och Joel Davidsson
 * DATUM:         2013-04-04
 *
 * BESKRIVNING:
 *
 */

#ifndef PWM_H_
#define PWM_H_

#include "controlsignals.h"

// Hjul: minsta värde 0 och högsta 19999 motsvarar 0 och 20 ms
#define MIN_PWM_WHEELS 0
#define INCREASE_PWM_WHEELS 197

// Klo: minsta värde 7 och högsta 12
#define MIN_PWM_CLAW 7

void pwmInit();
void pwmWheels(ControlSignals ctrlsig);
void pwmClaw(ControlSignals ctrlsig);

#endif /* PWM_H_ */