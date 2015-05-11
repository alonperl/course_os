#include "Chain.hpp"

Chain::Chain()
{
	// TODO make singelton
	_maxHeight = 0;
	_size = EMPTY;
	s_initiated = true;
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
 * @return iterator to the chain's tails
 */
std::vector<Block*>::iterator Chain::getTails(void)
{
	return _tails.begin();
}

/**
 * @return true if was initiated
 */ 
bool Chain::initiated(void)
{
	return s_initiated;
}

/**
 *
 */
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

void Chain::pushBlock(Block *newTail)
{
	// add myself to tails list
	_tails.push(newTail);
	// make me the tip
	tip = newTail; 
	//Anyways add size
	_size++;

	//TODO maybe height should be determint here??
	// in case the new block is of bigger height update height
	if (Chain::getMaxHeight() < newTail.getHeight())
	{
		_maxHeight++;
	}

	if (_size == EMPTY)
	{
		root = newTail;
	}
	else
	{
		//delete my father from tails list
		Block *fatherBlock = newTail.getPrevBlock();
		_tails.erease(newTail.getPrevBlock());

	}
}

void deleteBlock(Block *toDelete)
{

}

/**
 * @return the lowest ID available
 */
int getLowestID()
{
	if (usedIDList.empty())
	{
		return _size;
	}

	smallestUsedId = usedIDList.front(); //assuming usedID list is always sorted after adding an element there -if not change the .front())
	if (_size > smallestUsedId) 
	{
		usedIDList.remove(smallestUsedId); // erase from used list
		return smallestUsedId;
	}
	return _size;
}