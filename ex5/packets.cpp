/**
 * @file packets.h
 * @author  griffonn ednussi
 * @version 1.0
 * @date 10 June 2015
 * 
 * @brief Simplified File Transfer Protocol packet functions implementation
 *
 * @section LICENSE
 * This program is a free software. You can freely redistribute it.
 *
 * @section DESCRIPTION
 * Implementation of functions to manipulate from struct packet.
 */

#include "packets.h"
#include <malloc.h>
#include <string.h>
#include <iostream>

#ifndef SYSCALL_ERROR
#define SYSCALL_ERROR(syscall) "Error: function: " << syscall << "errno: " << errno << "\n"
#endif

#define FAILURE -1;

using namespace std;

/**
 * Allocate memory for packet
 *
 * @return nullptr if malloc failed, pointer to allocated packet otherwise
 */
Packet* initPacket()
{
	Packet* packet;

	packet = (Packet*) malloc(sizeof(Packet));
	if (packet == nullptr)
	{
		cerr << SYSCALL_ERROR("malloc");
		return nullptr;
	}

	return packet;
}

/**
 * Create buffer with the packet's data.
 * Do not forget to free it.
 * 
 * @param packet: pointer to packet with data
 *
 * @return nullptr if packet not allocated or malloc failed;
 *		   pointer to buffer otherwise
 */
char* packetToBytes(Packet* packet)
{
	if (packet == nullptr)
	{
		return nullptr;
	}

	char* buffer = (char*) malloc(sizeof(short) + sizeof(char) * (1 + packet->dataSize));
	if (buffer == nullptr)
	{
		cerr << SYSCALL_ERROR("malloc");
		return nullptr;
	}

	memcpy(buffer, &(packet->dataSize), FIELD_LEN_DATASIZE);
	memcpy(buffer + FIELD_LEN_DATASIZE, &(packet->status), FIELD_LEN_STATUS);
	memcpy(buffer + FIELD_LEN_DATASIZE + FIELD_LEN_STATUS, packet->data, packet->dataSize);
	
	// // Pad with zeros
	// memset(buffer + FIELD_LEN_DATASIZE + FIELD_LEN_STATUS + packet->dataSize, '\0', 
	// 	   FIELD_LEN_DATA - packet->dataSize);

	return buffer;
}

/**
 * Create packet from buffer.
 * 
 * @param buffer: pointer to buffer for data
 *
 * @return nullptr if packet not allocated;
 *		   pointer to buffer otherwise
 */
Packet* bytesToPacket(char* buffer)
{
	if (buffer == nullptr)
	{
		return nullptr;
	}

	Packet* packet = (Packet*) malloc(sizeof(packet));
	if (packet == nullptr)
	{
		cerr << SYSCALL_ERROR("malloc");
		return nullptr;
	}
	
	memcpy(&(packet->dataSize), buffer, FIELD_LEN_DATASIZE);
	memcpy(&(packet->status), buffer + FIELD_LEN_DATASIZE, FIELD_LEN_STATUS);

	packet->data = (char*) malloc(sizeof(char) * packet->dataSize);
	if (packet->data == nullptr)
	{
		cerr << SYSCALL_ERROR("malloc");
		return nullptr;
	}
	
	memcpy(packet->data, buffer + FIELD_LEN_DATASIZE + FIELD_LEN_STATUS, packet->dataSize);

	return packet;
}

/**
 * Free memory allocated for packet
 */
void freePacket(Packet* packet)
{
	free(packet->data);
	free(packet);
	packet = nullptr;
}

/**
 * Allocate memory for Packet data
 *
 * @return nullptr if malloc failed, pointer to data block otherwise
 */
char* allocPacketData(int dataSize)
{
	char* pData = (char*) malloc(sizeof(char) * dataSize);
	if (pData == nullptr)
	{
		cerr << SYSCALL_ERROR("malloc");
		return nullptr;
	}

	return pData;
}
