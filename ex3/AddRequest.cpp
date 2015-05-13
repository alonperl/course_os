#include <string.h>
#include "AddRequest.hpp"

AddRequest::AddRequest(const char* newData, const int dataLength, const int blockNum,
					   const void* const newFather) : blockNum(blockNum), dataLength(dataLength), father(newFather) {
	strcpy((char *) data, newData);
}

AddRequest::~AddRequest()
{
	// TODO: how to free data?
	data = NULL;
	father = NULL;
}