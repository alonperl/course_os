#include "Block.hpp"
#include <time.h>

/**
 * @brief 
 * @param 
 * @return 
 */
Block::Block(int id, std::string dataHash, int dataLength, Block* father, int height)
{
	pthread_mutex_init(&blockMutex, NULL);

	_blockId = id;
	_height = height;
	_dataHash = dataHash;
	_dataLength = dataLength;
	_prevBlock = father;
}

Block::~Block()
{

}

/**
 * @return the block's Id
 */
int Block::getId(void)
{
	return _blockId;
}

/**
 * @return the block's height
 */
int Block::getHeight(void)
{
	return _height;
}

/**
 * @return the block's hashdata
 */
std::string Block::getHash(void)
{
	return _dataHash;
}

/**
 * Updates the block's hash
 */
void Block::setHash(char *hash)
{

}

int Block::getDataLength()
{
	return _dataLength;
}


/**
 * @return the blocks father
 */
Block* Block::getPrevBlock(void)
{
	return _prevBlock;
}
