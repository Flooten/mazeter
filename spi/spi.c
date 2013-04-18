/*
 * FILNAMN:       spi.c
 * PROJEKT:       Mazeter
 * PROGRAMMERARE: Martin Andersson
 *                Mattias Fransson
 * DATUM:         2013-04-04
 *
 */

#include "spi.h"

#include <avr/io.h>

void spiMasterInit()
{
    // MOSI, SS_styr, SS_sensor och SCK som output
	DDRB = (1 << DDB5) | (1 << DDB4) | (1 << DDB3) | (1 << DDB7);
    
	// SPE = SPI enable, MSTR = master select, SPR0 = 1 -> sck = f_osc/16
	SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0);
}

void spiSlaveInit()
{
    // MISO output
	DDRB = (1 << DDB6);
    
	// SPE = SPI enable, SPIE = SPI enable interrupt
	SPCR = (1 << SPE) | (1 << SPIE);
}

void spiSelectSlave(uint8_t slave)
{
	// Sätt båda SS höga.
	PORTB |= 0x18;
    //PORTB |= 0b00011000;

	if (slave)
    	PORTB &= ~(1 << slave);
}

uint8_t spiSendByte(uint8_t data)
{
    SPDR = data;
	while (!(SPSR & (1 << SPIF)));
	return SPDR;
}

uint8_t spiSendCommand(uint8_t command, uint8_t to)
{
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
	//uint8_t response = spiSendCommand(command, to);
	
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
