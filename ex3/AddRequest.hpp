#ifndef _ADD_REQUEST_H
#define _ADD_REQUEST_H

class AddRequest
{
public:
	AddRequest(const char* data, const int dataLength, const int blockNum, 
			   const Block* const father);
	~AddRequest();

	// Data for the new block
	const char* data;
	
	// Data length
	const int dataLength;
	
	// New block ID
	const int blockNum;

	// Expected father. 
	// May be changed before actual block attachment.
	const Block* const father;
};

#endif