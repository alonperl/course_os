/**
* header the class for the client
*/

#ifndef _CLFTP_H
#define _CLFTP_H
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

using namespace std;

#define BUFF_SIZE 1024
#define SUCCESS 0
#define MIN_SERVER_PORT 1
#define MAX_SERVER_PORT 65535
#define FAIL -1
/**
* class client.
*/
class Client
{
public: 
	// need to be cons?
	Client();
	~Client();
	int _serverPort;
	int _socket;
	int _fileNameSize;
	int _fileSize;
	char* _fileToTransfer;
	char* _fileNameToSave;
	void sendData(ifstream &temp, int size , int socket);
	void sendBaffer(char* buff, int buffSize, int socket);
	int getSize(ifstream &temp);

private:

};

#endif