#ifndef _PACKETS_H
#define _PACKETS_H

struct sftpPacket
{
	int messageLength;
	char status;
	int fileSize;
	char data[PATH_MAX];
};

int packetToBytes(struct sftpPacket *packet, char *buffer);
int bytesToPacket(struct sftpPacket *packet, char *buffer);

#define STATUS_CLIENT_REQUEST 0
#define STATUS_CLIENT_DATA 1

#define STATUS_SERVER_RESPONSE 2

#define MESSAGE_LENGTH_CLIENT_REQUEST PATH_MAX + sizeof(int) + sizeof(int) + sizeof(char)
#define MESSAGE_LENGTH_CLIENT_DATA PATH_MAX + sizeof(int) + sizeof(char)

#endif