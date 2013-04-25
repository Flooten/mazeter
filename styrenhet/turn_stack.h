/*
 * FILNAMN:       turn_stack.h
 * PROJEKT:       Mazeter
 * PROGRAMMERARE: Martin Andersson
 *				  Joel Davidsson
 * DATUM:         2013-04-25
 *
 * BESKRIVNING: Vi sparar alla svängar som ska tas när man kör ut.
 *
 */

#ifndef TURN_STACK_H_
#define TURN_STACK_H_

#include <stdlib.h>
#include <stdint.h>

typedef struct TurnNode
{
	uint8_t turn;
	struct TurnNode* next;
} TurnNode;

typedef struct TurnStack
{
	TurnNode* first;
} TurnStack;

/* Node */
TurnNode* newTurnNode(uint8_t turn);
void freeTurnNode(TurnNode* node);

/* Stack */
TurnStack createTurnStack();
void freeTurnStack(volatile TurnStack* stack);
void pushTurnStack(volatile TurnStack* stack, TurnNode* node);
uint8_t popTurnStack(volatile TurnStack* stack);
uint8_t empty(const volatile TurnStack* stack);

#endif /* TURN_STACK_H_ */