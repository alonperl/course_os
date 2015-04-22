class Block
{
public:
	Block(arguments);
	~Block();

int getId(void);
time_t getTimestamp(void);
int getHeight(void);
string getHashData(void);
*Block getPrevBlock(void); 

private:
	int _blockId;
	time_t _timestamp;
	int _height;
	string _hashData;
	Block* _prevBlock;
};