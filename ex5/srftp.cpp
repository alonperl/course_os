/**
 * @file srftp.cpp
 * @author  griffonn ednussi
 * @version 1.0
 * @date 10 June 2015
 * 
 * @brief Simplified File Transfer Protocol server logic implementation
 *
 * @section LICENSE
 * This program is a free software. You can freely redistribute it.
 *
 * @section DESCRIPTION
 * The purpose of the server in SimpleFTP is to wait for clients to connect
 * and to be able to exchange metadata with them and receive files (in binary
 * mode). All communication with client shall be done in separate thread.
 * 
 * Usage: srftp server_port maxFileSize
 * where:
 *		server_port: the port of SimpleFTP Server application to listen to.
 *		maxFileSize: maximum file size this server can accept.
 */

/* Includes */
#include <unistd.h>
#include <limits.h>
#include <pthread.h>
#include <errno.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <packets.h>

/* Definitions */
#define MAX_HOSTNAME 128
#define MAX_CONNECTIONS 5
#define MAX_PORT_NUM 65535
#define MIN_PORT_NUM 1
#define DECIMAL 10

#define USAGE "Usage: srftp server-port max-file-size\n"

#ifndef SYSCALL_ERROR
#define SYSCALL_ERROR(syscall) "Error: function: " << syscall << " errno: " << errno << endl
#endif

#define SUCCESS 0
#define FAILURE -1

#define CONNECTION_CLOSED 0

#define ARGS_NUM 3
#define ARGS_PORT 1
#define ARGS_MAX_FILE_SIZE 2

using namespace std;

/**
 * Server parameters: port and max filesize this server can accept.
 */
struct args_struct
{
	int port;
	unsigned long long maxFileSize;
};

/**
 * Client parameters: thread handler, socket handler, and max file size
 * that can be accepted from this client (in our case it is always as server's).
 */
struct client_data
{
	pthread_t clientThread;
	int clientSocket;
	unsigned long long maxFileSize;
};

/**
 * Validate and store parameters
 * 
 * @param argc: number of arguments
 * @param argv: array of arguments
 * @param args: pointer to struct where the params should be stored
 * 
 * @return -1 on error and USAGE is printed, 0 otherwise
 */
int validateArgs(int argc, char const *argv[], struct args_struct *args)
{
	if (argc != ARGS_NUM)
	{
		return FAILURE;
	}

	args->port = (int)strtol(argv[ARGS_PORT], NULL, DECIMAL);
	if (args->port == 0L || args->port > MAX_PORT_NUM || args->port < MIN_PORT_NUM)
	{
		return FAILURE;
	}

	args->maxFileSize = (int)strtol(argv[ARGS_MAX_FILE_SIZE], NULL, DECIMAL);
	if (args->maxFileSize < 0L || args->maxFileSize > ULLONG_MAX)
	{
		return FAILURE;
	}

	return SUCCESS;
}

/**
 * Power up server: create socket, bind it to a port, start listening to it.
 *
 * @param port: local port to bind to (1 - 65535)
 *
 * @return -1 if error occured and prints at what stage, 0 otherwise
 */
int serverUp(int port)
{
	// Get own hostname
 	char hostname[MAX_HOSTNAME+1];
 	struct hostent *host;
 	
 	gethostname(hostname, MAX_HOSTNAME);
 	host = gethostbyname(hostname);

 	if (host == NULL)
 	{
 		cerr << SYSCALL_ERROR("gethostbyname");
 		return FAILURE;
 	}

 	// Get own address
 	struct sockaddr_in socketAddress;
 	socketAddress.sin_family = host->h_addrtype;
 	socketAddress.sin_port = htons(port);
 	memcpy(&socketAddress.sin_addr, host->h_addr, host->h_length);

 	// Create welcome socket
 	int welcomeSocket;
 	if ((welcomeSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
 	{
 		cerr << SYSCALL_ERROR("socket");
 		return FAILURE;
 	}

 	// Bind welcome socket
 	if (bind(welcomeSocket, (struct sockaddr*) &socketAddress,
 			 sizeof(struct sockaddr_in)) < 0)
 	{
 		cerr << SYSCALL_ERROR("bind");
 		close(welcomeSocket);
 		return FAILURE;
 	}

 	// Wait for clients
 	listen(welcomeSocket, MAX_CONNECTIONS);

 	return welcomeSocket;
}


/**
 * Send welcome packet: Tell client what size of files server can accept
 *
 * @param pClient: client_data struct with client parameters
 *
 * @return nullptr
 */
int sendWelcomePacket(struct client_data* client)
{
	Packet welcomePacket;
	welcomePacket.status = SERVER_RESPONSE;
	welcomePacket.dataSize = sizeof(unsigned long long);
	welcomePacket.data = (char*) malloc(sizeof(char) * sizeof(unsigned long long));

	memcpy(welcomePacket.data, &(client->maxFileSize), sizeof(unsigned long long));
	if (welcomePacket.data == nullptr)
	{
		cerr << SYSCALL_ERROR("memcpy");
		return FAILURE;
	}

	char* buffer = (char*) malloc(sizeof(char) * WELCOME_PACKET_SIZE);
	if (buffer == nullptr)
	{
		cerr << SYSCALL_ERROR("malloc");
		return FAILURE;
	}

	packetToBytes(&welcomePacket, buffer);
	
	unsigned int dataSent = 0;
	int sent = FAILURE; // Default

	// Send welcome packet with size. Client should disconnect if its file exceeds
	while (dataSent < WELCOME_PACKET_SIZE)
	{
		sent = send(client->clientSocket, buffer + dataSent, PACKET_SIZE, 0);
		if (sent == FAILURE)
		{
			cerr << SYSCALL_ERROR("send");
			return FAILURE;
		}
	
		dataSent += sent;
	}

	free(buffer);
	free(welcomePacket.data);

	return SUCCESS;
}

/**
 * Client handle routine: Tell client what size of files server can accept,
 *		receive its file if sent and store on disk.
 * @param pClient: client_data struct with client parameters
 *
 * @return nullptr
 */
void* clientHandler(void* pClient)
{
	// Exchange vars
	bool nameReceived, sizeReceived; // Indicators
	
	struct client_data* client = (struct client_data*) pClient; // Client parameters

	unsigned int dataReceived, expectSize; // Net Counters
	int received; // Temp Net vars
	int currentPacketDataSize;
	
	Packet recvPacket;
	recvPacket.data = (char*) malloc(sizeof(char) * PACKET_SIZE);
	if (recvPacket.data == nullptr)
	{
		cerr << SYSCALL_ERROR("malloc");
		close(client->clientSocket);
		return nullptr;
	}

	char* filename = nullptr; // Filename holder
	unsigned int filesize;
	
	fstream outputStream; // File stream

	// Send welcome packet: tell client how much data server can accept
	if (sendWelcomePacket(client) == FAILURE)
	{
		pthread_exit(NULL); // Close client thread
	}

	// Prepare for receiving data
	dataReceived = 0;
	expectSize = PACKET_SIZE;

	char* buffer = (char*) malloc(sizeof(char) * PACKET_SIZE); // Maximal packet possible
	if (buffer == nullptr)
	{
		cerr << SYSCALL_ERROR("malloc");
		close(client->clientSocket);
		return nullptr;
	}

	// Read packet from socket and process it
	// If 0 bytes read => connection closed
	while ((received = recv(client->clientSocket, buffer, FIELD_LEN_DATASIZE, 0)) != 0)
	{
		if (received == FAILURE) // Error in receiving
		{
			cerr << SYSCALL_ERROR("recv");
			close(client->clientSocket);
			return nullptr;
		}

		dataReceived += received;

		// Get data length
		while (dataReceived < FIELD_LEN_DATASIZE)
		{
			received = recv(client->clientSocket, buffer + dataReceived, FIELD_LEN_DATASIZE - dataReceived, 0);
			if (received == FAILURE) // Error in receiving
			{ // TODO check errno
				cerr << SYSCALL_ERROR("recv");
				close(client->clientSocket);
				return nullptr;
			}

			dataReceived += received;
		}

		// Datasize field got, update expected size
		memcpy(&currentPacketDataSize, buffer, FIELD_LEN_DATASIZE);

		expectSize = FIELD_LEN_DATASIZE + FIELD_LEN_STATUS + currentPacketDataSize;

		// Get full packet
		while (dataReceived < expectSize)
		{
			received = recv(client->clientSocket, buffer + dataReceived, expectSize - dataReceived, 0);
			if (received == FAILURE) // Error in receiving
			{ // TODO check errno
				cerr << SYSCALL_ERROR("recv");
				close(client->clientSocket);
				return nullptr;
			}
			else if (received == 0) // client closed connection
			{
				break;
			}

			dataReceived += received;
		}

		// Convert buffer to packet
		recvPacket.data = (char*) realloc(recvPacket.data, sizeof(char) * PACKET_SIZE); //TODO smaller
		if (&recvPacket.data == nullptr)
		{
			cerr << SYSCALL_ERROR("realloc");
			close(client->clientSocket);
			return nullptr;
		}

		bytesToPacket(&recvPacket, buffer);

		if (recvPacket.status == CLIENT_FILENAME) // Filename packet type
		{
			if (nameReceived) // Filename has already been received
			{
				continue;
			}
			else // Save filename
			{
				filename = (char*) malloc(sizeof(char) * (recvPacket.dataSize + 1)); // For NULL byte
				if (filename == nullptr)
				{
					cerr << SYSCALL_ERROR("malloc");
					close(client->clientSocket);
					return nullptr;
				}
				
				memcpy(filename, recvPacket.data, recvPacket.dataSize);
				if (filename == nullptr)
				{
					cerr << SYSCALL_ERROR("memcpy");
					close(client->clientSocket);
					return nullptr;
				}

				filename[recvPacket.dataSize] = '\0';

				// Open file
				outputStream.open(filename, ofstream::out | ofstream::binary);
				if (!outputStream.good())				{
					cerr << SYSCALL_ERROR("open");
					close(client->clientSocket);
					return nullptr;
				}
				
				nameReceived = true;
			}
		}

		else if (recvPacket.status == CLIENT_FILESIZE) // Filesize packet type
		{
			if (sizeReceived) // Filesize has already been received
			{
				continue;
			}
			else // Save filesize
			{
				memcpy(&filesize, recvPacket.data, recvPacket.dataSize);

				if (filesize > client->maxFileSize)
				{
					cerr << "Transmission failed: too big file." << endl;
					close(client->clientSocket);
					return nullptr;
				}

				sizeReceived = true;
			}
		}

		else if (recvPacket.status == CLIENT_DATA)
		{
			if (nameReceived && sizeReceived && 
				outputStream.is_open() && outputStream.good()) // All metadata is here
			{	
				// Write chunk to file
				outputStream.write(recvPacket.data, recvPacket.dataSize);
				if (!outputStream.good())
				{
					cerr << SYSCALL_ERROR("write");
					close(client->clientSocket);
					return nullptr;
				}
				
				dataReceived += recvPacket.dataSize;
			}
		}

		dataReceived = 0;
		expectSize = PACKET_SIZE;
	}

	if (outputStream.is_open())
	{
		outputStream.close();
	}

	free(buffer);
	free(filename);
	free(recvPacket.data);
	free(pClient);

	sleep(8);

	return nullptr;
}

/**
 * Create and store client data, create separate pthread for this client.
 *
 * @param clientSocket: socket to connect to client through
 * @param clientSocketAddress: additional socket data
 * @param maxFileSize: maximal file size that can be accepted from this client
 *
 * @return -1 on error and prints at what stage, 0 otherwise
 */
int createClientThread(int clientSocket, struct sockaddr *clientSocketAddress, 
					   unsigned long long maxFileSize)
{
	
	struct client_data* client = (struct client_data*) malloc(sizeof(struct client_data));
	if (client == nullptr)
	{
		cerr << SYSCALL_ERROR("malloc");
		return FAILURE;
	}

	client->maxFileSize = maxFileSize;
	client->clientSocket = clientSocket;
	
	if (pthread_create(&(client->clientThread), NULL, clientHandler, client))
	{
		cerr << SYSCALL_ERROR("pthread_create");
		return FAILURE;
	}

// TODO do we need to store all the pthread_ts?
	// clients.push_back(clientThread);

	return SUCCESS;
}

/**
 * Main server routine: validate params, bind network connection, accept clients.
 *
 * @param argc: number of cli arguments
 * @param argv: array of cli arguments
 *
 * @return -1 on error, 0 otherwise (never actually happens)
 */
int main(int argc, char const *argv[])
{
	// Validate input
 	struct args_struct args;
 	if (validateArgs(argc, argv, &args))
 	{
 		cout << USAGE;
 		return FAILURE;
 	}

 	// Connect to the world
 	int welcomeSocket = serverUp(args.port);
 	if (welcomeSocket == FAILURE)
 	{
 		return FAILURE;
 	}

 	// Client socket data
 	int clientSocket;
	struct sockaddr clientSocketAddress;
	socklen_t sockaddr_len = sizeof(struct sockaddr_storage);
 	
 	// Wait for clients
 	while(true)
 	{
 		// Accept client
 		clientSocket = accept(welcomeSocket, &clientSocketAddress, &sockaddr_len);
 		if (clientSocket == FAILURE)
 		{
 			cerr << SYSCALL_ERROR("accept");
 			return FAILURE;
 		}

 		createClientThread(clientSocket, &clientSocketAddress, args.maxFileSize);//, &clients);
 	}

 	return SUCCESS;
}