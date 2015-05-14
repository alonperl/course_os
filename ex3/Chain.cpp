#include "Chain.hpp"

#define GENESIS_BLOCK_NUM 0

// TODO write right numbers
#define STATUS_PENDING 2
#define STATUS_ATTACHED 1
#define NOT_FOUND -2

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
	if (s_initiated)
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

		// Process new request TODO do we need to make this for all request or only for front?
		// Create worker thread
		AddRequest *newReq = _pending.front();
		_pending.pop_front();
		Worker *worker = new Worker(newReq);
		_workers.push_back(worker);
		// TODO unlock pending now?
		worker->act();
	}
	// Unlock _pendingBlocks
	pthread_mutex_unlock(&_pendingMutex);

	return NULL;
}

/**
 * @return random longest tip
 */
Block *Chain::getRandomDeepest()
{
	unsigned long index = rand() % _deepestTails.size();
	return _deepestTails[index];
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
	(void) blockNum;
	if (!isInitiated())
	{
		return FAIL;
	}

	if (_attached[blockNum] != NULL)
	{
		return STATUS_ATTACHED;
	}

	for (std::deque<AddRequest*>::iterator it = _pending.begin(); it != _pending.end(); ++it)
	{
		if ((*it)->blockNum == blockNum) {
			(*it)->father = getRandomDeepest();
			// return STATUS_PENDING
			return SUCESS;
		}
	}

	for (std::vector<Worker*>::iterator it = _workers.begin(); it != _workers.end(); ++it)
	{
		if ((*it)->blockNum == blockNum) {
			(*it)->_toLongestFlag = true;
			// return STATUS_WORKING
			return SUCESS;
		}
	}

	return NOT_FOUND;
}

int Chain::attachNow(int blockNum)
{
	if (!isInitiated())
	{
		return FAIL;
	}

	if (_attached[blockNum] != NULL)
	{
		return STATUS_ATTACHED;
	}

	for (std::deque<AddRequest*>::iterator it = _pending.begin(); it != _pending.end(); ++it)
	{
		if ((*it)->blockNum == blockNum) {
			/* This request is not yet processed, force it to process right now */
			// Lock pending from new requests
			pthread_mutex_lock(&_pendingMutex);
			/* TODO THIS IS OVERKILL BUT I LIKE IT
			// Run worker with chain blocking
			BlockingWorker *worker = new BlockingWorker(*it);
			worker->act();
			 */
			// Move desired block to the deque front
			_pending.erase(it);
			_pending.push_front((*it));
			// Unlock pending
			pthread_mutex_unlock(&_pendingMutex);
			return SUCESS;
		}
	}

	for (std::vector<Worker*>::iterator it = _workers.begin(); it != _workers.end(); ++it)
	{
		if ((*it)->blockNum == blockNum) {
			// return STATUS_WORKING
			return SUCESS;
		}
	}

	return NOT_FOUND;
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