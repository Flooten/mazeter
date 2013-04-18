/*
 * FILNAMN:       spi.h
 * PROJEKT:       Mazeter
 * PROGRAMMERARE: Martin Andersson
 *                Mattias Fransson
 * DATUM:         2013-04-04
 *
 * BESKRIVNING:
 *
 */

#ifndef SPI_H
#define SPI_H

#include <stdint.h>
#include "spi_commands.h"

/* Slavenheter */
#define SENSOR_ENHET 3
#define STYR_ENHET 4

/* Slavstatus */
#define SPI_READY 1
#define SPI_RECEIVING_DATA 2

void spiMasterInit();
void spiSlaveInit();
void spiSelectSlave(uint8_t slave);
uint8_t spiSendByte(uint8_t data);
uint8_t spiSendCommand(uint8_t command, uint8_t to);
uint8_t spiSendData(uint8_t command, uint8_t to, const uint8_t* buffer, uint8_t size);
uint8_t spiReadData(uint8_t command, uint8_t from, uint8_t* buffer, uint8_t size);

#endif /* SPI_H */