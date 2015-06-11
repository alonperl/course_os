/**
 * @file clftp.cpp
 * @author  griffonn ednussi
 * @version 1.0
 * @date 10 June 2015
 * 
 * @brief Simplified File Transfer Protocol client logic implementation
 *
 * @section LICENSE
 * This program is a free software. You can freely redistribute it.
 *
 * @section DESCRIPTION
 * The purpose of client in SimpleFTP is to be able to connect to 
 * server, exchange metadata with it and send files (in binary mode).
 * 
 * Usage: clftp server_port server_hostname file_to_transfer filename_in_server
 * where:
 *		server_port: the port of SimpleFTP Server application at destination.
 *		server_hostname: the host name of the destination server.
 *		file_to_transfer: the local file name to be transferred to server.
 *			Can be absolute or relative.
 *		filename_in_server: the desired filename to store the file on server at.
 */
 

//Includes
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <fstream>
#include <packets.h> 

using namespace std;

//Variables
#define SUCCESS 0
#define ERROR -1
#define EXIT_CODE 1
#define BUFFER_SIZE 1024

#define PORT_PARA_INDX 1
#define HOST_NAME_PARA_INDX 2
#define TRANSFER_FILE_NAME_PARA_INDX 3
#define DESIRED_FILE_NAME_IN_SERVER_PARA_INDX 4
#define CORRECT_ARGS_NUM 5


void error(string errorMessage)
{
	cerr << errorMessage;
	exit(EXIT_CODE);
}

int getFileSize(ifstream &ifs) 
{
	long begin, end;
	begin = ifs.tellg();
	ifs.seekg(0, ios::end);
	end = ifs.tellg();
	ifs.seekg(ios::beg);
	return end - begin;
}

void sendAllBuffer (char* buffer, int bufferSize, int serverSocket)
{
	
	int bytesSent = 0;
	int sent = 1;

	while (bytesSent < bufferSize)
	{
		sent = send(serverSocket, buffer + bytesSent, bufferSize - bytesSent, 0);
		if (sent == ERROR)
		{
			error ("ERROR: reciving data");
		}
		bytesSent += sent;
	}
}

void sendFileContent(ifstream& ifs, int fileSize)
{
	char* buffer = (char*) malloc(BUFFER_SIZE);
	if (buffer == NULL)
	{
		error ("ERROR: malloc error.");
	}
	int toSend = fileSize;

	while (toSend > BUFFER_SIZE)
	{
		ifs.read(buffer, BUFFER_SIZE);
		sendAllBuffer(buffer, BUFFER_SIZE);
		toSend -= BUFFER_SIZE;
	}
	if (toSend != 0)
	{
		ifs.read(buffer, toSend);
		sendAllBuffer(buffer, toSend);
	}
	
	free (buffer);
}

bool checkArgs(int argc, char** argv)
{
	if (argc != CORRECT_ARGS_NUM)
	{
		return false;
	}

    args->port = (int) strtol(argv[ARGS_PORT], NULL, DECIMAL);
    if (args->port == 0 || args->port > MAX_PORT_NUM || args->port < MIN_PORT_NUM)
    {
        return FAILURE;
    }

    args->maxFileSize = (int) strtol(argv[ARGS_MAX_FILE_SIZE], NULL, DECIMAL);
    if (args->maxFileSize == 0 || args->maxFileSize == LONG_MAX || args->maxFileSize == LONG_MIN)
    {
        return FAILURE;
    }

    return SUCCESS;

	//TODO check other param

}


//*********************************** MAIN ************************************


int main(int argc, char** argv){

	if (!checkArgs(argc, argv))
	{
		exit(1); //TODO maybe print error?
	}

	//Initialize parameters:
	int port = atoi(argv[PORT_PARA_INDX]);
	struct hostent *server = gethostbyname(argv[HOST_NAME_PARA_INDX]);
	string fileNameInServer = argv[DESIRED_FILE_NAME_IN_SERVER_PARA_INDX]; //get name of file to be stored in server
	string transferFileName = argv[TRANSFER_FILE_NAME_PARA_INDX]; //get local file name
	int fileInServerSize = argv[DESIRED_FILE_NAME_IN_SERVER_PARA_INDX].length();

	char* fileToSave = (char*)malloc(fileNameInServer.size()+1);
	char* fileToTransfer = (char*)malloc(transferFileName.size()+1);

	if (fileToTransfer == NULL || fileToSave == NULL)
	{
		error ("ERROR: malloc error.");
		exit(1); //TODO - only exit??
	}

	memcpy (fileToTransfer, transferFileName.c_str(), transferFileName.size()+1);
	memcpy (fileToSave, fileNameInServer.c_str(), fileNameInServer.size()+1);
	//TODO in future maybe delete this no need to set string than char*

	//create a socket:
	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in serverAddres;
	bzero((char *) &serverAddres, sizeof(serverAddres)); //puds with 0 for some reason?
	serverAddres.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&serverAddres.sin_addr.s_addr, server->h_length);

	// TODO check port is of right type: Description: uint16_t htons(uint16_t hostshort);
	serverAddres.sin_port = htons(port);

	//open file and get it's size:
	ifstream ifs(fileToTransfer, ios::in);
	if (ifs == NULL)
	{
		error("ERROR: open file");	
	}

	//connect to server.
	if (connect(serverSocket,((struct sockaddr*)&serverAddres),sizeof(serverAddres)) < 0)
	{
		error("ERROR connecting");
	}

	char* serverDetailsBuffer;
	//recieve data getting char* -> turn to packet
	int serverDetails = recv(serverSocket, serverDetailsBuffer, PACKET_SIZE, 0);

	if (serverDetails == FAILURE)
	{
		cerr << SYSCALL_ERROR("recv");
		exit(1);
		//TODO or maybe keep waiting and now exit?
	}
	else if (serverDetails == 0)
	{
		cerr << "Server is down";
		exit(1);
		//TODO or maybe keep waiting and now exit?
	}

	// Turn recieved details from server into packet and check args
	Packet *workPacket = initPacket();
	workPacket->bytesToPacket(serverDetailsBuffer);
	if (workPacket->status != SERVER_RESPONSE)
	{
		cerr << "Got wrong packet somehow";
		exit(1);
		//TODO or maybe keep waiting and now exit?
	}

	//Check if the size 
	 (unsigned int) workPacket->data


	int fileSize = getFileSize(ifs);

	//send information

	
	//intialize first packet
	workPacket->dataSize =
	workPacket->status = 
	workPacket->data =  



	sendAllBuffer ((char*) &fileSize, sizeof (int), serverSocket);
	sendAllBuffer((char*) &fileInServerSize , sizeof(int), serverSocket);
	sendAllBuffer(fileToSave, fileInServerSize + 1, serverSocket);
	
	sendFileContent(ifs, fileSize);

	//closing
	free (packet);
	free (fileToSave);
	free (fileToTransfer);
	close(serverSocket);
	ifs.close();

	return SUCCESS;
}

//******************************* END OF PROGRAM*******************************
