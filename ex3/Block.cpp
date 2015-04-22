#include "Block.hpp"
#include <time.h>

/**
 * @brief 
 * @param 
 * @return 
 */
Block::Block(int id, string dataHash, Block* father, int height)
{
	_blockId = id;
	_height = height;
	_hashData = dataHash;
	_prevBlock = father;
	_timestamp = time();
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
 * @return the block's timestamp
 */
time_t Block::getTimestamp(void)
{
	return _timestamp;
}

/**
 * @return the block's height
 */
int Block::getHeight(void)
{
	return _height;
}

/**
 * @return the block's hash data
 */
string Block::getHashData(void)
{
	return _hashData;
}

/**
 * @return the blocks father
 */
Block* Block::getPrevBlock(void)
{
	return _prevBlock;
}
