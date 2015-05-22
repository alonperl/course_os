/**
 * @file Request.cpp
 * @author  griffonn ednussi
 * @version 1.0
 * @date 18 May 2015
 * 
 * @brief An implementation of Request class.
 *
 * @section LICENSE
 * This program is a free software. You can freely redistribute it.
 *
 * @section DESCRIPTION
 * Request stores data of single new block that should be attached
 * to blockchain.
 */
#include <string.h>
#include <malloc.h>
#include "Request.hpp"

#define FAIL -1

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
				 dataLength(dataLength), 
				 blockNum(blockNum), 
				 father(newFather)
{
	if (newData == NULL)
	{ 
		// Cannot create block without data
		throw -1;
	}
	data = (char*)malloc(sizeof(char) * dataLength);
	if (data == NULL)
	{
		exit(FAIL);
	}
	strcpy(data, newData);
}

/**
 * @brief Request Destructor
 */
Request::~Request()
{
	if (data != NULL)
	{
		free(data);
	}
	data = NULL;
	father = NULL;
}
