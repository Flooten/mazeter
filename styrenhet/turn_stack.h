/*
 * FILNAMN:       turn_stack.h
 * PROJEKT:       Mazeter
 * PROGRAMMERARE: Martin Andersson
 *				  Joel Davidsson
 * DATUM:         2013-04-25
 *
 * BESKRIVNING:   Vi puskar alla svängar som tas när man kör in,
 *                och popar på vägen ut.
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
	uint8_t new_node;
} TurnStack;

/* Node */
TurnNode* newTurnNode(uint8_t turn);
void freeTurnNode(TurnNode* node);

/* Stack */
volatile TurnStack createTurnStack();
void freeTurnStack(volatile TurnStack* stack);
void pushTurnStack(TurnStack* stack, TurnNode* node);
uint8_t popTurnStack(volatile TurnStack* stack);
uint8_t topTurnStack(const TurnStack* stack);
uint8_t empty(const volatile TurnStack* stack);
void clear(volatile TurnStack* stack);

#endif /* TURN_STACK_H_ */