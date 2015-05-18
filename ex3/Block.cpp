#include <string.h>
#include <malloc.h>
#include "Block.hpp"

/**
 * @brief Block Constructor
 * 
 * @param id - New block unique number
 * @param hash - Hashed data to shore
 * @param height - Block level in the Chain
 * @param father - Pointer to parent block
 */
Block::Block(int id, char* hash, int height, Block* father)
{
	_pruneFlag = true;
	_blockId = id;
	_height = height;
	_prevBlock = father;

	if (hash != NULL)
	{
		_hash = hash;
	}
}

/**
 * @brief Block Destructor
 */
Block::~Block()
{
	if (_hash != NULL)
	{
		free(_hash);
	}
	_prevBlock = NULL;
}

/**
 * @return block's Id
 */
int Block::getId()
{
	return _blockId;
}

/**
 * @return block's height
 */
int Block::getHeight()
{
	return _height;
}

/**
 * @return true if the block was marked to be pruned
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
 * @return pointer to block's father
 */
Block* Block::getPrevBlock()
{
	return _prevBlock;
}
