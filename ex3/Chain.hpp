#include <vector>
#include "Block.hpp"

#define FAIL -1
class Chain
{
public:
	
	~Chain();

	/**
	 * @return the chain's max height
	 */
	int getMaxHeight(void);

	/**
	 * @return the chain's size
	 */

	int getSize(void);

	/**
	 * @return iterator to the chain's tails
	 */
	std::vector<Block*>::iterator getTails(void);
	void pushBlock(Block *newTail);
	
	/**
	 * @return true if was initiated
	 */ 
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