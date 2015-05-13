#include <string.h>
#include "Chain.hpp"

#define RESET_BLOCK_ID -1
#define GENESIS_BLOCK_NUM 0
#define FLAG_NEW_PENDING_BLOCK 1
#define FLAG_HASHING_FINISHED 2

// TODO write right numbers
#define STATUS_PENDING 1
#define STATUS_ATTACHED 2
#define NOT_FOUND -1

#define HASH_LENGTH 128
unsigned int event = 0;

// When worker finishes, this points to number of the relevant block
int lastHashedBlockId = RESET_BLOCK_ID;
char *lastHash;


Chain::Chain()
{	
	pthread_mutex_init(&_usedIDListMutex, NULL);
	pthread_mutex_init(&_deepestTailsMutex, NULL);
	pthread_mutex_init(&_attachedMutex, NULL);
	pthread_mutex_init(&_pendingMutex, NULL);

	pthread_cond_init(&_pendingCV, NULL);

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
	return Chain::getInstance()->daemonRoutine(ptr);
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
	return _tails.begin();
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
	return _pending.empty();
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
	pthread_mutex_lock(&_pendingMutex);
	while (!_isClosed)
	{
		// Wait for "hey! someone pending" signal
		pthread_cond_wait(&_pendingCV, &_pendingMutex);

		if (event & FLAG_NEW_PENDING_BLOCK)
		{	// Run hashing for new blocks
			for (std::deque<AddRequest*>::iterator it = _pending.begin(); it != _pending.end(); ++it)
			{
				// Create worker thread
				Worker *worker = new Worker(*it);
				_workers.push_back(worker);
				_pending.erase(it);
			}

			event ^= FLAG_NEW_PENDING_BLOCK;
		}

		if (event & FLAG_HASHING_FINISHED)
		{	// Update block hash
			// Get finished worker
			Worker *finishedWorker;
			Block *newBlock;

			for (std::vector<Worker*>::iterator it = _workers.begin(); it != _workers.end(); ++it)
			{
				if ((*it)->finished == 0)
				{
					finishedWorker = *it;
					_workers.erase(it);

					Block* father = (Block*)finishedWorker->blockFather;
					newBlock = new Block(finishedWorker->blockNum, (char*)finishedWorker->blockHash,
												HASH_LENGTH, father->getHeight()+1, father);
				}
			}


			// Attach block to chain
			// TODO Not sure about locking!
			pthread_mutex_lock(&_attachedMutex);
			_attached[newBlock->getId()] = newBlock;
			pthread_mutex_unlock(&_attachedMutex);

			// Reset blockId
			lastHashedBlockId = RESET_BLOCK_ID;
			event ^= FLAG_HASHING_FINISHED;
		}
	}
	// Unlock _pendingBlocks
	pthread_mutex_unlock(&_pendingMutex);

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

	pthread_create(&daemonThread, NULL, Chain::staticDaemonRoutine, NULL);	// master thread created
	
	// Create genesis block and insert to chain
	Block* genesisBlock = new Block(GENESIS_BLOCK_NUM, NULL, EMPTY, EMPTY, NULL); // TODO maybe height is determined from father later
	getInstance()->pushBlock(genesisBlock);

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
	if (!Chain::isInitiated())
	{
		return FAIL;
	}

	int newId = Chain::getLowestID();
	pthread_mutex_lock(&_pendingMutex);
	_pending.push_back(new AddRequest(data, length, newId, Chain::getRandomDeepest()));
	pthread_mutex_unlock(&_pendingMutex);

	return newId;
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
	if (!isInitiated())
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
	if (!isInitiated())
	{
		return FAIL;
	}
	return SUCESS;
}

int Chain::wasAdded(int blockNum)
{
	if (!isInitiated())
	{
		return FAIL;
	}

	std::unordered_map<unsigned int, Block*>::const_iterator foundBlock = _attached.find(blockNum);
	if (_attached.end() == foundBlock) // block was not added
	{
		// in case is in pending list
		std::deque<AddRequest*>::iterator it = _pending.begin();
		while (it != _pending.end())
		{
			if ((*it)->blockNum == blockNum)
			{
				return STATUS_PENDING;
			}
			*it++;
		}
		// in case id was not used
		return NOT_FOUND;
	}
	
	return STATUS_ATTACHED;
}

int Chain::chainSize()
{
	return isInitiated() ? Chain::chainSize() : FAIL;
}

//int Chain::pruneChain()
//{
//	if (!isInitiated())
//	{
//		return FAIL;
//	}
//	//TODO: add field named toPrune to all blocks and setter - make genesis crate with toPrune=false
//	pthread_mutex_lock(&_attachedMutex);
//	pthread_mutex_lock(&_deepestTailsMutex);
////	pthread_mutex_lock(&_tailsMutex); //TODO: do i need to lock more stuff??
//	Block *deepestBlock = getRandomDeepest();
//	// only in case we didn't reach the gensis block
//	// or we got to a part of a chain we pruned before - keep running
//	while (deepestBlock->getPrevBlock() != NULL || deepestBlock->toPrune != false)
//	{
//		deepestBlock->toPrune = false; //TODO: maybe change to setter
//		deepestBlock = deepestBlock->getPrevBlock();
//	}
//	// by now we marked everyone not to prune
//	Block* blockToPrune;
//	Block* tempBlock;
//	int counter = 0;
//	for (std::vector<Block*>::iterator it = _tails.begin(); it != _tails.end(); ++it)
//	{
//		blockToPrune = (*it);
//		//if we got a tail to delete earase from lists it were on
//		if (blockToPrune->toPrune == true)
//		{
//			// in case was one of the deepests
//			if (blockToPrune->getHeight() == _maxHeight)
//			{
//				// find where the block is in the vector
//				tempBlock = _deepestTails.begin();
//				while (blockToPrune != tempBlock)
//				{
//					i++;
//					tempBlock = _deepestTails.begin()+i
//				}
//				_deepestTails.erase(_deepestTails.begin()+i);
//				counter = 0;
//			}
//
//			//anyways delete from alltails list
//			tempBlock = _allTails.begin();
//			while (blockToPrune != tempBlock)
//			{
//				i++;
//				tempBlock = _allTails.begin()+i
//			}
//			_allTails.erase(_allTails.begin()+i);
//
//		}
//
//		// run through tree and delete all the sub branch
//		while (blockToPrune->toPrune == true)
//		{
//			tempBlock = blockToPrune->getPrevBlock();
//			_usedIDList.push_back(blockToPrune->getId()); //adds tp usedIDList
//			~Block(*blockToPrune); //destory the block
//			blockToPrune = tempBlock;
//		}
//
//	}
//	blockToPrune = NULL;
//	tempBlock = NULL;
//
//	pthread_mutex_unlock(&_allTailsMutex);
//	pthread_mutex_unlock(&_deepestTailsMutex);
//	pthread_mutex_unlock(&_blocksInChainMutex);
//	return SUCESS;
//}

//void *Chain::closeChain(void *c)
//{
//	pthread_mutex_lock(&_pendingBlocks);
//	std::deque<Block*>::iterator it = _pendingBlocks.begin();
//	while (it != mydeque.end())
//	{
//		//TODO - should First hash the data - and than print it
//		std::cout << *it->getHashData();
//		*it++;
//	}
//	pthread_mutex_unlock(&_pendingBlocks);
//
//}
//
//
//void Chain::closeChain()
//{
//	gClosing = true;
//
//	//TODO add this to all function except size()
//	//___________________
//	if (gClosing == true)
//	{
//		return FAIL;
//	}
//	//_____________________
//
//	pthread_create(&closingThread, NULL, Chain::closeChain, this);
//}