/*
 * FILNAMN:       bt.c
 * PROJEKT:       Mazeter
 * PROGRAMMERARE: Fredrik Stenmark
 *				  Joel Davidsson
 *				  Marcus Eriksson
 * 				  Mattias Fransson
 * DATUM:         2013-04-08
 *          
 */

#include "bt.h"
#include <avr/io.h>
#include <string.h>

volatile Queue rx_queue;
volatile uint8_t bt_status = BT_READY;
volatile uint8_t current_command = 0;
volatile uint8_t data_count = 0;
volatile Node* buffer_node = NULL;

void btInit()
{
	// Sätter upp BT enligt designspecifikationen
	
	// Baud-rate 9600 @ 2x
	UBRR0H = 0x00;
	UBRR0L = 0x67;

	// 8 databitar, 1 stoppbit, ingen paritet, etc.
	UCSR0A = 0x22;
	UCSR0B = 0x98; // 1001 1000
	UCSR0C = 0x06;
}

void btReadData()
{
	uint8_t received = UDR0;
	
	if (bt_status == BT_READY)
	{
		current_command = received;
		bt_status = BT_COMMAND_RECEIVED;
	}
	else if (bt_status == BT_COMMAND_RECEIVED)
	{
		if (received == 0)
		{
			buffer_node = (Node*)malloc(sizeof(Node));
			buffer_node->command = current_command;
			buffer_node->size = 0;
			buffer_node->data = 0;
			insert((Queue*)&rx_queue, (Node*)buffer_node);
			buffer_node = 0;
			bt_status = BT_READY;
			current_command = 0;
			return;
		}
		
		buffer_node = newNode(received);
		buffer_node->command = current_command;
		data_count = 0;
		bt_status = BT_READING_MESSAGE;
	}
	else if (bt_status == BT_READING_MESSAGE)
	{
		buffer_node->data[data_count++] = received;
		
		if (data_count == buffer_node->size)
		{
			insert((Queue*)&rx_queue, (Node*)buffer_node);
			buffer_node = NULL;
			bt_status = BT_READY;
			data_count = 0;
			current_command = 0;
		}
	}
}

void btSendByte(const uint8_t data)
{
	// Se till så att överföringsbuffern är tömd
	while (!(UCSR0A & (1 << UDRE0)));
	
	UDR0 = data;
}

void btSendData(uint8_t command, const uint8_t* buffer, uint8_t size)
{
	btSendByte(command);
	btSendByte(size);
	
	int i;
	for (i = 0; i < size; ++i)
	{
		btSendByte(buffer[i]);
	}
}

void btSendNode(const Node* node)
{
	btSendByte(node->command);
	btSendByte(node->size);
	
	int i;
	for (i = 0; i < node->size; ++i)
	{
		btSendByte(node->data[i]);
	}
}

void btSendString(const char* str)
{
	btSendByte(0x30); // Skickar en sträng
	
	int size = strlen(str);
	btSendByte(size);
	
	int i;
	for (i = 0; i < size; ++i)
	{
		btSendByte(str[i]);
	}
}