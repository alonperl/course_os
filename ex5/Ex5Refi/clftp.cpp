/**
* implementation of the client class
*/

#include "clftp.h"

// structures for handling internet addresses , build-in
struct sockaddr_in serv_addr; 
// stracter for the host. build-in
struct hostent *server = NULL;

/**
* client constractor
*/
Client::Client()
{

}
/**
* client distractor
*/
Client::~Client()
{

}
/**
* a function to get the file size
*/
int Client::getSize(ifstream &temp)
{
	// Returns the position of the current character in the input stream.
	long start = temp.tellg();
	// sets position of the next character to be extracted from
	// the input stream. read from 0 to the end of the stream
	temp.seekg(0, ios::end);
	long end = temp.tellg();
	temp.seekg(0, ios::beg);
	long size = end - start;
	return size;

}

/** 
* a function to send the file buffer 
*/
void Client::sendBaffer(char* buff, int buffSize, int socket)
{
	int numByte = 0;
	int wasSent = 1; // start with false
	while (numByte < buffSize)
	{
		// send to socket the data starting from buffer+ numByte in length of buffSize- numByte
		//cout<<"Sending to server: "<<(buff + numByte)<<endl;
		cout<<"buff size is"<<buffSize - numByte<<endl;
		wasSent = send(socket, buff + numByte, buffSize - numByte, 0);
		// TODO - error incase wasSent < 0 , no need to end program
		numByte += wasSent;
	}
}

/**
* a function to send the file data to the server
*/
void Client::sendData(ifstream &temp, int size , int socket)
{
	char* buff;
	//TODO - error incase chould not provide space.
	int needToSend = size; 
	while (needToSend > BUFF_SIZE)
	{
		buff = (char*) malloc(BUFF_SIZE);
		temp.read(buff, BUFF_SIZE); // read into buffer 1024 byte from temp
		cerr<< bitset<BUFF_SIZE>(buff) <<endl;
		sendBaffer(buff , BUFF_SIZE, socket); 
		needToSend -= BUFF_SIZE;
		free(buff);
	}
	

	if (needToSend > 0) // in case something more to send
	{
		char* buffDelta = (char*) malloc(needToSend);
		bzero(buffDelta, needToSend);
		temp.read(buffDelta, needToSend); // read what was left
		cerr<< bitset<BUFF_SIZE>(buff) <<endl;
		cout<<"The delta is: "<<needToSend<<endl;
		sendBaffer(buffDelta , needToSend, socket); // send to socket
		cout<<"Sending to server: "<<(buffDelta)<<endl;
		free(buffDelta);
	}
	cout<<"Finish send data"<<endl;

}





/**
* PARAMETERS : server- port, server- hostname, file-to-transfer, filename-in-server
*/
int main(int argc, char** argv)
{
	//less then 5 param
	if (argc < 5)
	{
		cerr << "Usage: clftp server-port server-hostname file-to-transfer filename-in-server";
		exit(1);
	}
	Client* newClient = new Client(); // TODO - dont forget to free it.
	// get port of server
	newClient->_serverPort = atoi(argv[1]); 
	if (newClient->_serverPort < MIN_SERVER_PORT || newClient->_serverPort > MAX_SERVER_PORT)
	{
		cerr << "Usage: clftp server-port server-hostname file-to-transfer filename-in-server";
		exit(1);
	}

	// get host name of the server
	server = gethostbyname(argv[2]); // TODO  - incase of error, return h_errno!!!! - how to do that?

	// get the local file name that we want to transfer
	string newfile = argv[3]; // TODO - check error if empty or directory.
	newClient->_fileToTransfer = (char*)malloc(newfile.size()+1); // TODO - check error for malloc , also why plus 1 ?
	// copy name to the new space
	memcpy(newClient->_fileToTransfer, newfile.c_str(), newfile.size()+1);

	// get the name of the file we need to save
	newfile = argv[4];
	newClient->_fileNameSize = newfile.length(); // TODO - what is it needed for?
	
	newClient->_fileNameToSave = (char*)malloc(newfile.size()+1); // TODO - check error for malloc
	// copy name to the new space
	memcpy(newClient->_fileNameToSave, newfile.c_str(), newfile.size()+1);
	// create socket to pass informetion 
	newClient->_socket = socket(AF_INET, SOCK_STREAM, 0);
	// set all the serv_addr to 0
	bzero((char *) &serv_addr, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET; 
	// copy -h_length- byte sequence from server to serv_addr for the socket
	bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr, server->h_length);
	// converts from host byte order to network byte order.
	serv_addr.sin_port = htons(newClient->_serverPort);
	// open file to read from it
	ifstream temp (newClient->_fileToTransfer, ios::in | ios::binary); // TODO - error if canot open
	connect(newClient->_socket,((struct sockaddr*)&serv_addr),sizeof(serv_addr)); // TODO - if cannot conect to server?
	newClient->_fileSize = newClient->getSize(temp); // TODO - check if bigger then max_file_size? or server do that?

	// send informetion to server
	cout<<"Now sending size of file:"<< newClient->_fileSize<<endl;
	newClient->sendBaffer ((char*) &newClient->_fileSize, sizeof (unsigned int), newClient->_socket);
	cout<<"Finish sending size of file:"<<endl;
	//Can i send this file
	char* buffAnswer = (char*) malloc(2);

	if(recv(newClient->_socket, buffAnswer, 2, 0) == FAIL)
	{
		//TODO error
	}
	else if(buffAnswer[0] == '0')
	{
		//TODO error// for file too big: "Transmission failed: too big file", free , and return 0;
	}

	
	cout<<"Now size of path:"<<endl;
	newClient->sendBaffer((char*) &newClient->_fileNameSize , sizeof(unsigned int), newClient->_socket);
	cout<<"Finish size of path:"<<endl;
	cout<<"Now path:"<<endl;
	//int wasSent = send(newClient->_socket, newClient->_fileNameToSave, 1024, 0);
	newClient->sendBaffer(newClient->_fileNameToSave, newClient->_fileNameSize, newClient->_socket);
	cout<<"Now finish path:"<<endl;
	cout<<"Now start data:"<<endl;
	newClient->sendData(temp, newClient->_fileSize, newClient->_socket);
	cout<<"Finish data:"<<endl;

	//close
	free(buffAnswer);
	free(newClient->_fileNameToSave);
	free(newClient->_fileToTransfer);
	delete(newClient);
	close(newClient->_socket);
	temp.close();


	return SUCCESS;

}





