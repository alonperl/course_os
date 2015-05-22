/**
 * @file Request.hpp
 * @author  griffonn ednussi
 * @version 1.0
 * @date 18 May 2015
 * 
 * @brief Description of Request class.
 *
 * @section LICENSE
 * This program is a free software. You can freely redistribute it.
 *
 * @section DESCRIPTION
 * Request stores data of single new block that should be attached
 * to blockchain.
 */
#ifndef _ADD_REQUEST_H
#define _ADD_REQUEST_H

#include <memory>
#include "Block.hpp"

/**
 * @brief Request class represents a new addition request to the chain,
 * holding data to store into the chain, in which block, and to which block
 * to attach it.
 */
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