#include "Chain.hpp"
#include <unistd.h>
#include <iostream>

// TODO
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
	pthread_mutex_init(&_tailsMutex, NULL);
	// pthread_mutex_init(&_deepestTailsMutex, NULL);
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
	pthread_mutex_lock(&_chainMutex);
	pthread_mutex_lock(&_tailsMutex);
	
	int height = newTail->getHeight();

	// In case the new block is of bigger height update chain height
	if (Chain::getMaxHeight() < height)
	{
		_maxHeight++;
	}

	// I am a leaf now
	// _tails[height][newTail->getId()] = newTail;
	_tails[height].push_back(newTail);
	
	/*// If I am not Genesis, my father is no more a leaf
	if (newTail->getId() != GENESIS_BLOCK_NUM)
	{
		int fatherId = newTail->getPrevBlock()->getId();
		int fatherHeight = newTail->getPrevBlock()->getHeight();
		_tails[fatherHeight].erase(fatherId);
	}
*/

	/*if (height == _maxHeight)
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
	}*/

	// Attach me to chain
	_attached[newTail->getId()] = newTail;

	// Virtual Size update
	_size++;

	pthread_mutex_unlock(&_tailsMutex);
	pthread_mutex_unlock(&_chainMutex);
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
	int blockId;
	_daemonWorkFlag = true;

	while (s_initiated)
	{
		// Lock _pendingBlocks
		pthread_mutex_lock(&_pendingMutex);
		
		// No requests to process
		if (_pending.empty())
		{
			if (_isClosing)
			{
				pthread_mutex_unlock(&_pendingMutex);
				std::cout <<"KILL MYSELF 1\n";
				_daemonWorkFlag = false;
				return NULL;
			}
			// Wait for "hey! someone pending" signal
			std::cout<< "pending unlocked - waiting\n";
			pthread_cond_wait(&_pendingCV, &_pendingMutex);
			std::cout << "pending locked from signal. Have " << _pending.size() << " items pending.\n";
		// 	wokeUp = true;
		// }
		// else
		// {
		// 	wokeUp = false;
		}

		if (_isClosing)
		{
			pthread_mutex_unlock(&_pendingMutex);
			std::cout <<"KILL MYSELF 2\n";
			_daemonWorkFlag = false;
			return NULL;
		}

		// if (!wokeUp)
		// {
		// 	pthread_mutex_lock(&_pendingMutex);
		// }

		if (_pending.size())
		{
			// Process new request
			Request *newReq = _pending.front();
			
			/*pthread_mutex_lock(&_statusMutex);
			_status[newReq->blockNum] = PROCESSING;
			pthread_mutex_unlock(&_statusMutex);*/

			_pending.pop_front();
			
			std::cout << "PRINTING\n";
			pthread_mutex_unlock(&_pendingMutex);

			// Do stuff
			blockId = createBlock(newReq);

			// Update status
			pthread_mutex_lock(&_statusMutex);
			_status[blockId] = ATTACHED;
			pthread_mutex_unlock(&_statusMutex);

			// Send signal to anyone waiting for attachNow
			pthread_cond_signal(&_attachedCV);
		}
		else
		{
			// Unlock _pendingBlocks
			pthread_mutex_unlock(&_pendingMutex);
		}
	}

	return NULL;
}

/**
 * @return random longest tip
 */
Block* Chain::getRandomDeepest()
{
	/*pthread_mutex_lock(&_deepestTailsMutex);
	std::cout << "Deepest Tails Size Is: " << _deepestTails.size() <<std::endl;
	if (_deepestTails.size() == 0)
	{
		std::cout << "\n\nCAN'T FUCKING BE \n\n";
	}
	long index = rand() % _deepestTails.size();
	std::cout << "(First in vector is 0), - Index is: " << index << " Vector Size is: " << _deepestTails.size() << std::endl; 
	pthread_mutex_unlock(&_deepestTailsMutex);
	return _deepestTails[index];*/

	pthread_mutex_lock(&_tailsMutex);
	long index = rand() % _tails.at(_maxHeight).size();
	BlockMap level = _tails.at(_maxHeight);
	Block* randomLeaf = level[index];
	pthread_mutex_unlock(&_tailsMutex);
	return randomLeaf;
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
	Block* genesisBlock(new Block(GENESIS_BLOCK_NUM, NULL, EMPTY, NULL)); // TODO maybe height is determined from father later
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
	if (!isInitiated() || _isClosing)
	{
		return FAIL;
	}

	int newId = getLowestID();
	Block* father = getRandomDeepest();

	// Add new task for daemon
	pthread_mutex_lock(&_pendingMutex);
	_pending.push_back(new Request(data, length, newId, father));
	pthread_mutex_unlock(&_pendingMutex);

	// Update status
	pthread_mutex_lock(&_statusMutex);
	_status[newId] = PENDING;
	pthread_mutex_unlock(&_statusMutex);

	// Update expected size
	_expected_size++;

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

	pthread_mutex_lock(&_statusMutex);
	if (_status.find(blockNum) != _status.end() && _status[blockNum] == ATTACHED)
	{
		pthread_mutex_unlock(&_statusMutex);
		return ATTACHED;
	}
	pthread_mutex_unlock(&_statusMutex);

	pthread_mutex_lock(&_toLongestMutex);
	_toLongestFlags[blockNum] = true;
	pthread_mutex_unlock(&_toLongestMutex);

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

	pthread_mutex_lock(&_statusMutex);
	int blockStatus = _status[blockNum];
	switch (blockStatus)
	{
		case PENDING:
			pthread_mutex_lock(&_pendingMutex);
			for (std::deque<Request *>::iterator it = _pending.begin(); it != _pending.end(); ++it)
			{
				if ((*it)->blockNum == blockNum)
				{
					/*_pending.push_front((*it));*/
					int blockId = createBlock(*it);

					// Update status
					_status[blockId] = ATTACHED;

					_pending.erase(it);

					pthread_mutex_unlock(&_pendingMutex);
					pthread_mutex_unlock(&_statusMutex);
					return SUCESS;
				}
			}

			pthread_mutex_unlock(&_pendingMutex);
			pthread_cond_wait(&_attachedCV, &_statusMutex);
			pthread_mutex_unlock(&_statusMutex);
			return SUCESS;

		case ATTACHED:
			pthread_mutex_unlock(&_statusMutex);
			return SUCESS;

		default:
			pthread_mutex_unlock(&_statusMutex);
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

	pthread_mutex_lock(&_chainMutex);
	
	// Bubble up on random longest chain and mark not to prune it
	while (deepestBlock != NULL)
	{
		deepestBlock->setPruneFlag(false);
		deepestBlock = deepestBlock->getPrevBlock();
	}

	int heightPos = 0;
	for (BlockHeightMap::iterator tailsIterator = _tails.begin(); tailsIterator != _tails.end(); tailsIterator++)
	{
		for (BlockMap::iterator heightIterator = _tails.at(heightPos).begin(); heightIterator != _tails.at(heightPos).end();)
		{
			if ((*heightIterator) != NULL && (*heightIterator)->getPruneFlag())
			{
				heightIterator = _tails.at(heightPos).erase(heightIterator);
			}
			else
			{
				heightIterator++;
			}
		}

		if (_tails.at(heightPos).empty())
		{
			_tails.erase(tailsIterator);
		}

		heightPos++;
	}

	/*//Delete from tails vector
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
	}*/

	//Delete from attached map - nad add id to list
	for (std::unordered_map<unsigned int, Block*>::iterator blockIterator = _attached.begin(); blockIterator != _attached.end();)
	{
		if (blockIterator->second != NULL && blockIterator->second->getPruneFlag())
		{
			_usedIDList.push_back(blockIterator->second->getId()); // Reuse later
			delete blockIterator->second; // Finally destory the block
			blockIterator = _attached.erase(blockIterator);
		}
		else
		{
			blockIterator++;
		}
	}

	pthread_mutex_unlock(&_chainMutex);
	return SUCESS;
}

void *Chain::closeChainLogic(void *pChain)
{
	Chain* chain = (Chain*)pChain;

	std::cout <<"isClosing?" << chain->_isClosing << "\n";
	std::cout <<"CLOSE STARTED\n";
	// Wait untill deamon closes
	while(chain->_daemonWorkFlag)
	{
		pthread_cond_signal(&(chain->_pendingCV));
	}
	std::cout << chain->_daemonWorkFlag;
	// for (int i = 0; i < 2000; i++) {std::cout << ".";}
	pthread_join(s_daemonThread, NULL);
	
	s_instance = NULL;
	s_initiated = false;

	std::cout <<"CLOSE STARTED LOCKING\n";
	std::cout <<"CLOSE LOCK 1\n";
	pthread_mutex_lock(&(chain->_pendingMutex));
	std::cout <<"CLOSE LOCK 2\n";
	pthread_mutex_lock(&(chain->_chainMutex));
	std::cout <<"CLOSE LOCK 3\n";
	pthread_mutex_lock(&(chain->_tailsMutex));
	std::cout <<"CLOSE LOCKED\n";
	// print out what's in pending list - and delete 'em
	while (chain->_pending.size())
	{
		char* unusedHash = getInstance()->hash(chain->_pending.front());
		std::cout << unusedHash << std::endl;
		chain->_pending.pop_front();
	}

	// TODO as it is now map of maps, do we need to erase every inner map?
	chain->_tails.clear();

	//Delete from attached map - and destroy blocks
	for (std::unordered_map<unsigned int, Block*>::iterator it = chain->_attached.begin(); it != chain->_attached.end(); ++it)
	{
		if (it->second != NULL)
		{
			delete it->second; // Destory the block
		}
	}
	
	chain->_attached.clear();
	chain->_usedIDList.clear();

	pthread_mutex_unlock(&(chain->_tailsMutex));
	pthread_mutex_unlock(&(chain->_chainMutex));
	pthread_mutex_unlock(&(chain->_pendingMutex));
	
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
		if (pthread_join(_closingThread, NULL))
		{
			return FAIL;
		}
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
/*
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
}*/

int Chain::createBlock(Request *req)
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

		if ((_toLongestFlags[req->blockNum] && !cachedLongest) || cachedFather == NULL)
		{
			rehash = true;
			rehashCount++;

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
	Block* newBlock = new Block(req->blockNum, blockHash,
								req->father->getHeight()+1, req->father);

	// Attach block to chain
	Chain::getInstance()->pushBlock(newBlock);

	return newBlock->getId();
}

char* Chain::hash(Request *req)
{
	int nonce = generate_nonce(req->blockNum, req->father->getId());
	return generate_hash(req->data, (size_t)req->dataLength, nonce);
}