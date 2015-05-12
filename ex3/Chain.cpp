#include "Chain.hpp"

#define RESET_BLOCK_ID -1
#define GENESIS_BLOCK_NUM 0
#define FLAG_NEW_PENDING_BLOCK 1
#define FLAG_HASHING_FINISHED 2

pthread_t daemonThread;

unsigned int event = 0;

// When worker finishes, this points to number of the relevant block
int lastHashedBlockId = RESET_BLOCK_ID;
char *lastHash;

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

/**
 * @brief Daemon thread routine
 * @details This function is passed as start_routine to daemon thread,
 *          it mainly waits for two event types:
 *          	- when new block enters _pendingBlocks queue, it creates
 *          	a worker to hash its data
 *          	- when hashing finished, it adds the block to chain and 
 *          	pops it from _pending
 * 
 * @param chain_ptr pointer to chain
 * @return NULL
 */
void *Chain::maintainChain(void *chain_ptr)
{
	// Lock _pendingBlocks
	pthread_mutex_lock(&_pendingBlocksMutex);
	while (!isClosed())
	{
		// Wait for "hey! someone pending" signal
		pthread_cond_wait(&_pendingBlocksCV, &_pendingBlocksMutex);

		if (event & FLAG_NEW_PENDING_BLOCK)
		{	// Run hashing for new blocks
			for (std::deque<Block*>::iterator it = _pendingBlocks.begin(); it != _pendingBlocks.end(); ++it)
			{
				// Get next pending block
				Block* nextBlock = *it;

				// Create worker thread
				pthread_t *worker;
				_workers.push_back(worker);

				pthread_create(worker, NULL, Chain::hash, nextBlock);	// master thread created
				pthread_join(*worker, NULL);
				// TODO: do we need to NULLify nextBlock? worker?
			}

			event ^= FLAG_NEW_PENDING_BLOCK;
		}

		if (event & FLAG_HASHING_FINISHED)
		{	// Update block hash
			// Get block
			Block *block;
			for (std::deque<Block*>::iterator it = _pendingBlocks.begin(); it != _pendingBlocks.end(); ++it)
			{
				if (*it->getId() == lastHashedBlockId);
				{
					block = *it;
					_pendingBlocks.erase(it);
					break;
				}
			}

			// Update the block hash
			pthread_mutex_lock(&(block->blockMutex));
			block->setHash(hash);
			pthread_mutex_unlock(&(block->blockMutex));

			// Attach block to chain
			// TODO Not sure about locking!
			pthread_mutex_lock(&chainMutex);
			_blocksInChain[block->getId()] = block;
			pthread_mutex_unlock(&chainMutex);

			// Reset blockId
			lastHashedBlockId = RESET_BLOCK_ID;
			event ^= FLAG_HASHING_FINISHED;
		}
	}
	// Unlock _pendingBlocks
	pthread_mutex_unlock(&_pendingBlocksMutex);

	return NULL;
}

/**
 * @brief Hashing routine
 * @details This function is passed as start_routine of workers.
 *          Its job is to calculate the hash of given block (and
 *          possibly rehash if its father was changed)
 * 
 * @param block_ptr Desired block
 * @return NULL
 */
void *Chain::hash(void *block_ptr)
{
	// TODO consider maybe split block and its data, we don't really 
	// need block here, only its and it father's id and the data
	
	Block *block = (Block*) block_ptr;
	int id = block->getId();

	// Save father id
	int fatherID = block->getPrevBlock()->getId();

	// Calculate hash
	do
	{	
		int nonce = generate_nonce(block->getId(), block->getPrevBlock()->getId());
		char* hash = generate_hash(block->getData(), block->getDataLength(), nonce);

		// If the father was changed meanwhile, update it and recalculate the hash
		fatherID = block->getPrevBlock()->getId();
	} while (block->getPrevBlock()->getId() != fatherID);

	// TODO Consider using retval of pthread_join
	// Busy wait, not good!
	while(lastHashedBlockId != RESET_BLOCK_ID);  //		\ 
	lastHashedBlockId = id;						 //		| TODO Must be atomic
	strcpy(lastHash, hash);						 //		/

	// Send signal to daemon that hashing is finished
	pthread_mutex_lock(&_pendingBlocksMutex);
	event |= FLAG_HASHING_FINISHED;
    pthread_cond_signal(&var);
	pthread_mutex_unlock(&_pendingBlocksMutex);
    
	return NULL;
}


int Chain::initiateBlockchain()
{
	if (initiated())
	{
		return FAIL;
	}
	create(); // TODO make sure chain is singelton 
	init_hash_generator();
	pthread_create(&daemonThread, NULL, Chain::maintainChain, this);	// master thread created
	Block* genesisBlock = new Block(GENESIS_BLOCK_NUM, NULL, NULL, Chain::getMaxHeight()); // TODO maybe height is determined from father later
	pushBlock(genesisBlock); //create genesis block and insert to chain
	return SUCESS;
}

int Chain::addBlock(char *data, int length)
{
	if (!Chain::initiated())
	{
		return FAIL;
	}

	Block *father = Chain::getFather();
	int blockID = Chain::getLowestID();
	Block *newBlock = new Block(blockID, data, length, father, father->getHeight()+1);
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