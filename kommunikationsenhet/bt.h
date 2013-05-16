/*
 * FILNAMN:       bt.h
 * PROJEKT:       Mazeter
 * PROGRAMMERARE: Fredrik Stenmark
 *				  Joel Davidsson
 *				  Marcus Eriksson
 * 				  Mattias Fransson
 * DATUM:         2013-04-08
 *
 * BESKRIVNING:   I denna fil finns funktionalitet för 
 *				  blåtandskommunikation från kommunikationsenheten. 
 *          
 */

#ifndef BT_H_
#define BT_H_

#include <stdint.h>
#include "queue.h"

// Status för bluetooth-mottagning
#define BT_READY 1
#define BT_COMMAND_RECEIVED 2
#define BT_READING_MESSAGE 3

// Meddelandekön och dess hjälpvariabler
extern volatile Queue rx_queue;
extern volatile uint8_t bt_connected;
extern volatile uint8_t bt_status;
extern volatile uint8_t current_command;
extern volatile uint8_t data_count;
extern volatile Node* buffer_node;

// Bluetooth funktioner
void btInit();
void btReadData();
void btSendByte(uint8_t data);
void btSendData(uint8_t command, const uint8_t* buffer, uint8_t size);
void btSendNode(const Node* node);
void btSendString(const char* str);

#endif /* BT_H_ */
