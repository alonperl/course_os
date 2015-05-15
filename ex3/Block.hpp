#ifndef _BLOCK_H
#define _BLOCK_H

#include <pthread.h>

class Block
{
public:
	Block(int id, char* hash, int height, int hashLength, Block* father);
	~Block();

	/**
	 * @return the block's Id
	 */
	int getId();

	/**
	 * @return the block's height
	 */
	int getHeight();

	/**
	 * @return the block's hashdata
	 */
	char* getHash();

	/**
	 * @return the block's data length
	 */
	int getHashLength();

	/**
	 * @return true if the block needs to be pruned
	 */
	bool getPruneFlag();

	/**
	 * Sets the PruneFlag state
	 */
	void setPruneFlag(bool newState);

	/**
	 * @return the blocks father
	 */
	Block *getPrevBlock();

	pthread_mutex_t blockMutex;

	private:
		bool _pruneFlag;

		int _blockId;
		int _height;
		int _hashLength;
		char* _hash;
		
		Block* _prevBlock;
};

#endif