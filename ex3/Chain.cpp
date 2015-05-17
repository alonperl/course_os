#include <iostream>
#include <unistd.h>
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
pthread_t Chain::s_daemonThread;

Chain::Chain()
{	
	pthread_mutex_init(&_usedIDListMutex, NULL);
	pthread_mutex_init(&_deepestTailsMutex, NULL);
	pthread_mutex_init(&_attachedMutex, NULL);
	pthread_mutex_init(&_pendingMutex, NULL);

	pthread_cond_init(&_pendingCV, NULL);
	pthread_cond_init(&_attachedCV, NULL);

	_maxHeight = EMPTY;
	_expected_size = EMPTY;
	_isClosing = false;

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
	return NULL;
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
	pthread_mutex_lock(&_tailsMutex);
	pthread_mutex_lock(&_deepestTailsMutex);
	pthread_mutex_lock(&_attachedMutex);
	
	int height = newTail->getHeight();

	// In case the new block is of bigger height update height
	if (Chain::getMaxHeight() < height)
	{
		_maxHeight++;
	}

	// Add myself to tails list
	_tails.push_back(newTail);
	if (height == _maxHeight)
	{
		_deepestTails.push_back(newTail);
		for (std::vector<Block* >::iterator it = _deepestTails.begin(); 
			 it != _deepestTails.end(); it++)
		{
			if (*it != NULL && (*it)->getHeight() < height)
			{
				_deepestTails.erase(it);
			}
		}


	}

	// If I am not Genesis, I have a father leaf, that is no more a leaf
	if (newTail->getId() != GENESIS_BLOCK_NUM)
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
	_attached[newTail->getId()] = newTail;

	// Update status
	_status[newTail->getId()] = ATTACHED;

	// Virtual Size update
	_size++;

	pthread_mutex_unlock(&_attachedMutex);
	pthread_mutex_unlock(&_deepestTailsMutex);
	pthread_mutex_unlock(&_tailsMutex);
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
		return _expected_size+1;
	}

	_usedIDList.sort();
	int smallestUsedId = _usedIDList.front(); //assuming usedID list is always sorted after adding an element there -if not change the .front())
	
	pthread_mutex_lock(&_usedIDListMutex);
	_usedIDList.remove(smallestUsedId); // erase from used list
	pthread_mutex_unlock(&_usedIDListMutex);
	return smallestUsedId;
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
	bool wokeUp = false;

	// Lock _pendingBlocks
	pthread_mutex_lock(&_pendingMutex);
	while (s_initiated)
	{
		// No requests to process
		if (_pending.empty())
		{
			// Wait for "hey! someone pending" signal
			pthread_cond_wait(&_pendingCV, &_pendingMutex);
			wokeUp = true;
		}
		else
		{
			wokeUp = false;
		}

		// Chain is not initiated
		if (!s_initiated)
		{
			return NULL;
		}

		if (!wokeUp)
		{
			pthread_mutex_lock(&_pendingMutex);
		}

		if (_pending.size())
		{
			// Process new request
			AddRequest *newReq = _pending.front();
			// Create worker thread
			Worker *worker = new Worker(newReq);
			_workers.push_back(worker);

			_pending.pop_front();
			pthread_mutex_unlock(&_pendingMutex);

			// Do stuff
			worker->act();

			// Send signal to anyone waiting for attachNow
			pthread_cond_signal(&_attachedCV);
		}
	}
	// Unlock _pendingBlocks
	// pthread_mutex_unlock(&_pendingMutex);
	return NULL;
}

/**
 * @return random longest tip
 */
Block* Chain::getRandomDeepest()
{
	pthread_mutex_lock(&_deepestTailsMutex);
	long index = rand() % _deepestTails.size();
	pthread_mutex_unlock(&_deepestTailsMutex);
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

	pthread_create(&s_daemonThread, NULL, Chain::staticDaemonRoutine, NULL);	// master thread created
	
	// Create genesis block and insert to chain
	Block* genesisBlock(new Block(GENESIS_BLOCK_NUM, NULL, EMPTY, EMPTY, NULL)); // TODO maybe height is determined from father later
	getInstance()->pushBlock(genesisBlock);
	getInstance()->_status[EMPTY] = ATTACHED;
	getInstance()->_size = EMPTY;

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

	// Add new task for daemon
	pthread_mutex_lock(&_pendingMutex);
	_pending.push_back(new AddRequest(data, length, newId, getRandomDeepest()));

	pthread_mutex_unlock(&_pendingMutex);

	// Signal daemon that it has more work
	pthread_cond_signal(&_pendingCV);

	// Update expected size
	_expected_size++;
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
		if ((*it)->blockNum == blockNum)
		{
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
	if (!isInitiated() || _isClosing)
	{
		return FAIL;
	}

	// Lock pending from new requests
	std::cout<<"Locking _pending...\n";
	pthread_mutex_lock(&_pendingMutex);
	std::cout<<"Locking _attached...\n";
	pthread_mutex_lock(&_attachedMutex);

	if (_attached.find(blockNum) != _attached.end() && _attached[blockNum] != NULL)
	{
		std::cout<<"FOUND Unlocking _attached...\n";
		pthread_mutex_unlock(&_attachedMutex);
		return ATTACHED;
	}

	for (std::deque<AddRequest*>::iterator it = _pending.begin(); it != _pending.end(); ++it)
	{
		if ((*it)->blockNum == blockNum) {
			/* This request is not yet processed, force it to process right now */
			
			/* TODO THIS IS OVERKILL BUT I LIKE IT
			// Run worker with chain blocking
			BlockingWorker *worker = new BlockingWorker(*it);
			worker->act();
			 */
			// Move desired block to the deque front
			_pending.erase(it);
			_pending.push_front((*it));
			// Unlock pending
			std::cout<<"IN PENDING Unlocking _pending...\n";
			pthread_mutex_unlock(&_pendingMutex);

			std::cout<<"IN PENDING Waiting for _attachedCV...\n";
			pthread_cond_wait(&_attachedCV, &_attachedMutex);
			std::cout<<"IN PENDING Unlocking _attached...\n";
			pthread_mutex_unlock(&_attachedMutex);
			
			return ATTACHED;
		}
	}

	for (std::vector<Worker*>::iterator it = _workers.begin(); it != _workers.end(); ++it)
	{
		if ((*it)->req->blockNum == blockNum)
		{
			std::cout<<"IN WORKERS Waiting for _attachedCV...\n";
			pthread_cond_wait(&_attachedCV, &_attachedMutex);
			std::cout<<"IN WORKERS Unlocking _attached...\n";
			pthread_mutex_unlock(&_attachedMutex);
			std::cout<<"IN WORKERS Unlocking _pending...\n";
			pthread_mutex_unlock(&_pendingMutex);

			return ATTACHED;
		}
	}
	
	std::cout<<"Final Unlocking _attached...\n";
	pthread_mutex_unlock(&_attachedMutex);
	std::cout<<"Final Unlocking _pending...\n";
	pthread_mutex_unlock(&_pendingMutex);
	
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
	return (isInitiated() ? _size : FAIL);
}

int Chain::pruneChain()
{
	// TODO prune is non-blocking! but we block...?
	if (!isInitiated() || _isClosing)
	{
		return FAIL;
	}

	// Save random longest chain
	Block* deepestBlock = getRandomDeepest();

	pthread_mutex_lock(&_attachedMutex);	
	pthread_mutex_lock(&_tailsMutex);
	pthread_mutex_lock(&_deepestTailsMutex);
	
	// Bubble up on longest chain and mark not to prune it
	while (deepestBlock != NULL)
	{
		deepestBlock->setPruneFlag(false);
		deepestBlock = deepestBlock->getPrevBlock();
	}
	
	// by now we marked everyone not to prune
	Block* temp;

	//Delete from tails vector
	for (std::vector<Block* >::iterator it = _tails.begin(); it != _tails.end();)
	{
		temp = *it;
		if (temp != NULL && temp->getPruneFlag())
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
		temp = *it;
		if (temp != NULL && temp->getPruneFlag())
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
		temp = it->second;
		if (temp != NULL && temp->getPruneFlag())
		{
			_usedIDList.push_back(temp->getId()); // Reuse later
			_attached.erase(it++);
			delete temp; // Finally destory the block
		}
		else
		{
			++it;
		}
	}

	temp = NULL;

	pthread_mutex_unlock(&_deepestTailsMutex);
	pthread_mutex_unlock(&_tailsMutex);
	pthread_mutex_unlock(&_attachedMutex);
	return SUCESS;
}

void *Chain::closeChainLogic(void *pChain)
{
	Chain* chain = (Chain*)pChain;
	pthread_mutex_lock(&(chain->_pendingMutex));

	// print out what's in pending list - and delete 'em
	while (chain->_pending.size())
	{
		std::cout << Worker::hash(chain->_pending.front()) << std::endl;
		chain->_pending.pop_front();
	}
	// _pending.clear(); TODO maybe add to be sure

	Block* temp;
	//Delete everything on tails and deepest vectors

	//Delete from tails vector
	chain->_tails.clear();
	//Delete from deepest tails vector
	chain->_deepestTails.clear();
	//Delete from attached map - and destroy blocks
	for (std::unordered_map<unsigned int, Block*>::iterator it = chain->_attached.begin(); it != chain->_attached.end(); ++it)
	{
		temp = it->second;
		if (temp != NULL)
		{
			delete temp; // Destory the block
		}
	}
	chain->_attached.clear();
	chain->_usedIDList.clear();
	chain->_workers.clear();

	pthread_mutex_unlock(&(chain->_pendingMutex));
	
	s_initiated = false;
	s_instance = NULL;
	// TODO check how to properly destroy
	// s_daemonThread = NULL;
	
	pthread_cond_signal(&(chain->_pendingCV));
	pthread_join(s_daemonThread, NULL);

	delete chain;

	return NULL;
}


void Chain::closeChain()
{
	_isClosing = true;
	pthread_create(&_closingThread, NULL, Chain::closeChainLogic, this);
}

int Chain::returnOnClose()
{
	if (!isInitiated())
	{
		return FAIL;
	}

	if(!_isClosing)
	{
		return CLOSE_CHAIN_NOT_CALLED;
	}

	pthread_join(_closingThread, NULL);

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
	// if (blockNum > || blockNum < 0)
	// {
		// return FAIL;
	// }

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
		if (it->second != NULL)
		{
			q = it->second->getHeight();
			while(q--)
			{
				std::cout << " ";
			}
			std::cout << it->first;
			std::cout << ": H" << it->second->getHeight() << ", P" << it->second->getPruneFlag();
			if (it->second->getPrevBlock() != NULL)
			{
				std::cout << ", F" << it->second->getPrevBlock()->getId() << "\n";
			}
			else
			{
				std::cout << ", GENESIS\n";			
			}
		}
		it++;
	}
}

void Chain::printDeepest()
{
	std::cout << "DEEPEST SIZE " << _deepestTails.size() <<"\n";
	std::vector<Block*>::iterator it = _deepestTails.begin();
	int q = 0;
	while (it != _deepestTails.end())
	{
		if (*it != NULL)
		{
			q = (*it)->getHeight();
			while(q--)
			{
				std::cout << " ";
			}
			std::cout << (*it)->getId();
			std::cout << ": H" << (*it)->getHeight() << ", P" << (*it)->getPruneFlag();
			if ((*it)->getPrevBlock() != NULL)
			{
				std::cout << ", F" << (*it)->getPrevBlock()->getId() << "\n";
			}
			else
			{
				std::cout << ", GENESIS\n";			
			}
		}
		it++;
	}
}