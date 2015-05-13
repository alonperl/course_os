#include <pthread>

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
	 * @return the blocks father
	 */
	Block *getPrevBlock();

	pthread_mutex_t blockMutex;

	private:
		int _blockId;
		int _height;
		int _hashLength;
		char* _hash;
		Block* _prevBlock;
};