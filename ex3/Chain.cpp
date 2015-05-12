#include "Chain.hpp"

#define GENESIS_BLOCK_NUM 0

pthread_t daemonThread;

Chain::Chain()
{
	// TODO make singelton
	_maxHeight = 0;
	_size = EMPTY;
	s_initiated = true;
}

//Chain::~Chain(); //TODO doesn't compile with destructor

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

bool Chain::getDaemonWorkFlag()
{
	return _daemonWorkFlag;
}

bool Chain::isPendingBlocksEmpty()
{
	if (_pendingBlocks.empty())
	{
		return true;
	}
	return false;
}

/**
 * @return iterator to the chain's tails
 */
std::vector<Block*>::iterator Chain::getTails(void)
{
	return _allTails.begin();
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
	(void) newTail; // supress unused warnnig
	/**
	//REWRITE AFTER FINISHED ADDBLOCK THREAD
	//___________________________________________________________
	//___________________________________________________________
	// add myself to tails list
	_tails.push(newTail); 
	// make me the tip
	tip = newTail; 
	//Anyways add size
	_size++;

	
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
	//___________________________________________________________
	//___________________________________________________________
	*/
}

void Chain::deleteBlock(Block *toDelete)
{
	(void) toDelete;
}

/**
 * @return the lowest ID available
 */
int Chain::getLowestID()
{
	// TODO: looks for the lowest number available and returns it:
	// get lowest number from usedID list
	// get size of list - chose the smaller of the two
	pthread_mutex_lock(&_usedIDListMutex);
	if (_usedIDList.empty())
	{
		return _size;
	}

	int smallestUsedId = _usedIDList.front(); //assuming usedID list is always sorted after adding an element there -if not change the .front())
	if (_size > smallestUsedId) 
	{
		_usedIDList.remove(smallestUsedId); // erase from used list
		return smallestUsedId;
	}
	pthread_mutex_unlock(&_usedIDListMutex);
	return _size;
}

Block *Chain::getFather()
{
	pthread_mutex_lock(&_deepestTailsMutex);

	pthread_mutex_unlock(&_deepestTailsMutex);
	return _tip; //TODO change - just for compiling
}

void Chain::maintainChain()
{
	//TODO logic of the deamon thread
	Chain *chain = Chain::getInstance();
	while(chain->getDaemonWorkFlag())
	{
		if (chain->isPendingBlocksEmpty())
		{

		}
	}
	//TODO if closed was called and there are still pending blocks
	//TODO should print them out than delete them
}

int Chain::initiateBlockchain()
{
	if (initiated())
	{
		return FAIL;
	}
	create(); // TODO make sure chain is singelton 
	init_hash_generator();
	pthread_create(&daemonThread, NULL, Chain::maintainChain)	// master thread created
	Block* genesisBlock = new Block(GENESIS_BLOCK_NUM, NULL, NULL, Chain::getMaxHeight());
	pushBlock(genesisBlock); //create genesis block and insert to chain
	return SUCESS;
}

int Chain::addBlock(char *data, int length)
{
	(void) length; //TODO -erase - to compile

	if (!Chain::initiated())
	{
		return FAIL;
	}

	Block *father = Chain::getFather();
	int blockID = Chain::getLowestID();
	Block *newBlock = new Block(blockID, data, father, father->getHeight()+1);
	_pendingBlocks.push_back(newBlock); 
	// TODO: don't forget the thread that manges this needs to check in the end if the father still exists otherwise recalculate the hashed-data

	return blockID;
}

int Chain::toLongest(int blockNum)
{
	(void) blockNum; //TODO erase - to compile
	if (!initiated())
	{
		return FAIL;
	}
	//WHAT IF WE HAD INTERRUPT AFTER BLOCK WAS FOUND
	// AND THAN HE CAN"T FIND IT ANYMORE SINCE IT WAS ADDED??


	// check if was added 
	// finds the request in deamon list and changes parameters
	// 
	return SUCESS;
}

int Chain::attachNow(int blockNum)
{
	(void) blockNum; //TODO erase - to compile
	if (!initiated())
	{
		return FAIL;
	}
	return SUCESS;
}

int Chain::wasAdded(int blockNum)
{
	(void) blockNum; //TODO erase - to compile
	if (!initiated())
	{
		return FAIL;
	}
	return SUCESS;
}

int Chain::chainSize()
{
	return initiated() ? Chain::getSize() : FAIL;
}

int Chain::pruneChain()
{
	if (!initiated())
	{
		return FAIL;
	}
	return SUCESS;
}

void Chain::closeChain()
{

}

int Chain::returnOnClose()
{
	if (!initiated())
	{
		return FAIL;
	}
	return SUCESS;
}