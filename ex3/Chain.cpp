#include <iostream>
#include "Chain.hpp"

#define GENESIS_BLOCK_NUM 0

// TODO write right numbers
#define NOT_FOUND -2
#define PENDING 0
#define ATTACHED 1
#define PROCESSING 2
#define CLOSE_CHAIN_NOT_CALLED -2

// Define static members
bool Chain::s_initiated = false;
Chain *Chain::s_instance = NULL;
pthread_t Chain::daemonThread;

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
	if (Chain::s_initiated)
	{
		return Chain::s_instance;
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

void Chain::pushBlock(Block* newTail)
{
	// In case the new block is of bigger height update height
	if (Chain::getMaxHeight() < newTail->getHeight())
	{
		_maxHeight++;
	}

	// Add myself to tails list
	_tails.push_back(newTail);
	if (newTail->getHeight() == _maxHeight)
	{
		_deepestTails.push_back(newTail);
	}

	// Update status
	_status[newTail->getId()] = 1;

	_attached[newTail->getId()] = newTail;

	// // Size increment
	// _size++;

	// If I am not Genesis, I have a father leaf, that is no more a leaf
	if (newTail->getId() != 0)
	{
		// Delete my father from tails list
		int fatherId = newTail->getPrevBlock()->getId();
		for (std::vector<Block* >::iterator it = _tails.begin(); it != _tails.end(); ++it)
		{
			if ((*it)->getId() == fatherId)
			{
				_tails.erase(it);
				break;
			}
		}
		for (std::vector<Block* >::iterator it = _deepestTails.begin(); it != _deepestTails.end(); ++it)
		{
			if ((*it)->getId() == fatherId)
			{
				_deepestTails.erase(it);
				break;
			}
		}
	}
}

void Chain::deleteBlock(Block* toDelete)
{
	(void)toDelete;
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

	_usedIDList.sort();
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
		if (_pending.empty())
		{
			pthread_cond_wait(&_pendingCV, &_pendingMutex);
		}

		// Process new request TODO do we need to make this for all request or only for front?
		// Create worker thread
		AddRequest *newReq = _pending.front();
		_pending.pop_front();
		Worker *worker = new Worker(newReq);
		_workers.push_back(worker);
		// TODO unlock pending now?
		pthread_mutex_unlock(&_pendingMutex);
		worker->act();
	}
	// Unlock _pendingBlocks
	pthread_mutex_unlock(&_pendingMutex);

	return NULL;
}

/**
 * @return random longest tip
 */
Block* Chain::getRandomDeepest()
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

	Chain::s_instance = new Chain();
	Chain::s_initiated = true;

	init_hash_generator();

	pthread_create(&daemonThread, NULL, Chain::staticDaemonRoutine, NULL);	// master thread created
	
	// Create genesis block and insert to chain
	Block* genesisBlock(new Block(GENESIS_BLOCK_NUM, NULL, EMPTY, EMPTY, NULL)); // TODO maybe height is determined from father later
	getInstance()->pushBlock(genesisBlock);
	getInstance()->_status[EMPTY] = ATTACHED;

	// Update virtual size;
	getInstance()->_size++;

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

	// Virtual Size update
	_size++;

	// Add new task for daemon
	pthread_mutex_lock(&_pendingMutex);
	_pending.push_back(new AddRequest(data, length, newId, getRandomDeepest()));
	pthread_mutex_unlock(&_pendingMutex);

	// Signal daemon that it has more work
	pthread_cond_signal(&_pendingCV);

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
	if (!isInitiated() || _isClosing)
	{
		return FAIL;
	}

	if (_attached[blockNum] != NULL)
	{
		return ATTACHED;
	}

	for (std::deque<AddRequest*>::iterator it = _pending.begin(); it != _pending.end(); ++it)
	{
		std::cout<<it<<"\n";
		std::cout<<*it<<"\n";
		if ((*it)->blockNum == blockNum) {
			(*it)->father = getRandomDeepest();
			// return STATUS_PENDING
			return SUCESS;
		}
	}

	for (std::vector<Worker*>::iterator it = _workers.begin(); it != _workers.end(); ++it)
	{
		if ((*it)->req->blockNum == blockNum) {
			(*it)->_toLongestFlag = true;
			// return STATUS_WORKING
			return SUCESS;
		}
	}

	return NOT_FOUND;
}

int Chain::attachNow(int blockNum)
{
	if (!isInitiated() || _isClosing)
	{
		return FAIL;
	}

	if (_attached[blockNum] != NULL)
	{
		return ATTACHED;
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
		if ((*it)->req->blockNum == blockNum) {
			// return STATUS_WORKING
			return SUCESS;
		}
	}

	return NOT_FOUND;
}

int Chain::wasAdded(int blockNum)
{
	// TODO WasAdded should work when closing?
	if (!isInitiated() || _isClosing)
	{
		return FAIL;
	}

	return getBlockStatus(blockNum);
}

int Chain::chainSize()
{
	// TODO _size is updated only on actual attachment (in pushBlock)
	// TODO and this is good, but in test:69 busy_waiting for right size stucks
	// return (isInitiated() ? _size : FAIL);
	return (isInitiated() ? _attached.size()-1 : FAIL);
}

int Chain::pruneChain()
{
	if (!isInitiated() || _isClosing)
	{
		return FAIL;
	}
	//TODO: add field named toPrune to all blocks and setter - make genesis crate with toPrune = false
	pthread_mutex_lock(&_attachedMutex);
	pthread_mutex_lock(&_deepestTailsMutex);
	pthread_mutex_lock(&_tailsMutex); //TODO: do i need to lock more stuff??

	// Find random deepest and go from him to the top and mark
	// not to prune the longest path
	Block* deepestBlock = getRandomDeepest();

	// only in case we didn't reach the gensis block
	// or we got to a part of a chain we pruned before - keep running
	while (deepestBlock != NULL)// || deepestBlock->getPruneFlag())
	{
		// TODO refactor flag check, maybe use _atomic_flag
		deepestBlock->setPruneFlag(false);
		deepestBlock = deepestBlock->getPrevBlock();
	}
	// by now we marked everyone not to prune
	Block* blockToPrune;

	//TODO MEGA - is vector rearranging after erase??
printChain();
	//Delete from tails vector
	for (std::vector<Block* >::iterator it = _tails.begin(); it != _tails.end();)
	{
		blockToPrune = *it;
		if (blockToPrune->getPruneFlag())
		{
			it = _tails.erase(it);
		}
		else
		{
			++it;
		}
	}

	//Delete from deepest tails vector
	for (std::vector<Block* >::iterator it = _deepestTails.begin(); it != _deepestTails.end();)
	{
		blockToPrune = *it;
		if (blockToPrune->getPruneFlag())
		{
			it = _deepestTails.erase(it);
		}
		else
		{
			++it;
		}
	}

	//Delete from attached map - nad add id to list
	for (std::unordered_map<unsigned int, Block* >::iterator it = _attached.begin(); it != _attached.end();)
	{
		blockToPrune = it->second;
		std::cout<< blockToPrune << " " << blockToPrune->getId() << " " << blockToPrune->getPruneFlag()<<"\n";
		if (blockToPrune->getPruneFlag())
		{
			_usedIDList.push_back(blockToPrune->getId()); //adds tp usedIDList
			_attached.erase(it++);
			delete blockToPrune; //TODO: destory the block
		}
		else
		{
			++it;
		}
	}
printChain();
	pthread_mutex_unlock(&_tailsMutex);
	pthread_mutex_unlock(&_deepestTailsMutex);
	pthread_mutex_unlock(&_attachedMutex);
	return SUCESS;
}

void *Chain::closeChainLogic(void *ptr)
{
	(void)ptr;
	pthread_mutex_lock(&(Chain::getInstance()->_pendingMutex));
	std::deque<AddRequest*>::iterator it = Chain::getInstance()->_pending.begin();
	while (it != Chain::getInstance()->_pending.end())
	{
		//TODO - should First hash the data - and than print it
		std::cout << (*it)->data;
		*it++;
	}
	pthread_mutex_unlock(&(Chain::getInstance()->_pendingMutex));
	return NULL;
}


void Chain::closeChain()
{
	_isClosing = true;
	pthread_t closingThread;
	pthread_create(&closingThread, NULL, Chain::closeChainLogic, this);
	pthread_join(closingThread, NULL);
	pthread_cond_signal(&_finishedClosingCV);
}

int Chain::returnOnClose()
{
	if (!isInitiated())
	{
		return FAIL;
	}

	if( _isClosing != true)
	{
		return CLOSE_CHAIN_NOT_CALLED;
	}
	pthread_cond_wait(&_finishedClosingCV, &_pendingMutex);

	return SUCESS;
}


/**
 * @return block status, or -1 in case of illegal input
 * Statuses:
 * -2	NOT_FOUND
 * 0	PENDING
 * 1	ATTACHED
 * 2	PROCESSING
 */
int Chain::getBlockStatus(int blockNum)
{
	if (blockNum > getInstance()->getLowestID() || blockNum < 0)
	{
		return FAIL;
	}

	if (_status.find(blockNum) == _status.end())
	{
		return NOT_FOUND;
	}

	return _status[blockNum];
}

void Chain::printChain()
{
	std::cout << "SIZE " << _attached.size()-1 <<"\n";
	std::unordered_map<unsigned int, Block*>::iterator it = _attached.begin();
	int q = 0;
	while (it != _attached.end())
	{
		q = it->second->getHeight();
		while(q--)
		{
			std::cout << " ";
		}
		std::cout << it->second->getId() << ": H" << it->second->getHeight() << ", P" << it->second->getPruneFlag();
		if (it->second->getPrevBlock() != NULL)
		{
			std::cout << ", F" << it->second->getPrevBlock()->getId() << "\n";
		}
		else
		{
			std::cout << ", GENESIS\n";			
		}
		it++;
	}

}