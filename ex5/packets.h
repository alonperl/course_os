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

#define CLIENT_FILESIZE_DATASIZE sizeof(unsigned int)

/* Possible packet status */
enum Status:char 
{ 
	CLIENT_FILENAME,
	CLIENT_FILESIZE,
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
 * @param buffer: pointer to destination buffer
 *
 * @return -1 if packet not allocated or malloc failed;
 *		   0 otherwise
 */
int packetToBytes(Packet* packet, char* buffer);

/**
 * Create packet from buffer.
 * 
 * @param buffer: pointer to buffer for data
 * @param packet: pointer to destination packet
 *
 * @return -1 if packet not allocated; 0 otherwise
 */
int bytesToPacket(Packet* packet, char* buffer);

/**
 * Free memory allocated for packet
 */
void freePacket(Packet* packet);

/**
 * Allocate memory for Packet data
 *
 * @param packet: pointer to packet with data
 * @param buffer: pointer to buffer for data
 * 
 * @return -1 if malloc failed, 0 otherwise
 */
int allocPacketData(Packet* packet, int dataSize);

#endif