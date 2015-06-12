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
#define SYSCALL_ERROR(syscall) "Error: function: " << syscall << " errno: " << errno << "\n"
#endif


bool checkArgs(int argc, char** argv)
{
	//Checks num of args is valid
	if (argc != CORRECT_ARGS_NUM)
	{
		cout << "num of args invalid\n";
		return false;
	}

	//Checks port num is valid
	int port = atoi(argv[PORT_PARA_INDX]);
	if (port == 0 || port > MAX_PORT_NUM || port < MIN_PORT_NUM)
	{
		cout << "bad port num\n";
		return false;
	}

	//check names are valid
	int transferFileNameSize = strlen(argv[TRANSFER_FILE_NAME_PARA_INDX]);
	int desiredNameSize = strlen(argv[DESIRED_FILE_NAME_IN_SERVER_PARA_INDX]);
	if (transferFileNameSize == 0 || desiredNameSize == 0 || 
		transferFileNameSize > PATH_MAX || desiredNameSize > PATH_MAX)
	{
		cout << "bad names\n";
		return false;
	}

	//Check host name is valid
	struct hostent *serverName = gethostbyname(argv[HOST_NAME_PARA_INDX]);
	if (serverName == NULL)
	{
		cout << "bad host name\n";
		return false;
	}

	//Check file exists or is Directory
	char* path = realpath(argv[TRANSFER_FILE_NAME_PARA_INDX], NULL);
	if (path == NULL)
	{
		cout << "doesn't exist\n";
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
		cout << "is directory \n";
		free(path);
		return false;
	}
	free(path);

	//Check if we can open file
	string transferFileName = argv[TRANSFER_FILE_NAME_PARA_INDX];
	char* fileToTransfer = (char*)malloc(transferFileName.size()+1);
	memcpy (fileToTransfer, transferFileName.c_str(), transferFileName.size()+1);
	ifstream ifs(fileToTransfer, ios::in);
	if (ifs == NULL)
	{
		cout << "can't open \n";

		free(fileToTransfer);
		return false;
	}
	free(fileToTransfer);
	ifs.close();

    return true;
}

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

void sendBuffer (char* buffer, int bufferSize, int serverSocket)
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

int main(int argc, char** argv){

	if (!checkArgs(argc, argv))
	{
		error("Usage: clftp server-port server-hostname file-to-transfer filename-in-server");
	}

	//Initialize parameters:
	int port = atoi(argv[PORT_PARA_INDX]);
	struct hostent *server = gethostbyname(argv[HOST_NAME_PARA_INDX]);
	string fileNameInServer = argv[DESIRED_FILE_NAME_IN_SERVER_PARA_INDX]; //get name of file to be stored in server
	string transferFileName = argv[TRANSFER_FILE_NAME_PARA_INDX]; //get local file name
	int nameSize = fileNameInServer.length();

	char* fileToSave = (char*)malloc(fileNameInServer.size()+1);
	char* fileToTransfer = (char*)malloc(transferFileName.size()+1);

	if (fileToTransfer == NULL || fileToSave == NULL)
	{
		error ("ERROR: malloc error.");
	}

	memcpy (fileToTransfer, transferFileName.c_str(), transferFileName.size()+1);
	memcpy (fileToSave, fileNameInServer.c_str(), fileNameInServer.size()+1);
	//TODO in future maybe delete this no need to set string than char*

	//Create a socket:
	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in serverAddres;
	bzero((char *) &serverAddres, sizeof(serverAddres)); //puds with 0 for some reason?
	serverAddres.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&serverAddres.sin_addr.s_addr, server->h_length);
	serverAddres.sin_port = htons(port);

	//Open file and check accessiblity
	ifstream ifs(fileToTransfer, ios::in);
	if (ifs == NULL)
	{
		error("ERROR: open file.");	
	}

	//Connect to server.
	if (connect(serverSocket,((struct sockaddr*)&serverAddres),sizeof(serverAddres)) < 0)
	{
		cout << errno << endl;
		cout << "serverSocket is: " << serverSocket << "\n";
		cout << "server h_addr is: " << server->h_addr << endl;
		cout << "server h_name is: " << server->h_name << endl;
		cout << "serverAddres.sin_addr.s_addr is: " <<serverAddres.sin_addr.s_addr <<endl;
		error("ERROR connecting.");
	}

	//Receive from Server Deatils
	char* serverDetailsBuffer = nullptr;
	int serverDetails = recv(serverSocket, serverDetailsBuffer, PACKET_SIZE, 0);
	if (serverDetails == ERROR)
	{
		cerr << SYSCALL_ERROR("recv");
		exit(1);
		//TODO or maybe keep waiting and now exit?
	}
	else if (serverDetails == 0)
	{
		error ("ERROR: Server is Down.");
		//TODO or maybe keep waiting and not exit?
	}

	// Initalize packet and check args
	Packet *workPacket = initPacket();
	workPacket = bytesToPacket(serverDetailsBuffer);

	if (workPacket->status != SERVER_RESPONSE)
	{
		error ("ERROR: Recieved Unknown Packet Type.");
		//TODO or maybe keep waiting and not exit?
	}

	//Check size of files server can recieve
	int fileSize = getFileSize(ifs);
	if (fileSize < 0 )
	{
		error("ERROR: Size of file is negative");
	}
	if (atoi(workPacket->data) <= fileSize)
	{
		//Close connection and exit
		close(serverSocket);
		error ("ERROR: Server Doesn't support files of desired Size.");
	}

	//Intialize first packet to send containning file size
	workPacket->dataSize = CLIENT_FILESIZE_DATASIZE;
	workPacket->status = CLIENT_FILESIZE;
	workPacket->data = allocPacketData(CLIENT_FILESIZE_DATASIZE);
	memcpy(workPacket->data, &fileSize, CLIENT_FILESIZE_DATASIZE);
	//Send first packet
	sendBuffer(packetToBytes(workPacket), PACKET_SIZE, serverSocket);

	//Intialize second packet to send containning file name
	workPacket->dataSize = nameSize;
	workPacket->status = CLIENT_FILENAME;
	free (workPacket->data); //Free allocated memory from before
	workPacket->data = allocPacketData(nameSize);
	memcpy(workPacket->data, &fileToTransfer, nameSize);
	//Send second packet
	sendBuffer(packetToBytes(workPacket), PACKET_SIZE, serverSocket);

	//Intialize packet to send containning data
	workPacket->status = CLIENT_DATA;
	if ((unsigned int)fileSize >= FIELD_LEN_DATA)
	{
		workPacket->dataSize = FIELD_LEN_DATA;
	}

	//Send all data using packets
	unsigned int toSend = fileSize;
	char* buffer = (char*) malloc(FIELD_LEN_DATA);
	if (buffer == NULL)
	{
		error ("ERROR: malloc error.");
	}

	while (toSend > PACKET_SIZE)
	{
		free (workPacket->data); //Free allocated memory from before
		workPacket->data = allocPacketData(FIELD_LEN_DATA);
		ifs.read(buffer, FIELD_LEN_DATA);
		memcpy(workPacket->data, buffer, FIELD_LEN_DATA);
		sendBuffer(packetToBytes(workPacket), PACKET_SIZE, serverSocket); //Send data packet
		toSend -= FIELD_LEN_DATA;
	}
	if (toSend != 0) //In case there is still data with smaller size than max size of packet
	{
		workPacket->dataSize = toSend;
		ifs.read(buffer, toSend);
		memcpy(workPacket->data, buffer, toSend);
		sendBuffer(packetToBytes(workPacket), PACKET_SIZE, serverSocket);
	}

	//closing
	free (workPacket->data);
	free (workPacket);
	free (buffer);
	free (fileToSave);
	free (fileToTransfer);
	close(serverSocket);
	ifs.close();

	return SUCCESS;
}
