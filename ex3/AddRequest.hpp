#ifndef _ADD_REQUEST_H
#define _ADD_REQUEST_H

#include <memory>
#include "Block.hpp"

class AddRequest
{
public:
	AddRequest(const char* data, const int dataLength, const int blockNum, 
			   std::shared_ptr<Block>  father);
	~AddRequest();

	// Data for the new block
	const char* data;
	
	// Data length
	const int dataLength;
	
	// New block ID
	const int blockNum;

	// Expected father. 
	// May be changed before actual block attachment.
	std::shared_ptr<Block> father;
};

#endif