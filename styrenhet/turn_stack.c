/*
 * FILNAMN:       turn_stack.c
 * PROJEKT:       Mazeter
 * PROGRAMMERARE: Martin Andersson
 *				  Joel Davidsson
 * DATUM:         2013-04-25
 *
 * BESKRIVNING: 
 *
 */


#include "turn_stack.h"
#include <stdio.h>
#include <string.h>


/*
 *	TurnNode
 */

/*
 *	Skapar en ny nod och allokerar utrymme size för ett meddelande.
 */
TurnNode* newTurnNode(uint8_t turn)
{
	TurnNode* new_node = (TurnNode*)malloc(sizeof(TurnNode));
	
	new_node->turn = turn;
	new_node->next = NULL;

	return new_node;
}

/*
 *	Återlämnar minne.
 */
void freeTurnNode(TurnNode* node)
{
	if (node == NULL)
		return;
	freeTurnNode(node->next);
	free(node);
}

/*
 *	Stack
 */

/*
 *	Skapar och initialiserar en ny kö.
 */
volatile TurnStack createTurnStack()
{
	TurnStack stack;

	stack.first = NULL;
	stack.new_node = 0;

	return stack;
}

/*
 *	Återlämnar minne.
 */
void freeTurnStack(volatile TurnStack* stack)
{
	freeTurnNode(stack->first);
	stack->first = NULL;
}

/*
 *	Pushar node på queue.
 */
void pushTurnStack(TurnStack* stack, TurnNode* node)
{
	if (empty(stack))
	{
		stack->first = node;
	}
	else
	{
		node->next = stack->first;
		stack->first = node;
	}
	
	stack->new_node = 1;
}

/*
 *	Returnerar det första elementet och tar bort det första elementet.
 */
uint8_t popTurnStack(volatile TurnStack* stack)
{
	if (empty(stack) == 1)
		return 0xEE;
		
	uint8_t turn = stack->first->turn;
	
	if (stack->first->next == NULL)
	{
		freeTurnStack(stack);
	}
	else
	{
		TurnNode* temp = stack->first;
		stack->first = stack->first->next;
		temp->next = NULL;
		freeTurnNode(temp);
	}
	
	return turn;
}

/*
 *	Returnerar det första elementet.
 */
uint8_t topTurnStack(const TurnStack* stack)
{
	if (empty(stack) == 1)
		return 0xEE;
		
	return stack->first->turn;
}

/*
 *	Kontrollerar om kön är tom.
 */
uint8_t empty(const volatile TurnStack* stack)
{
	return (stack->first == NULL) ? 1 : 0;
}

void clear(volatile TurnStack* stack)
{
	while(!empty(stack))
	{
		popTurnStack(stack);
	}
}