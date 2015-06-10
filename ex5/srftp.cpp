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

/* Definitions */
#define MAX_HOSTNAME 128
#define MAX_CONNECTIONS 5
#define MAX_PORT_NUM 65535
#define MIN_PORT_NUM 1
#define DECIMAL 10

#define USAGE "Usage: srftp server-port max-file-size\n"
#define SYSCALL_ERROR(syscall) "Error: function: " << syscall << "errno: " << errno << "\n"

#define SUCCESS 0
#define FAILURE -1

#define ARGS_NUM 3
#define ARGS_PORT 1
#define ARGS_MAX_FILE_SIZE 2

using namespace std;

struct args_struct
{
	int port;
	int maxFileSize;
};

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
	if (args->maxFileSize <= 0L || args->maxFileSize > PATH_MAX)
	{
		return FAILURE;
	}

	return SUCCESS;
}

int serverUp(struct args_struct args)
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
 	socketAddress.sin_port = htons(args.port);
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

void* clientHandler(void* maxFileSize)
{
	
}

int createClientThread(int clientSocket, struct sockaddr *clientSocketAddress, 
					   int maxFileSize)//, vector<pthread_t> *clients)
{
	pthread_t clientThread;
	
	if (pthread_create(&clientThread, NULL, clientHandler, maxFileSize))
	{
		cerr << SYSCALL_ERROR("pthread_create");
		return FAILURE;
	}

// TODO do we need to store all the pthread_ts?
	// clients.push_back(clientThread);

	return SUCCESS;
}

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
 	int welcomeSocket = serverUp(args);
 	if (welcomeSocket == FAILURE)
 	{
 		return FAILURE;
 	}

 	// Client threads collection
// 	vector<pthread_t> clients;

 	// Client socket data
 	int clientSocket;
	struct sockaddr clientSocketAddress;
	socklen_t sockaddr_len = sizeof(struct sockaddr_storage);
 	
 	// Wait for clients
 	while(true)
 	{
 		// Accept client
 		clientSocket = accept(welcomeSocket, &clientSocketAddress, &sockaddr_len);
 		createClientThread(clientSocket, &clientSocketAddress, args.maxFileSize);//, &clients);
 	}

 	return SUCCESS;
}