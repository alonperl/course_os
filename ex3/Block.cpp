#include "Block.hpp"
#include <time.h>

/**
 * @brief 
 * @param 
 * @return 
 */
Block::Block(int id, std::string dataHash, int dataLength, Block* father, int height)
{
	_blockId = id;
	_height = height;
	_dataHash = dataHash;
	_hashDataLength = dataLength;
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
 * @return the blocks father
 */
Block* Block::getPrevBlock(void)
{
	return _prevBlock;
}
