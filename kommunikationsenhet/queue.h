/*
 * FILNAMN:       queue.h
 * PROJEKT:       Mazeter
 * PROGRAMMERARE: Fredrik Stenmark
 *				  Marcus Eriksson
 * 				  Mattias Fransson
 * DATUM:         2013-04-05
 *
 * BESKRIVNING:   Definierar en FIFO-kö med tillhörande funktioner.
 *          
 */ 

#ifndef QUEUE_H
#define QUEUE_H

#include <stdlib.h>
#include <stdint.h>

typedef struct Node
{	
	uint8_t size;
	uint8_t command;
	uint8_t* data;
	struct Node* next;
} Node;

typedef struct Queue
{
	Node* first;
	Node* last;
} Queue;

/* Node */
Node* newNode(uint8_t size);
void freeNode(Node* node);

/* Queue */
Queue createQueue();
void freeQueue(Queue* queue);
//void insert(Queue* queue, uint8_t info);
void insert(Queue* queue, Node* node);
void pop(Queue* queue);
Node* top(const Queue* queue);
uint8_t empty(const Queue* queue);

#endif
