/*
 * FILNAMN:       spi.c
 * PROJEKT:       Mazeter
 * PROGRAMMERARE: Martin Andersson
 *                Mattias Fransson
 * DATUM:         2013-05-16
 *
 */

#include "spi.h"

#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>

void spiMasterInit()
{
    // MOSI, SS_styr, SS_sensor och SCK som output
	DDRB = (1 << DDB5) | (1 << DDB4) | (1 << DDB3) | (1 << DDB7);
    
	// SPE = SPI enable, MSTR = master select, SPR0 = 1 -> sck = f_osc/16
	SPCR = (1 << SPE) | (1 << MSTR) | (0 << SPR1) | (1 << SPR0); /* spr1 = 0 i original */
}

void spiSlaveInit()
{
    // MISO output
	DDRB = (1 << DDB6);
    
	// SPE = SPI enable, SPIE = SPI enable interrupt
	SPCR = (1 << SPE) | (1 << SPIE);
}

// Aktiverar chip-select (SS) för angiven slav. Endast SENSOR_ENHET,
// STYR_ENHET och 0 är tillåtna värden.
void spiSelectSlave(uint8_t slave)
{
	// Sätt båda SS höga.
	PORTB |= 0x18;

	if (slave)
    	PORTB &= ~(1 << slave);
}

// Skickar en byte till den slav som senast aktiverades med spiSelectSlave.
uint8_t spiSendByte(uint8_t data)
{
    SPDR = data;
	while (!(SPSR & (1 << SPIF)));
	return SPDR;
}

// Skickar ett kommando och returnerar slavens svar.
uint8_t spiSendCommand(uint8_t command, uint8_t to)
{
    // För att slaven ska kunna svara måste SS-pinnen hos slaven sättas hög.
	spiSelectSlave(to);
	spiSendByte(command);
	spiSelectSlave(0);
	spiSelectSlave(to);
	uint8_t response = spiSendByte(0);
	spiSelectSlave(0);
	
	return response;
}

uint8_t spiSendData(uint8_t command, uint8_t to, const uint8_t* buffer, uint8_t size)
{
    // Pga. att slavens avbrottsrutin direkt börjar ta emot data kan första
    // byten skickas med när svaret läses.
	spiSelectSlave(to);
	spiSendByte(command);
	uint8_t response = spiSendByte(buffer[0]);
	
	int i;
	for (i = 1; i < size; ++i)
	{
		spiSendByte(buffer[i]);
	}
	spiSelectSlave(0);
	
	return response;
}

uint8_t spiReadData(uint8_t command, uint8_t from, uint8_t* buffer, uint8_t size)
{
	// Skickar ett kommando till slaven och läser av dess svarskod.
	uint8_t response = spiSendCommand(command, from);
	
	// Ta emot den begärda datan.
	int i;
	for (i = 0; i < size; ++i)
	{
		spiSelectSlave(from);
		buffer[i] = spiSendByte(0);
		spiSelectSlave(0);
	}
	
	return response;
}

// Experimentell kod. Fungerar förmodligen inte, använd ej.
char* spiReadString(uint8_t command, uint8_t from)
{
	char* buffer = NULL;
	uint8_t size = spiSendCommand(command, from);
	
	buffer = (char*)malloc(size);
	
	if (buffer == NULL)
		return buffer;
	
	uint8_t i;
	for (i = 0; i < size; ++i)
	{
		spiSelectSlave(from);
		buffer[i] = spiSendByte(0);
		spiSelectSlave(0);
	}
		
	return buffer;
}
