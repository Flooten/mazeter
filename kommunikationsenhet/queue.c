/*
 * FILNAMN:       queue.c
 * PROJEKT:       Mazeter
 * PROGRAMMERARE: Fredrik Stenmark
 *				  Marcus Eriksson
 * 				  Mattias Fransson
 * DATUM:         2013-04-05
 *
 * BESKRIVNING:
 *          
 */ 

#include "queue.h"
#include <stdio.h>
#include <string.h>

/*
 *	Node
 */

/*
 *	Skapar en ny nod och allokerar utrymme size f�r ett meddelande.
 */
Node* newNode(uint8_t size)
{
	Node* new_node = (Node*)malloc(sizeof(Node));

	new_node->size = size;
	new_node->command = 0;
	new_node->data = (uint8_t*)malloc(size);
	memset(new_node->data, 0, size);
	new_node->next = NULL;

	return new_node;
}

/*
 *	�terl�mnar minne.
 */
void freeNode(Node* node)
{
	if (node == NULL)
		return;
	
	freeNode(node->next);
	free(node->data);
	free(node);
}

/*
 *	Queue
 */

/*
 *	Skapar och initialiserar en ny k�.
 */
Queue createQueue()
{
	Queue queue;

	queue.first = NULL;
	queue.last = NULL;

	return queue;
}

/*
 *	�terl�mnar minne.
 */
void freeQueue(Queue* queue)
{
	freeNode(queue->first);
	queue->first = NULL;
	queue->last = NULL;
}

/*
 *	Pushar node p� queue.
 */
void insert(Queue* queue, Node* node)
{
	if (empty(queue))
	{
		queue->first = node;
		queue->last = queue->first;
	}
	else
	{
		queue->last->next = node;
		queue->last = node;
	}
}

/*
 *	Tar bort det f�rsta elementet.
 */
void pop(Queue* queue)
{
	if (empty(queue) == 1)
		return;
	
	if (queue->first != queue->last)
	{
		// H�mta f�rsta noden
		Node* first_node = queue->first;
		
		// Koppla f�rbi noden
		queue->first = queue->first->next;
		
		// Avallokera minne
		freeNode(first_node);
	}
	else if (queue->first == queue->last)
	{		
		// Rensa k�n
		freeQueue(queue);
	}
}

/*
 *	Returnerar det f�rsta elementet.
 */
Node* top(const Queue* queue)
{
	return queue->first;
}

/*
 *	Kontrollerar om k�n �r tom.
 */
uint8_t empty(const Queue* queue)
{
	return (queue->first == NULL) ? 1 : 0;
}

