#ifndef _ADD_REQUEST_H
#define _ADD_REQUEST_H

#include <memory>
#include "Block.hpp"

class Request
{
public:
	/**
	 * @brief Request Constructor
	 * 
	 * @param newData Data to hash and store
	 * @param dataLength
	 * @param blockNum Unique number of the block
	 * @param newFather Pointer to block's father
	 */
	Request(const char* newData, const int dataLength, const int blockNum,
			   Block* father);
	
	/**
	 * @brief Request Destructor
	 */
	~Request();

	/* Data for the new block */
	char* data;
	
	/* Data length */
	const int dataLength;
	
	/* New block ID */
	const int blockNum;

	/* Expected father. 
	   May be changed before actual block attachment. */
	Block* father;
};

#endif