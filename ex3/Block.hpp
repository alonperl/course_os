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
	std::string getHashData();

	/**
	 * @return the blocks father
	 */
	Block *getPrevBlock(); 

	private:
		int _blockId;
		int _height;
		std::string _dataHash;
		Block* _prevBlock;
};