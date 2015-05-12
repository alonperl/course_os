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

bool getDaemonWorkFlag()
{
	return _daemonWorkFlag;
}

bool isPendingBlocksEmpty()
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
}

void Chain::deleteBlock(Block *toDelete)
{

}

/**
 * @return the lowest ID available
 */
int Chain::getLowestID()
{
	// TODO: looks for the lowest number available and returns it:
	// get lowest number from usedID list
	// get size of list - chose the smaller of the two
	pthread_mutex_lock(&chain->usedIDListMutex);
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
	pthread_mutex_unlock(&chain->usedIDListMutex);
	return _size;
}

Block *getFather()
{
	pthread_mutex_lock(&chain->deepestTails);

	pthread_mutex_unlock(&chain->deepestTails);
}

int Chain::maintain_chain(Chain *chain)
{
	//TODO logic of the deamon thread

	while(chain.getDaemonWorkFlag())
	{
		if (chain.isPendingBlocksEmpty())
		{

		}
	}
	//TODO if closed was called and there are still pending blocks
	//TODO should print them out than delete them

}

Block *Chain::genesis_Block_creator()
{
	Block genesisBlock = new Block(GENESIS_BLOCK_NUM, NULL, NULL, Chain::getMaxHeight());
	return &genesisBlock;
}

int Chain::initiateBlockchain()
{
	if (initiated())
	{
		return FAIL;
	}
	create(); // TODO make sure chain is singelton 
	init_hash_generator();
	pthread_create(&daemonThread, NULL, maintain_chain)	// master thread craeted
	pushBlock(genesis_Block_creator()); //create genesis block and insert to chain
}

int Chain::addBlock(char *data, int length)
{
	if (!Chain::initiated())
	{
		return FAIL;
	}

	Block *father = Chain::getFather();
	int blockID = Chain::getLowestID();
	Block newBlock = new Block(blockID, data, father, father.getHeight()+1);
	_pendingBlocks.push_back(&newBlock); 
	// TODO: don't forget the thread that manges this needs to check in the end if the father still exists otherwise recalculate the hashed-data

	return BlockID;
}

int Chain::toLongest(int blockNum)
{
	if (!initiated())
	{
		return FAIL;
	}
	//WHAT IF WE HAD INTERRUPT AFTER BLOCK WAS FOUND
	// AND THAN HE CAN"T FIND IT ANYMORE SINCE IT WAS ADDED??


	// check if was added 
	// finds the request in deamon list and changes parameters
	// 
}

int Chain::attachNow(int blockNum)
{
	if (!initiated())
	{
		return FAIL;
	}

}

int Chain::wasAdded(int blockNum)
{
	if (!initiated())
	{
		return FAIL;
	}
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
}

void Chain::closeChain()
{
	if (!initiated())
	{
		return FAIL;
	}
}

int Chain::returnOnClose()
{
	if (!initiated())
	{
		return FAIL;
	}

}