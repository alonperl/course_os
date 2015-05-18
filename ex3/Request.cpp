#include <string.h>
#include <malloc.h>
#include "Request.hpp"

/**
 * @brief Request Constructor
 * 
 * @param newData Data to hash and store
 * @param dataLength
 * @param blockNum Unique number of the block
 * @param newFather Pointer to block's father
 */
Request::Request(const char* newData, const int dataLength, const int blockNum,
				 Block* newFather) : 
				 blockNum(blockNum), 
				 dataLength(dataLength), 
				 father(newFather)
{
	if (newData == NULL)
	{ 
		// Cannot create block without data
		throw -1;
	}
	data = (char*)malloc(sizeof(char) * dataLength);
	strcpy(data, newData);
}

/**
 * @brief Request Destructor
 */
Request::~Request()
{
	data = NULL;
	father = NULL;
}
