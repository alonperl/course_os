#include <string>

class Block
{
public:
	Block(int id, std::string dataHash, int dataLength, Block* father, int height);
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
	std::string getHash();

	/**
	 * @return the block's data length
	 */
	int getDataLength();
	
	/**
	 * Updates the block's hash
	 */
	void setHash(char *hash);

	/**
	 * @return the blocks father
	 */
	Block *getPrevBlock();

	pthread_mutex_t blockMutex;

	private:
		int _blockId;
		int _height;
		int _dataLength;
		std::string _dataHash;
		Block* _prevBlock;
};