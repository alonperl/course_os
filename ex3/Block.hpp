#ifndef _BLOCK_H
#define _BLOCK_H

#include <memory>
#include <pthread.h>

class Block
{
public:
	/**
	 * Block constructur
	 */
	Block(int id, char* hash, int height, Block* father);
	~Block();

	/**
	 * @return block's Id
	 */
	int getId();

	/**
	 * @return block's height
	 */
	int getHeight();

	/**
	 * @return true if the block was marked to be pruned
	 */
	bool getPruneFlag();

	/**
	 * Sets the PruneFlag state
	 */
	void setPruneFlag(bool newState);

	/**
	 * @return block's father
	 */
	Block* getPrevBlock();

	private:
		/* Indicates if this block will be pruned on prune_chain return */
		bool _pruneFlag;

		/* Block Unique Number */
		int _blockId;

		/* Block Height */
		int _height;

		/* Block Data Hash */
		char* _hash;

		/* Pointer to Parent Block */
		Block* _prevBlock;
};

#endif