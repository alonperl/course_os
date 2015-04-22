class Block
{
public:
	Block(arguments);
	~Block();

	/**
	 * @return the block's Id
	 */
	int getId(void);

	/**
	 * @return the block's timestamp
	 */
	time_t getTimestamp(void);

	/**
	 * @return the block's height
	 */
	int getHeight(void);

	/**
	 * @return the block's hashdata
	 */
	string getHashData(void);

	/**
	 * @return the blocks father
	 */
	*Block getPrevBlock(void); 

	private:
		int _blockId;
		time_t _timestamp;
		int _height;
		string _hashData;
		Block* _prevBlock;
};