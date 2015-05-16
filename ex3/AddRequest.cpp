#include <string.h>
#include <malloc.h>
#include "AddRequest.hpp"

AddRequest::AddRequest(const char* newData, const int dataLength, const int blockNum,
					   Block* newFather) :
		blockNum(blockNum), dataLength(dataLength), father(newFather) {
	if (newData == NULL)
	{
		throw -1;
	}
	malloc(data, sizeof(char) * dataLength);
	strcpy(data, newData);
}

AddRequest::~AddRequest()
{
	free(data);
	data = NULL;
	father = NULL;
}