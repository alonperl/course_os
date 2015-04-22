#include <vector>
#include "Block.hpp"

#define FAIL -1
class Chain
{
public:
	
	~Chain();

	int getMaxHeight(void);
	int getSize(void);
	std::vector<Block*>::iterator getTails(void);
	void pushBlock(Block *newTail);
	static bool initiated(void);
	static void create();
	static Chain *getInstance();

private:
	Chain(arguments);
	int _maxHeight;
	int _size;
	std::vector<Block*> *_tails;
	Block *tip;
	static bool s_initiated = false;
	static Chain *s_instance;

};