#include "Chain.hpp"
#include "AddRequest.hpp"

#define RESET_BLOCK_ID -1
#define GENESIS_BLOCK_NUM 0
#define FLAG_NEW_PENDING_BLOCK 1
#define FLAG_HASHING_FINISHED 2

/*
unsigned int event = 0;

// When worker finishes, this points to number of the relevant block
int lastHashedBlockId = RESET_BLOCK_ID;
char *lastHash;
*/

Chain::Chain()
{	
	pthread_mutex_init(&_usedIDListMutex, NULL);
	pthread_mutex_init(&_deepestTailsMutex, NULL);
	pthread_mutex_init(&_attachedMutex, NULL);
	pthread_mutex_init(&_pendingMutex, NULL);

	pthread_cond_init(&_pendingBlocksCV, NULL);

	_isClosed = false;
	_maxHeight = EMPTY;
	_size = EMPTY;

	s_initiated = true;
}

Chain::~Chain()
{
	// TODO destroy mutexes and cv and maybe more thing
}

void *Chain::staticDaemonRoutine(void *ptr)
{
	return Chain::getInstance()->maintainChain(ptr);
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
		char* hash = generate_hash(block->getHash(), block->getDataLength(), nonce);

		// If the father was changed meanwhile, update it and recalculate the hash
		fatherID = block->getPrevBlock()->getId();
	} while (block->getPrevBlock()->getId() != fatherID);

	// TODO Consider using retval of pthread_join
	// Busy wait, not good!
	while(lastHashedBlockId != RESET_BLOCK_ID);  //		
	lastHashedBlockId = id;						 //	TODO Must be atomic
	strcpy(lastHash, hash);						 //		

	// Send signal to daemon that hashing is finished
	pthread_mutex_lock(&_pendingBlocksMutex);
	event |= FLAG_HASHING_FINISHED;
    pthread_cond_signal(&var);
	pthread_mutex_unlock(&_pendingBlocksMutex);
    
	return NULL;
}

/**
 * @return true if was initiated
 */ 
bool Chain::isInitiated(void)
{
	return s_initiated;
}

/**
 * @return Chain instance if exists, throw FAIL otherwise
 */
Chain *Chain::getInstance()
{
	if (s_initiated == true)
	{
		return s_instance;
	}
	throw FAIL;
}

/**
 * @return the chain's max height
 */
int Chain::getMaxHeight(void)
{
	return _maxHeight;
}

/**
 * @return iterator to the chain's tails
 */
std::vector<Block*>::iterator Chain::getTails(void)
{
	return _allTails.begin();
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
	// TODO
}

/**
 * @return the lowest ID available
 */
int Chain::getLowestID()
{
	// TODO: looks for the lowest number available and returns it:
	// get lowest number from usedID list
	// get size of list - chose the smaller of the two
	if (_usedIDList.empty())
	{
		return _size;
	}

	int smallestUsedId = _usedIDList.front(); //assuming usedID list is always sorted after adding an element there -if not change the .front())
	
	if (_size > smallestUsedId) 
	{
		pthread_mutex_lock(&_usedIDListMutex);
		_usedIDList.remove(smallestUsedId); // erase from used list
		pthread_mutex_unlock(&_usedIDListMutex);
		return smallestUsedId;
	}
	return _size;
}

// TODO @eran - what is this?
bool Chain::isDaemonWorking()
{
	return _daemonWorkFlag;
}

/**
 * @return Check if there are pending add requests
 */
bool Chain::isPendingEmpty()
{
	return _pendingBlocks.empty();
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
void *Chain::daemonRoutine(void *chain_ptr)
{
	(void) chain_ptr;

	// Lock _pendingBlocks
	pthread_mutex_lock(&_pendingBlocksMutex);
	while (!_isClosed)
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
				if ((*it)->getId() == lastHashedBlockId)
				{
					block = *it;
					_pendingBlocks.erase(it);
					break;
				}
			}

			// Update the block hash
			pthread_mutex_lock(&(block->blockMutex));
			block->setHash(lastHash);
			pthread_mutex_unlock(&(block->blockMutex));

			// Attach block to chain
			// TODO Not sure about locking!
			pthread_mutex_lock(&_blocksInChainMutex);
			_blocksInChain[block->getId()] = block;
			pthread_mutex_unlock(&_blocksInChainMutex);

			// Reset blockId
			lastHashedBlockId = RESET_BLOCK_ID;
			event ^= FLAG_HASHING_FINISHED;
		}
	}
	// Unlock _pendingBlocks
	pthread_mutex_unlock(&_pendingBlocksMutex);

	return NULL;
}

Block *Chain::getRandomDeepest()
{
	pthread_mutex_lock(&_deepestTailsMutex);

	pthread_mutex_unlock(&_deepestTailsMutex);
	return _tip; //TODO change - just for compiling
}

/**
 * @brief Create chain instance, init hash library, create genesis
 *
 * @return -1 if already initiated, 0 otherwise
 */
int Chain::initChain()
{
	if (isInitiated())
	{
		return FAIL;
	}

	s_instance = new Chain();
	s_initiated = true;

	init_hash_generator();

	pthread_create(&daemonThread, NULL, Chain::maintainChain, this);	// master thread created
	
	// Create genesis block and insert to chain
	Block* genesisBlock = new Block(GENESIS_BLOCK_NUM, NULL, NULL, Chain::getMaxHeight()); // TODO maybe height is determined from father later
	pushBlock(genesisBlock); 

	return SUCESS;
}

/**
 * @brief Add new data to pending list for daemon to process
 * 
 * @param data New block data
 * @param length Data length
 * 
 * @return New block ID
 */
int Chain::addRequest(char *data, int length)
{
	if (!Chain::initiated())
	{
		return FAIL;
	}

	pthread_mutex_lock(&_pendingMutex);
	_pending.push_back(new AddRequest(data, length, Chain::getLowestID(), Chain::getRandomDeepest()));
	pthread_mutex_unlock(&_pendingMutex);

	return blockID;
}

/**
 * @brief [brief description]
 * @details [long description]
 * 
 * @param blockNum [description]
 * @return [description]
 */
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
	if (!initiated())
	{
		return FAIL;
	}

	std::unordered_map<unsigned int, Block*>::const_iterator foundBlock = _blocksInChain.find(blockNum);
	if (_blocksInChain.end() == foundBlock) // block was not added
	{
		// in case is in pending list
		std::deque<Block*>::iterator it = _pendingBlocks.begin();
		while (it != _pendingBlocks.end())
		{
			if (*it->getId() == blockNum)
			{
				return FOUND_IN_PENDING;
			}
			*it++;
		}
		// in case id was not used
		return BLOCK_NUM_DOESNT_EXIST;
	}
	
	return FOUND;
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
	//TODO: add field named toPrune to all blocks and setter - make genesis crate with toPrune=false
	pthread_mutex_lock(&_blocksInChainMutex);
	pthread_mutex_lock(&_deepestTailsMutex);
	pthread_mutex_lock(&_allTailsMutex); //TODO: do i need to lock more stuff??
	Block *deepestBlock = getRandomDeepest();
	// only in case we didn't reach the gensis block
	// or we got to a part of a chain we pruned before - keep running
	while (deepestBlock->getPrevBlock() != NULL || deepestBlock->toPrune != false)
	{
		deepestBlock->toPrune = false; //TODO: maybe change to setter
		deepestBlock = deepestBlock->getPrevBlock();
	}
	// by now we marked everyone not to prune
	Block* blockToPrune;
	Block* tempBlock;
	int counter = 0;
	for (std::vector<Block*>::iterator it = _allTails.begin(); it != _allTails.end(); ++it)
	{
		blockToPrune = (*it);
		//if we got a tail to delete earase from lists it were on
		if (blockToPrune->toPrune == true)
		{
			// in case was one of the deepests
			if (blockToPrune->getHeight() == _maxHeight)
			{
				// find where the block is in the vector
				tempBlock = _deepestTails.begin();
				while (blockToPrune != tempBlock)
				{
					i++;
					tempBlock = _deepestTails.begin()+i
				}
				_deepestTails.erase(_deepestTails.begin()+i);
				counter = 0;
			}

			//anyways delete from alltails list
			tempBlock = _allTails.begin();
			while (blockToPrune != tempBlock)
			{
				i++;
				tempBlock = _allTails.begin()+i
			}
			_allTails.erase(_allTails.begin()+i);

		}

		// run through tree and delete all the sub branch
		while (blockToPrune->toPrune == true)
		{
			tempBlock = blockToPrune->getPrevBlock();
			_usedIDList.push_back(blockToPrune->getId()); //adds tp usedIDList
			~Block(*blockToPrune); //destory the block
			blockToPrune = tempBlock;
		}
		
	}
	blockToPrune = NULL;
	tempBlock = NULL;

	pthread_mutex_unlock(&_allTailsMutex);
	pthread_mutex_unlock(&_deepestTailsMutex);
	pthread_mutex_unlock(&_blocksInChainMutex);
	return SUCESS;
}

void Chain::closeChain()
{
	// TODO
}

int Chain::returnOnClose()
{
	if (!initiated())
	{
		return FAIL;
	}
	// TODO
	return SUCESS;
}