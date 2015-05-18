#ifndef _ADD_REQUEST_H
#define _ADD_REQUEST_H

#include <memory>
#include "Block.hpp"

class Request
{
public:
	Request(const char* newData, const int dataLength, const int blockNum,
			   Block* father);
	~Request();

	// Data for the new block
	char* data;
	
	// Data length
	const int dataLength;
	
	// New block ID
	const int blockNum;

	// Expected father. 
	// May be changed before actual block attachment.
	Block* father;
};

#endif