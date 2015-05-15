#include <string.h>
#include <malloc.h>
#include "AddRequest.hpp"

AddRequest::AddRequest(const char* newData, const int dataLength, const int blockNum,
					   const void* const newFather) :
		blockNum(blockNum), dataLength(dataLength), father(newFather) {
	if (newData == NULL)
	{
		throw -1;
	}
	strcpy((char *) data, newData);
}

AddRequest::~AddRequest()
{
	free((void*)data);
	data = NULL;
	father = NULL;
}