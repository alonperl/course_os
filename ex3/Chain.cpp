#include "Chain.hpp"
#include <unistd.h>
#include <iostream>

// TODO
#define HASH_LENGTH 128
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
	pthread_mutex_init(&_statusMutex, NULL);
	pthread_mutex_init(&_pendingMutex, NULL);

	pthread_mutex_init(&_chainMutex, NULL);
	pthread_mutex_init(&_deepestTailsMutex, NULL);
	pthread_mutex_init(&_attachedMutex, NULL);

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
	std::cout<< __FUNCTION__;pthread_mutex_lock(&_chainMutex);std::cout<< ": chain locked." <<std::endl;
	std::cout<< __FUNCTION__;pthread_mutex_lock(&_statusMutex);std::cout<< ": status locked." <<std::endl;
	
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

	std::cout<< __FUNCTION__;pthread_mutex_unlock(&_statusMutex);std::cout<< ": status unlocked." <<std::endl;
	std::cout<< __FUNCTION__;pthread_mutex_unlock(&_chainMutex);std::cout<< ": chain unlocked." <<std::endl;
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
	
	std::cout<< __FUNCTION__;pthread_mutex_lock(&_usedIDListMutex);std::cout<< ": usedIDList locked." <<std::endl;
	_usedIDList.remove(smallestUsedId); // erase from used list
	std::cout<< __FUNCTION__;pthread_mutex_unlock(&_usedIDListMutex);std::cout<< ": usedIDList unlocked." <<std::endl;
	return smallestUsedId;
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
	std::cout<< __FUNCTION__;pthread_mutex_lock(&_pendingMutex);std::cout<< ": pending locked." <<std::endl;
	while (s_initiated)
	{
		// No requests to process
		if (_pending.empty())
		{
			// Wait for "hey! someone pending" signal
			std::cout<< "pending unlocked - waiting\n";
			pthread_cond_wait(&_pendingCV, &_pendingMutex);
			std::cout<< "pending locked from signal. Have " << _pending.size() << " items pending.\n";
			wokeUp = true;
		}
		else
		{
			wokeUp = false;
		}

		// Chain is not initiated
		if (_isClosing)
		{
			std::cout<< __FUNCTION__;pthread_mutex_unlock(&_pendingMutex);std::cout<< ": pending unlocked." <<std::endl;
			return NULL;
		}

		// if (!wokeUp)
		// {
		// 	std::cout<< __FUNCTION__;pthread_mutex_lock(&_pendingMutex);std::cout<< ": pending locked." <<std::endl;
		// }

		if (_pending.size())
		{
			// Process new request
			AddRequest *newReq = _pending.front();
			
			std::cout<< __FUNCTION__;pthread_mutex_lock(&_statusMutex);std::cout<< ": status locked." <<std::endl;
			_status[newReq->blockNum] = PROCESSING;
			std::cout<< __FUNCTION__;pthread_mutex_unlock(&_statusMutex);std::cout<< ": status unlocked." <<std::endl;

			_pending.pop_front();
			
			std::cout<< __FUNCTION__;pthread_mutex_unlock(&_pendingMutex);std::cout<< ": pending unlocked." <<std::endl;

			// Do stuff
			createBlock(newReq);

			// Send signal to anyone waiting for attachNow
			pthread_cond_signal(&_attachedCV);
		}
	}
	// Unlock _pendingBlocks
	std::cout<< __FUNCTION__;pthread_mutex_unlock(&_pendingMutex);std::cout<< ": pending unlocked." <<std::endl;
	return NULL;
}

/**
 * @return random longest tip
 */
Block* Chain::getRandomDeepest()
{
	std::cout<< __FUNCTION__;pthread_mutex_lock(&_deepestTailsMutex);std::cout<< ": deepestTails locked." <<std::endl;
	std::cout << "Deepest Tails Size Is: " << _deepestTails.size() <<std::endl;
	if (_deepestTails.size() == 0)
	{
		std::cout << "\n\nCAN'T FUCKING BE \n\n";
	}
	long index = rand() % _deepestTails.size();
	std::cout << "(First in vector is 0), - Index is: " << index << " Vector Size is: " << _deepestTails.size() << std::endl; 
	std::cout<< __FUNCTION__;pthread_mutex_unlock(&_deepestTailsMutex);std::cout<< ": deepestTails unlocked." <<std::endl;
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
	std::cout<<"ENTER AddRequest"<<std::endl;
	if (!isInitiated() || _isClosing)
	{
		return FAIL;
	}

	int newId = getLowestID();

	// Add new task for daemon
	std::cout<< __FUNCTION__;pthread_mutex_lock(&_pendingMutex);std::cout<< ": pending locked." <<std::endl;
	_pending.push_back(new AddRequest(data, length, newId, getRandomDeepest()));
	std::cout<< __FUNCTION__;pthread_mutex_unlock(&_pendingMutex);std::cout<< ": pending unlocked." <<std::endl;

	// Update status
	std::cout<< __FUNCTION__;pthread_mutex_lock(&_statusMutex);std::cout<< ": status locked." <<std::endl;
	_status[newId] = PENDING;
	std::cout<< __FUNCTION__;pthread_mutex_unlock(&_statusMutex);std::cout<< ": status unlocked." <<std::endl;

	// Update expected size
	_expected_size++;

	// Signal daemon that it has more work
	pthread_cond_signal(&_pendingCV);
	std::cout<<"EXIT AddRequest"<<std::endl;
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
//std::cout<<"Enter ToLongest"<<std::endl;
//
//std::cout<<"Locking pending"<<std::endl;
//	std::cout<< __FUNCTION__;pthread_mutex_lock(&_pendingMutex);std::cout<< ": pending locked." <<std::endl;
//	for (std::deque<AddRequest*>::iterator it = _pending.begin(); it != _pending.end(); ++it)
//	{
//		if ((*it)->blockNum == blockNum)
//		{
//			(*it)->father = getRandomDeepest();
//			std::cout<< __FUNCTION__;pthread_mutex_unlock(&_pendingMutex);std::cout<< ": pending unlocked." <<std::endl;
//			return SUCESS;
//		}
//	}
//	std::cout<< __FUNCTION__;pthread_mutex_unlock(&_pendingMutex);std::cout<< ": pending unlocked." <<std::endl;
//std::cout<<"Locking workers"<<std::endl;
//	std::cout<< __FUNCTION__;pthread_mutex_lock(&_workerMutex);std::cout<< ": worker locked." <<std::endl;
//	for (std::vector<Worker*>::iterator it = _workers.begin(); it != _workers.end(); ++it)
//	{
//		if ((*it)->blockNum == blockNum) {
//			(*it)->_toLongestFlag = true;
//			std::cout<< __FUNCTION__;pthread_mutex_unlock(&_workerMutex);std::cout<< ": worker unlocked." <<std::endl;
//			return SUCESS;
//		}
//	}
//	std::cout<< __FUNCTION__;pthread_mutex_unlock(&_workerMutex);std::cout<< ": worker unlocked." <<std::endl;
//std::cout<<"Finished ToLongest"<<std::endl;
//
//std::cout<<"Locking status"<<std::endl;
	std::cout<< __FUNCTION__;pthread_mutex_lock(&_statusMutex);std::cout<< ": status locked." <<std::endl;
	if (_status.find(blockNum) != _status.end() && _status[blockNum] == ATTACHED)
	{
		std::cout<< __FUNCTION__;pthread_mutex_unlock(&_statusMutex);std::cout<< ": status unlocked." <<std::endl;
		return ATTACHED;
	}
	std::cout<< __FUNCTION__;pthread_mutex_unlock(&_statusMutex);std::cout<< ": status unlocked." <<std::endl;

	std::cout<< __FUNCTION__;pthread_mutex_lock(&_toLongestMutex);std::cout<< ": toLongest locked." <<std::endl;
	_toLongestFlags[blockNum] = true;
	std::cout<< __FUNCTION__;pthread_mutex_unlock(&_toLongestMutex);std::cout<< ": toLongest unlocked." <<std::endl;

	return NOT_FOUND;
}

int Chain::attachNow(int blockNum)
{
	if (!isInitiated() || _isClosing)
	{
		return FAIL;
	}

	/* TODO
	Two ideas:
	1. Don't lock status, check realtime if attached
	2. Lock status and make it work
	4. getRandomDeepest inf loop - Problem is in Phase 2 - Size is stuck on 50 and not increasing to 59 like it should
	*/

	//std::cout<< __FUNCTION__;pthread_mutex_lock(&_statusMutex);std::cout<< ": status locked." <<std::endl;
	switch (_status[blockNum])
	{
		case PENDING:
			std::cout << "Case Pending" << std::endl; 
			std::cout<< __FUNCTION__;pthread_mutex_lock(&_pendingMutex);std::cout<< ": pending locked." <<std::endl;
			for (std::deque<AddRequest*>::iterator it = _pending.begin(); it != _pending.end(); ++it)
			{
				if ((*it)->blockNum == blockNum)
				{
					std::cout << "Pending has " << _pending.size() << std::endl; 
					_pending.erase(it);
					_pending.push_front((*it));
					std::cout << "Pending updated to " << _pending.size() << std::endl; 
					break;
				}
			}
			std::cout<< __FUNCTION__;pthread_mutex_unlock(&_pendingMutex);std::cout<< ": pending unlocked." <<std::endl;

		case PROCESSING:
			std::cout << "Case Processing" << std::endl; 
			std::cout<< __FUNCTION__;pthread_mutex_lock(&_attachedMutex);std::cout<< ": attached locked." <<std::endl;

			std::cout<< __FUNCTION__;pthread_mutex_lock(&_statusMutex);std::cout<< ": status locked." <<std::endl;
			if (_status[blockNum] != ATTACHED)
			{
				pthread_cond_signal(&_pendingCV);std::cout<< "attachNow: signal sent." <<std::endl;
				std::cout<< __FUNCTION__;pthread_mutex_unlock(&_statusMutex);std::cout<< ": status unlocked." <<std::endl;
				pthread_cond_wait(&_attachedCV, &_attachedMutex);
			}
			std::cout<< __FUNCTION__;pthread_mutex_unlock(&_statusMutex);std::cout<< ": status unlocked." <<std::endl;
			
			std::cout<< __FUNCTION__;pthread_mutex_unlock(&_attachedMutex);std::cout<< ": attached unlocked." <<std::endl;
			return ATTACHED;

		case ATTACHED:
			//std::cout<< __FUNCTION__;pthread_mutex_unlock(&_statusMutex);std::cout<< ": status unlocked." <<std::endl;
			return ATTACHED;

		default:
			//std::cout<< __FUNCTION__;pthread_mutex_unlock(&_statusMutex);std::cout<< ": status unlocked." <<std::endl;
			return NOT_FOUND;
	}
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

	std::cout<< __FUNCTION__;pthread_mutex_lock(&_chainMutex);std::cout<< ": chain locked." <<std::endl;
	
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

	std::cout<< __FUNCTION__;pthread_mutex_unlock(&_chainMutex);std::cout<< ": chain unlocked." <<std::endl;
	return SUCESS;
}

void *Chain::closeChainLogic(void *pChain)
{
	Chain* chain = (Chain*)pChain;

	// Wait untill deamon closes
	pthread_cond_signal(&(chain->_pendingCV));
	pthread_join(s_daemonThread, NULL);
	s_instance = NULL;
	s_initiated = false;

	pthread_mutex_lock(&(chain->_pendingMutex));
	pthread_mutex_lock(&(chain->_chainMutex));
	pthread_mutex_lock(&(chain->_deepestTailsMutex));
	// print out what's in pending list - and delete 'em
	while (chain->_pending.size())
	{
		char* unusedHash = getInstance()->hash(chain->_pending.front());
		std::cout << unusedHash << std::endl;
		chain->_pending.pop_front();
	}

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

	pthread_mutex_unlock(&(chain->_chainMutex));
	pthread_mutex_unlock(&(chain->_pendingMutex));
	pthread_mutex_unlock(&(chain->_deepestTailsMutex));
	
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

	if (isInitiated())
	{
		return pthread_join(_closingThread, NULL);
	}

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

void Chain::createBlock(AddRequest *req)
{
	Block* cachedFather = req->father;
	// Save if current father is longest or not
	bool cachedLongest = cachedFather->getHeight() == Chain::getInstance()->getMaxHeight();
	bool rehash = false;
	char* blockHash;
	int rehashCount = 0;
	do
	{
		blockHash = hash(req);
//		blockHash = "a";

		if ((_toLongestFlags[req->blockNum] && !cachedLongest) || cachedFather == NULL)
		{
			rehash = true;
			rehashCount++;
			if (rehashCount > 10)
			{
				std::cout << "\nWhy The Endless Rehash (1) ?? \n\n";
			}
			cachedFather = Chain::getInstance()->getRandomDeepest();
			cachedLongest = cachedFather->getHeight() == Chain::getInstance()->getMaxHeight();
		}
		else
		{
			rehash = false;
		}
	} while (rehash);

	// _toLongestFlag was false till now, so from now on toLongest(this) will not act on this block
	// Create block
	Block* newBlock = new Block(req->blockNum, blockHash, HASH_LENGTH,
								req->father->getHeight()+1, req->father);

	// Attach block to chain
	Chain::getInstance()->pushBlock(newBlock);
}

char* Chain::hash(AddRequest *req)
{
	int nonce = generate_nonce(req->blockNum, req->father->getId());
	// return generate_hash(req->data, (size_t)req->dataLength, nonce);
	return "a";
}