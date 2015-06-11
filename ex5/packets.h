/**
 * @file packets.h
 * @author  griffonn ednussi
 * @version 1.0
 * @date 10 June 2015
 * 
 * @brief Simplified File Transfer Protocol packet definition
 *
 * @section LICENSE
 * This program is a free software. You can freely redistribute it.
 *
 * @section DESCRIPTION
 * Packets in SFTP are simple structs with three fields:
 *	- int dataSize: how much usable data this packet holds
 *	- (char) status: what type of packet is it
 *	- char[] data: actual data
 */

#ifndef _PACKETS_H
#define _PACKETS_H

#define PACKET_SIZE sizeof(char) * 1024

#define FIELD_LEN_DATASIZE sizeof(short)
#define FIELD_LEN_STATUS sizeof(char)
#define FIELD_LEN_DATA PACKET_SIZE - FIELD_LEN_DATASIZE - FIELD_LEN_STATUS

/* Possible packet status */
enum Status:char { 
	CLIENT_REQUEST, 
	CLIENT_DATA, 
	SERVER_RESPONSE
}; 

/**
 * Packet struct
 */
typedef struct packet
{
	short dataSize; 			// Length of actual data in data field 
	Status status; 				// Packet status
	char* data;					// Actual data
} Packet;

/**
 * Allocate memory for packet
 */
Packet* initPacket();

/**
 * Create buffer with the packet's data.
 * Do not forget to free it.
 * 
 * @param packet: pointer to packet with data
 *
 * @return nullptr if packet not allocated or malloc failed;
 *		   pointer to buffer otherwise
 */
char* packetToBytes(Packet* packet);

/**
 * Create packet from buffer.
 * 
 * @param buffer: pointer to buffer for data
 *
 * @return nullptr if packet not allocated;
 *		   pointer to buffer otherwise
 */
Packet* bytesToPacket(char* buffer);


/**
 * Free memory allocated for packet
 */
void freePacket(Packet* packet);


#endif