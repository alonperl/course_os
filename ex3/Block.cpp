#include <string.h>
#include <malloc.h>
#include "Block.hpp"

/**
 * @brief Block Constructor
 * @details [long description]
 * 
 * @param id [description]
 * @param hash [description]
 * @param height [description]
 * @param father [description]
 */
Block::Block(int id, char* hash, int height, Block* father)
{
	_pruneFlag = true;
	_blockId = id;
	_height = height;
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
int Block::getId()
{
	return _blockId;
}

/**
 * @return the block's height
 */
int Block::getHeight()
{
	return _height;
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
Block* Block::getPrevBlock()
{
	return _prevBlock;
}
