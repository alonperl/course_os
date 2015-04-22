class Block
{
public:
	Block(arguments);
	~Block();

int getId(void);
int getTimestamp(void);
int getHeight(void);
string getHashData(void);
*Block getPrevBlock(void); 

private:
	int _blockId;
	int _timestamp;
	int _height;
	string _hashData;
	*Block _prevBlock;
};