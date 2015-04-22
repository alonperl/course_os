#include "Chain.hpp"

Chain::Chain()
{
_maxHeight = 0;
_size = 0;
_initiated = true;
}
	Chain::~Chain();
/**
 * @return the chain's max height
 */
int Chain::getMaxHeight(void)
{
	return _maxHeight;
}

/**
 * @return the chain's size
 */
int Chain::getSize(void)
{
	return _size;
}

/**
 * @return the chain's tails
 */
std::vector<Block*>::iterator Chain::getTails(void)
{
	return _tails.begin();
}

bool Chain::initiated(void)
{
	return s_initiated;
}

void Chain::create()
{
	s_instance = new Chain();
	s_initiated = true;
}

Chain *Chain::getInstance()
{
	if (s_initiated == true)
	{
		return s_instance;
	}
		throw FAIL;
}

void pushBlock(Block *newTail)
{
	// TODO: think on how to add a child
	if(newTail.getPrevBlock().get)
	_tails.push(newTail);
}