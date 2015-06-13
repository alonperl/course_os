#include "srftp.h"






static Server* myServer;

Server::Server(int port, int maxSize)
{
	_srvPort = port;
	_maxFileSize = maxSize;
}

Server::~Server()
{

}


int Server::uploadServer()
{
 	
 	gethostname(srvHostName, MAX_HOSTNAME);
 	srvHost = gethostbyname(srvHostName);

 	if (srvHost == NULL)
 	{
 		// cerr << SYSCALL_ERROR("gethostbyname");//TODO error
 		// return FAILURE;
 	}

 	// Get own address
 	
 	srvSocketAddress.sin_family = srvHost->h_addrtype;
 	srvSocketAddress.sin_port = htons(_srvPort);
 	memcpy(&srvSocketAddress.sin_addr, srvHost->h_addr, srvHost->h_length);

 	// Create welcome socket
 	
 	if ((_srvSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
 	{
 		cerr<<SYSTEM_ERROR("pthread_create");//TODO error messagess
 		return FAIL;
 	}


 	if (bind(_srvSocket, (struct sockaddr*) &srvSocketAddress,
 			 sizeof(struct sockaddr_in)) < 0)
 	{
 		//cerr << SYSCALL_ERROR("bind")TODO error message;
 		close(_srvSocket);
 		return FAIL;
 	}


 	listen(_srvSocket, LIMIT_CONNECTIONS);

 	return _srvSocket;
}







int validateParameters(int argc, char const *argv[])
{
    if (argc != 3 || atoi(argv[1]) < MIN_PORT_NUM || atoi(argv[1]) > MAX_PORT_NUM || argv[2] < 0)
    {
        return FAIL;
    }

    return SUCCESS;       
   
}


bool isFileEx(const char *fileName)
{
    std::ifstream infile(fileName);
    return infile.good();
}


int writeToFile(char* readData, char* fileName, int fileSize)
{
	//cout<<"in write to file"<<endl;
	fstream outputStream;
	//cout<<"File name: "<<fileName<<endl;
	//cout<<"File size"<<fileSize<<endl;
	//cout<<"Read Data : "<<readData<<endl;
	outputStream.open(fileName, ofstream::out | ofstream::binary);

	if (!outputStream.good())
	{
		//cout<<"error"<<errno<<endl;
	// File already open TODO WHAT TO DO?
	}
	outputStream.write(readData, fileSize);
	outputStream.close();

	//cout<<"finish writing"<<endl;
	return SUCCESS;

}

int readDataClient(char* readData, int fileSize, int clientSocket)
{
	char* readBuff = NULL; 
	int readByte = 0;
	int realRead = 0;
	int endWhileOffset = fileSize / BUFF_SIZE;
	cout<<"filesize "<<fileSize<<endl;

	cout<<"offset is "<<endWhileOffset<<endl;

	while(readByte < endWhileOffset*BUFF_SIZE)
	{
		readBuff = (char*) malloc(BUFF_SIZE);
		if((realRead = recv(clientSocket, readBuff, BUFF_SIZE, 0)) == FAIL)
		{
			//TODO error
		}
		//cout<<"in while readBuff "<<readBuff<<endl;
		strncpy(readData + readByte , readBuff , realRead);
		readByte += realRead;
		fileSize -= realRead;
		free(readBuff);
	}

	cout<<"need to read more"<<fileSize<<endl;
	while(fileSize > 0)
	{
		char* readEndBuff = (char*) malloc(fileSize);
		if((realRead = recv(clientSocket, readEndBuff, fileSize, 0)) == FAIL)
		{
			//TODO error
		}
		//cout<<"in if readBuff endddddddddddddddd "<<readEndBuff[fileSize]<<endl;
		cout<<"in if readBuff "<<readEndBuff<<endl;
		strncpy(readData + readByte, readEndBuff, realRead);
		readByte += realRead;
		fileSize -= realRead;
		free(readEndBuff);
	}
	cout<<"Data inside function: "<<readData<<endl;
	return readByte;
}


void* connectionHandler(void* clientSocketHandler)
{

	unsigned int fileSize;
	unsigned int maxFileSize = myServer->_maxFileSize;

	int clientSocket = *((int*)(clientSocketHandler));
	//Receving file size
	char* buffFileSize = (char*) malloc(sizeof(unsigned int));//TODO error

	if(recv(clientSocket, buffFileSize, sizeof(unsigned int), 0) == FAIL)//TODO maybe buff size
	{
		//TODO
	}

	cout<<"After file size the size is: "<<(*((unsigned int*)(buffFileSize)))<<endl;
	fileSize = (*((unsigned int*)(buffFileSize)));

	if((*((unsigned int*)(buffFileSize))) > maxFileSize)
	{
		cout<<"too big file omg"<<endl;
		free(buffFileSize);
		char* buffAnswer = (char*) malloc(2);
		buffAnswer[0] = '0';//TODO magic
		if(send(clientSocket, buffAnswer, 2, 0))
		{
			free(buffAnswer);
			//TODO error
		}
		//Close the thread
	}
	
	//Its okay to work with this kind of size
	free(buffFileSize);
	char* buffAnswer = (char*) malloc(2);
	buffAnswer[0] = '1';//TODO magic 
	cout<<"Sending aprooval to client "<<endl;
	if(send(clientSocket, buffAnswer, 2, 0) == FAIL)//TODO magic
	{
		//TODO error
	}
		
	//Size of file path

	free(buffAnswer);
	char* buffFilePathSize = (char*) malloc(sizeof(unsigned int));
	cout<<"Wating for file size path "<<endl;
	if(recv(clientSocket, buffFilePathSize, sizeof(unsigned int), 0 ) == FAIL)
	{
		//TODO error
	}

	unsigned int filePathSize = (*((unsigned int*)(buffFilePathSize)));
	cout<<"The file path size is "<<(*((unsigned int*)(buffFilePathSize)))<<endl;


	free(buffFilePathSize);
	char* fileName = (char*) malloc(filePathSize);
	char* buffPath = (char*) malloc(filePathSize);

	cout<<"Waiting for file path it self "<<endl;
	if(recv(clientSocket, buffPath, filePathSize, 0 ) == FAIL)
	{
		//TODO error
	}
	cout<<"The file path is :"<<buffPath<<endl;
	//strncpy(fileName , buff , BUFF_SIZE);
	memcpy(fileName , buffPath, filePathSize);

	free(buffPath);


	//Ready for data
	cout<<"File name before read :"<<fileName<<endl;
	
	char* readData = (char*) malloc(fileSize);
	int readDataResult = readDataClient(readData , fileSize, clientSocket);

	cout<<"File name is after read :"<<fileName<<endl;

	//cout<<"Data: "<<readData<<endl;
	

	int writeFileResult = writeToFile(readData, fileName, fileSize);
	cout<<"finish"<<endl;
	free(readData);
	close(clientSocket);//Close connection TODO error




}



int createThread(size_t clientSocket)
{
	pthread_t socketThread;
	cout<<"creating a thread for socket: "<<clientSocket<<endl;
	if(pthread_create(&socketThread, NULL, connectionHandler, &clientSocket))
	{
		cerr<<SYSTEM_ERROR("pthread_create");
	}
	return 0 ;// TODO 

}



int main(int argc ,char const *argv[])
{
	if(validateParameters(argc , argv) !=  SUCCESS) 
	{
		cout << USEAGE_ERROR <<endl;
		return FAIL;
	}


	myServer = new Server(atoi(argv[1]),atoi(argv[2]));

	myServer->uploadServer(); // TODO - check error

	int clientSocket;
	struct sockaddr clientSocketAddress;
	socklen_t sockAddrLen = sizeof(struct sockaddr_storage);
	cout<<"The server is up: "<<myServer->srvHostName<<endl;

	while((clientSocket = accept(myServer->_srvSocket,&clientSocketAddress,&sockAddrLen)) != FAIL)
	{
		cout<<"inside the while"<<endl;
		createThread(clientSocket);
	}

}





// //	int clientSocket;
// struct sockaddr clientSocketAddress;
// socklen_t sockaddr_len = sizeof(struct sockaddr_storage);
// // Wait for clients
// while(true)
// {
// // Accept client
// clientSocket = accept(welcomeSocket, &clientSocketAddress, &sockaddr_len);
// createClientThread(clientSocket, &clientSocketAddress, args.maxFileSize);//, &clients);
// }
