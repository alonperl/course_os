#include "AddRequest.hpp"

AddRequest::AddRequest(const char* data, const int dataLength, const int blockNum, 
					   const Block* const father)
{
	blockNum = blockNum;
	dataLength = dataLength;
	father = newFather;
	strcpy(data, newData);
}

AddRequest::~AddRequest()
{
	blockNum = NULL;
	father = NULL;
}