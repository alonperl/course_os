#include <string.h>
#include <malloc.h>
#include "Request.hpp"

Request::Request(const char* newData, const int dataLength, const int blockNum,
					   Block* newFather) :
		blockNum(blockNum), dataLength(dataLength), father(newFather) {
	if (newData == NULL)
	{
		throw -1;
	}
	data = (char*)malloc(sizeof(char) * dataLength);
	strcpy(data, newData);
}

Request::~Request()
{
	free(data);
	data = NULL;
	father = NULL;
}