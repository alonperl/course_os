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
#include <limits.h>
#include <sys/stat.h>


using namespace std;

//Variables
#define MIN_POSSIBLE_SOCKET 0
#define EMPTY_FILE 0
#define HEADER_LNEGTH 3
#define SUCCESS 0
#define ERROR -1
#define EXIT_CODE 1
#define MAX_PORT_NUM 65535
#define MIN_PORT_NUM 1
#define INIT_STRUCT 0
#define PORT_PARA_INDX 1
#define HOST_NAME_PARA_INDX 2
#define TRANSFER_FILE_NAME_PARA_INDX 3
#define DESIRED_FILE_NAME_IN_SERVER_PARA_INDX 4
#define CORRECT_ARGS_NUM 5
#define IS_DIRECTORY 0
#ifndef SYSCALL_ERROR
#define SYSCALL_ERROR(syscall) cerr << "Error: function: " << syscall << " errno: " << errno << "\n"; exit(1)
#endif

int gError = 0;

void error(string errorMessage)
{
	cerr << errorMessage;
	exit(EXIT_CODE);
}

bool checkArgs(int argc, char** argv)
{
	//Checks num of args is valid
	if (argc != CORRECT_ARGS_NUM)
	{
		return false;
	}

	//Checks port num is valid
	int port = atoi(argv[PORT_PARA_INDX]);
	if (port == 0 || port > MAX_PORT_NUM || port < MIN_PORT_NUM)
	{
		return false;
	}

	//check names are valid
	int transferFileNameSize = strlen(argv[TRANSFER_FILE_NAME_PARA_INDX]);
	int desiredNameSize = strlen(argv[DESIRED_FILE_NAME_IN_SERVER_PARA_INDX]);
	if (transferFileNameSize == 0 || desiredNameSize == 0 || 
		transferFileNameSize > PATH_MAX || desiredNameSize > PATH_MAX)
	{
		return false;
	}

	//Check host name is valid
	struct hostent *serverName = gethostbyname(argv[HOST_NAME_PARA_INDX]);
	if (serverName == NULL || strlen(serverName->h_name) > HOST_NAME_MAX)
	{
		return false;
	}

	//Check file exists or is Directory
	char* path = realpath(argv[TRANSFER_FILE_NAME_PARA_INDX], NULL);
	if (path == NULL)
	{
		free(path);
		return false;
	}

	/**
	 * File stat struct
	 */
	struct stat fileStatBuf = {INIT_STRUCT};
	stat(path, &fileStatBuf);
	if (S_ISDIR(fileStatBuf.st_mode))
	{
		free(path);
		return false;
	}
	free(path);

	//Check if we can open file
	string transferFileName = argv[TRANSFER_FILE_NAME_PARA_INDX];
	char* fileToTransfer = (char*)malloc(transferFileName.size()+1);
	if (fileToTransfer == NULL)
	{
		SYSCALL_ERROR("malloc");
	}
	memcpy (fileToTransfer, transferFileName.c_str(), transferFileName.size()+1);
	ifstream ifs(fileToTransfer, ios::in | ios::binary);
	if (ifs == NULL)
	{
		cout << "ERROR: can't open file.\n";
		free(fileToTransfer);
		return false;
	}
	free(fileToTransfer);
	ifs.close();

    return true;
}

unsigned long long getFileSize(ifstream &ifs) 
{
	long begin, end;
	begin = ifs.tellg();
	ifs.seekg(0, ios::end);
	end = ifs.tellg();
	ifs.seekg(ios::beg);
	if (begin == ERROR || end == ERROR)
	{
		gError = ERROR;
	}
	return end - begin;
}

void sendBuffer (char* buffer, int bufferSize, int serverSocket)
{
	int bytesSent = 0;
	int sent = 1;

	while (bytesSent < bufferSize)
	{
		sent = send(serverSocket, buffer + bytesSent, bufferSize - bytesSent, 0);
		if (sent == ERROR)
		{
			SYSCALL_ERROR("send");
		}
		bytesSent += sent;
	}
}

int main(int argc, char** argv){
//TODO free memory on errors
	if (!checkArgs(argc, argv))
	{
		error("Usage: clftp server-port server-hostname file-to-transfer filename-in-server");
	}

	cerr << "ARGS ARE FINE" << endl;
	
	//Initialize parameters:
	int port = atoi(argv[PORT_PARA_INDX]);
	struct hostent *server = gethostbyname(argv[HOST_NAME_PARA_INDX]);
	string fileNameInServer = argv[DESIRED_FILE_NAME_IN_SERVER_PARA_INDX]; //get name of file to be stored in server
	string transferFileName = argv[TRANSFER_FILE_NAME_PARA_INDX]; //get local file name
	int nameSize = fileNameInServer.length();
	cerr<< "nameSize is: "<< nameSize<<endl;
	char* fileToSave = (char*)malloc(fileNameInServer.size()+1);
	char* fileToTransfer = (char*)malloc(transferFileName.size()+1);

	if (fileToTransfer == NULL || fileToSave == NULL)
	{
		free(fileToSave);
		free(fileToTransfer);
		SYSCALL_ERROR("malloc");
	}

	memcpy (fileToTransfer, transferFileName.c_str(), transferFileName.size()+1);
	memcpy (fileToSave, fileNameInServer.c_str(), fileNameInServer.size()+1);
	//TODO in future maybe delete this no need to set string than char*

	cerr << "CREATING SOCKET" << endl;

	//Create a socket:
	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket < MIN_POSSIBLE_SOCKET)
	{
		free(fileToSave);
		free(fileToTransfer);
		SYSCALL_ERROR("socket");
	}
	struct sockaddr_in serverAddres;
	bzero((char *) &serverAddres, sizeof(serverAddres)); //puds with 0 for some reason?
	serverAddres.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&serverAddres.sin_addr.s_addr, server->h_length);
	serverAddres.sin_port = htons(port);

	cerr << "CREATED SOCKET" << endl;

	//Open file and check accessiblity
	ifstream ifs(fileToTransfer, ios::in | ios::binary);
	if (ifs == NULL)
	{
		free(fileToSave);
		free(fileToTransfer);
		close(serverSocket);
		SYSCALL_ERROR("open");
	}

	cerr << "CONNECT TO SOCKET" << endl;
	//Connect to server.
	if (connect(serverSocket,((struct sockaddr*)&serverAddres),sizeof(serverAddres)) < 0)
	{
		// cout << errno << endl;
		// cout << "serverSocket is: " << serverSocket << "\n";
		// cout << "server h_addr is: " << server->h_addr << endl;
		// cout << "server h_name is: " << server->h_name << endl;
		// cout << "serverAddres.sin_addr.s_addr is: " <<serverAddres.sin_addr.s_addr <<endl;
		free(fileToSave);
		free(fileToTransfer);
		ifs.close();
		SYSCALL_ERROR("connect");
	}

	cerr << "Recieved Details From Server" << endl;

	//Receive from Server Deatils
	char* serverDetailsBuffer = (char*)malloc(FIELD_LEN_DATASIZE + FIELD_LEN_DATA + sizeof(int));
	if (serverDetailsBuffer == NULL)
	{
		free(fileToSave);
		free(fileToTransfer);
		close(serverSocket);
		ifs.close();
		SYSCALL_ERROR("malloc");
	}

	int serverDetails = recv(serverSocket, serverDetailsBuffer, PACKET_SIZE, 0);
	if (serverDetails == ERROR)
	{
		free(fileToSave);
		free(fileToTransfer);
		free(serverDetailsBuffer);
		close(serverSocket);
		ifs.close();
		SYSCALL_ERROR("recv");
		//TODO or maybe keep waiting and now exit?
	}
	else if (serverDetails == 0)
	{
		free(fileToSave);
		free(fileToTransfer);
		free(serverDetailsBuffer);
		close(serverSocket);
		ifs.close();
		error ("ERROR: Server is Down.");
		//TODO what to print??
	}

	cerr << "Turn INFO From Server To Packet" << endl;

	// Initalize packet and check args
	Packet workPacket;
	workPacket.data = (char*) malloc(FIELD_LEN_DATA * sizeof(char)); //TODO could be shortened
	if (workPacket.data == NULL)
	{
		free(fileToSave);
		free(fileToTransfer);
		free(serverDetailsBuffer);
		close(serverSocket);
		ifs.close();
		SYSCALL_ERROR("malloc");
	}
	bytesToPacket(&workPacket, serverDetailsBuffer);

	if (workPacket.status != SERVER_RESPONSE)
	{
		free(workPacket.data);
		free(fileToSave);
		free(fileToTransfer);
		free(serverDetailsBuffer);
		close(serverSocket);
		ifs.close();
		error ("ERROR: Recieved Unknown Packet Type.");
		//TODO or maybe keep waiting and not exit?
	}

	//Check size of files server can recieve
	unsigned long long fileSize = getFileSize(ifs);
	if (fileSize < 0l || gError == ERROR)
	{
		free(workPacket.data);
		free(fileToSave);
		free(fileToTransfer);
		free(serverDetailsBuffer);
		close(serverSocket);
		ifs.close();
		SYSCALL_ERROR("tellg");
	}

	unsigned long long serverMaxSizeOfFile = 0l;
	memcpy(&serverMaxSizeOfFile, workPacket.data, workPacket.dataSize);

	if (serverMaxSizeOfFile < fileSize)
	{
		//Close connection and exit
		free(workPacket.data);
		free(fileToSave);
		free(fileToTransfer);
		free(serverDetailsBuffer);
		close(serverSocket);
		ifs.close();
		error ("Transmission failed: too big file");
	}

	cerr << "Send File size Packet" << endl;

	//Intialize first packet to send containning file size
	workPacket.dataSize = CLIENT_FILESIZE_DATASIZE;
	workPacket.status = CLIENT_FILESIZE;
	workPacket.data = (char*) realloc(workPacket.data, FIELD_LEN_DATA * sizeof(char));
	if (workPacket.data == NULL)
	{
		free(workPacket.data);
		free(fileToSave);
		free(fileToTransfer);
		free(serverDetailsBuffer);
		close(serverSocket);
		ifs.close();
		SYSCALL_ERROR("realloc");
	}

	memcpy(workPacket.data, &fileSize, CLIENT_FILESIZE_DATASIZE);
	//Send first packet
	char* buffer = (char*) malloc(sizeof(char) * PACKET_SIZE);
	if (buffer == NULL)
	{
		free(workPacket.data);
		free(fileToSave);
		free(fileToTransfer);
		free(serverDetailsBuffer);
		close(serverSocket);
		ifs.close();
		SYSCALL_ERROR("malloc");
	}

	packetToBytes(&workPacket, buffer);
	sendBuffer(buffer, CLIENT_FILESIZE_DATASIZE + HEADER_LNEGTH, serverSocket);

	cerr << "Send File name packet Packet" << endl;

	//Intialize second packet to send containning file name
	workPacket.dataSize = nameSize;
	workPacket.status = CLIENT_FILENAME;
	workPacket.data = (char*) realloc(workPacket.data, (nameSize) * sizeof(char));
	if (workPacket.data == NULL)
	{
		free(buffer);
		free(workPacket.data);
		free(fileToSave);
		free(fileToTransfer);
		free(serverDetailsBuffer);
		close(serverSocket);
		ifs.close();
		SYSCALL_ERROR("realloc");
	}

	cerr<<"workPacket.dataSize is: " << workPacket.dataSize << endl;
	memcpy(workPacket.data, fileNameInServer.c_str(), nameSize);
	//Send second packet
	packetToBytes(&workPacket, buffer);
	sendBuffer(buffer, nameSize + HEADER_LNEGTH, serverSocket);

	//Intialize packet to send containning data
	workPacket.status = CLIENT_DATA;
	if ((unsigned long long)fileSize >= FIELD_LEN_DATA)
	{
		workPacket.dataSize = FIELD_LEN_DATA;
	}

	cerr << "Send File with Packets" << endl;


	//Send all data using packets
	unsigned long long toSend = fileSize;
	// char* buffer = (char*) malloc(FIELD_LEN_DATA);
	// if (buffer == NULL)
	// {
	// 	error ("ERROR: malloc error.");
	// }

	char* dataBuffer = (char*) malloc(sizeof(char) * FIELD_LEN_DATA);
	if (dataBuffer == NULL)
	{
		free(buffer);
		free(workPacket.data);
		free(fileToSave);
		free(fileToTransfer);
		free(serverDetailsBuffer);
		close(serverSocket);
		ifs.close();
		SYSCALL_ERROR("malloc");
	}

	cerr<<"Strating to actually send file to Server"<<endl;
	while (toSend > PACKET_SIZE)
	{
		//free (workPacket.data); //Free allocated memory from before
		workPacket.data = (char*) realloc(workPacket.data ,FIELD_LEN_DATA * sizeof(char));
		if (workPacket.data == NULL)
		{
			free(workPacket.data);
			free(buffer);
			free(dataBuffer);
			free(fileToSave);
			free(fileToTransfer);
			free(serverDetailsBuffer);
			close(serverSocket);
			ifs.close();
			SYSCALL_ERROR("realloc");
		}
		ifs.read(dataBuffer, FIELD_LEN_DATA);
		memcpy(workPacket.data, dataBuffer, FIELD_LEN_DATA);
		packetToBytes(&workPacket, buffer);
		sendBuffer(buffer, PACKET_SIZE, serverSocket); //Send data packet
		toSend -= FIELD_LEN_DATA;
	}
	//In case there is still data with smaller size than max size of packet
	if (toSend != EMPTY_FILE) 
	{
		workPacket.data = (char*) realloc(workPacket.data ,toSend * sizeof(char));
		if (workPacket.data == NULL)
		{
			free(workPacket.data);
			free(buffer);
			free(dataBuffer);
			free(fileToSave);
			free(fileToTransfer);
			free(serverDetailsBuffer);
			close(serverSocket);
			ifs.close();
			SYSCALL_ERROR("realloc");
		}
		workPacket.dataSize = toSend;
		ifs.read(dataBuffer, toSend);
		memcpy(workPacket.data, dataBuffer, toSend);
		packetToBytes(&workPacket, buffer);
		sendBuffer(buffer, toSend + HEADER_LNEGTH, serverSocket);
	}

	//closing
	free(workPacket.data);
	free(buffer);
	free(dataBuffer);
	free(fileToSave);
	free(fileToTransfer);
	free(serverDetailsBuffer);
	close(serverSocket);
	ifs.close();

	return SUCCESS;
}
