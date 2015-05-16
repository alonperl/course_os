#include <string.h>
#include <malloc.h>
#include "Block.hpp"

Block::Block(int id, char* hash, int hashLength, int height, Block* father)
{
	pthread_mutex_init(&blockMutex, NULL);

	_pruneFlag = false;
	_blockId = id;
	_height = height;
	_hashLength = hashLength;
	_prevBlock = father;
	if (hash != NULL)
	{
		_hash = (char*)malloc(sizeof(char) * strlen(hash));
		strcpy(_hash, hash);
	}
}

Block::~Block()
{
	if (_hash != NULL)
	{
		free((void*)_hash);
	}
	_prevBlock = NULL;
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
 * @return true if the block needs to be pruned
 */
bool Block::getPruneFlag()
{
	return _pruneFlag;
}

/**
 * Sets the PruneFlag state
 */
void Block::setPruneFlag(bool newState)
{
	_pruneFlag = newState;
}

/**
 * @return the block's father
 */
Block* Block::getPrevBlock(void)
{
	return _prevBlock;
}
