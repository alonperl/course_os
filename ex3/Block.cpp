#include "Block.hpp"

Block::Block(int id, char* hash, int height, int hashLength, Block* father)
{
	pthread_mutex_init(&blockMutex, NULL);

	_blockId = id;
	_height = height;
	_hash = hash;
	_hashLength = hashLength;
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
char* Block::getHash(void)
{
	return _hash;
}

/**
 * @return the block's hashdata length
 */
int Block::getHashLength()
{
	return _hashLength;
}

/**
 * @return the block's father
 */
Block* Block::getPrevBlock(void)
{
	return _prevBlock;
}
