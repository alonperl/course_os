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
 * Create buffer with the packet's data.
 * Do not forget to free it.
 * 
 * @param packet: pointer to packet with data
 * @param buffer: pointer to destination buffer
 *
 * @return -1 if packet not allocated or malloc failed;
 *		   0 otherwise
 */
int packetToBytes(Packet* packet, char* buffer)
{
	if (packet == nullptr || buffer == nullptr)
	{
		return -1;
	}

	memcpy(buffer, &(packet->dataSize), FIELD_LEN_DATASIZE);
	memcpy(buffer + FIELD_LEN_DATASIZE, &(packet->status), FIELD_LEN_STATUS);
	memcpy(buffer + FIELD_LEN_DATASIZE + FIELD_LEN_STATUS, packet->data, packet->dataSize);
	
	// // Pad with zeros
	// memset(buffer + FIELD_LEN_DATASIZE + FIELD_LEN_STATUS + packet->dataSize, '\0', 
	// 	   FIELD_LEN_DATA - packet->dataSize);

	return 0;
}

/**
 * Create packet from buffer.
 * 
 * @param buffer: pointer to buffer with data
 * @param packet: pointer to destination packet
 *
 * @return -1 if packet not allocated; 0 otherwise
 */
int bytesToPacket(Packet* packet, char* buffer)
{
	if (buffer == nullptr || packet == nullptr || packet->data == nullptr)
	{
		return -1;
	}

	memcpy(&(packet->dataSize), buffer, FIELD_LEN_DATASIZE);
	memcpy(&(packet->status), buffer + FIELD_LEN_DATASIZE, FIELD_LEN_STATUS);
	memcpy(packet->data, buffer + FIELD_LEN_DATASIZE + FIELD_LEN_STATUS, packet->dataSize);

	return 0;
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
 * @param packet: pointer to packet with data
 * @param buffer: pointer to buffer for data
 * 
 * @return -1 if malloc failed, 0 otherwise
 */
int allocPacketData(Packet* packet, int dataSize)
{
	packet->data = (char*) realloc(packet->data, sizeof(char) * dataSize);
	if (packet->data == nullptr)
	{
		cerr << SYSCALL_ERROR("malloc");
		return -1;
	}

	return 0;
}
