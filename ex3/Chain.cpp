#include "Chain.hpp"
#include <unistd.h>
#include <iostream>

#define GENESIS_BLOCK_NUM 0
#define CLOSE_CHAIN_NOT_CALLED -2

/* Statuses */
#define NOT_FOUND -2
#define PENDING 0
#define ATTACHED 1
#define PROCESSING 2

// Define static members
bool Chain::s_initiated = false;
Chain *Chain::s_instance = NULL;
pthread_t Chain::s_daemonThread;

/**
 * @brief Chain Constructor
 */
Chain::Chain()
{	
	pthread_mutex_init(&_recycledIdsMutex, NULL);
	pthread_mutex_init(&_statusMutex, NULL);
	pthread_mutex_init(&_pendingMutex, NULL);

	pthread_mutex_init(&_chainMutex, NULL);
	pthread_mutex_init(&_tailsMutex, NULL);
	pthread_mutex_init(&_attachedMutex, NULL);

	pthread_cond_init(&_pendingCV, NULL);
	pthread_cond_init(&_attachedCV, NULL);

	_maxHeight = EMPTY;
	_expected_size = EMPTY;

	_isClosing = false;
	s_initiated = true;
}

/**
 * @brief Chain Destructor
 */
Chain::~Chain()
{
	pthread_mutex_destroy(&_recycledIdsMutex);
	pthread_mutex_destroy(&_statusMutex);
	pthread_mutex_destroy(&_pendingMutex);

	pthread_mutex_destroy(&_chainMutex);
	pthread_mutex_destroy(&_tailsMutex);
	pthread_mutex_destroy(&_attachedMutex);

	pthread_cond_destroy(&_pendingCV);
	pthread_cond_destroy(&_attachedCV);

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

	pthread_create(&s_daemonThread, NULL, Chain::staticDaemonRoutine, s_instance);

	// Create genesis block and insert to chain
	Block* genesisBlock(new Block(GENESIS_BLOCK_NUM, NULL, EMPTY, NULL)); // TODO maybe height is determined from father later
	getInstance()->pushBlock(genesisBlock);
	getInstance()->_status[EMPTY] = ATTACHED;

	getInstance()->_size = EMPTY;

	return SUCESS;
}

/**
 * @brief Static handler for Daemon Thread
 * 
 * @param pChain Pointer to the chain it is called for
 */
void *Chain::staticDaemonRoutine(void *pChain)
{
	return Chain::getInstance()->daemonRoutine(pChain);
}

/**
 * @brief Daemon thread routine
 * @details This function is passed as start_routine to daemon thread,
 *          it waits for new requests to come, and when _pendingCV is fired:
 *          	- daemon takes the request from _pending queue and hashes its data
 *          	- when hashing finished, it adds the block to chain
 *
 * @param chain_ptr pointer to chain
 * @return NULL
 */
void* Chain::daemonRoutine(void *pChain)
{
	(void) pChain;
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
				_daemonWorkFlag = false;
				return NULL;
			}
			// Wait for "hey! someone pending" signal
			pthread_cond_wait(&_pendingCV, &_pendingMutex);
		}

		if (_isClosing)
		{
			pthread_mutex_unlock(&_pendingMutex);
			_daemonWorkFlag = false;
			return NULL;
		}

		if (_pending.size())
		{
			// Get new request
			Request *newReq = _pending.front();
			_pending.pop_front();
			// Release the queue
			pthread_mutex_unlock(&_pendingMutex);

			// Hash and attach
			blockId = createBlock(newReq);

			// Update status
			pthread_mutex_lock(&_statusMutex);
			_status[blockId] = ATTACHED;
			pthread_mutex_unlock(&_statusMutex);

			// Send signal to anyone waiting for attaching
			pthread_cond_signal(&_attachedCV);
		}
		else // No requests to process
		{
			// Unlock _pending
			pthread_mutex_unlock(&_pendingMutex);
		}
	}

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
 * @brief Routine to close the chain: forces the daemon to finish,
 * 		  removes all the chain data, resets instance pointer.
 */
void* Chain::closeChainLogic(void *pChain)
{
	Chain* chain = (Chain*)pChain;

	// Wait until daemon closes
	while(chain->_daemonWorkFlag)
	{
		pthread_cond_signal(&(chain->_pendingCV));
	}
	pthread_join(s_daemonThread, NULL);

	s_instance = NULL;
	s_initiated = false;

	pthread_mutex_lock(&(chain->_pendingMutex));
	pthread_mutex_lock(&(chain->_attachedMutex));
	pthread_mutex_lock(&(chain->_tailsMutex));

	// Print out what's in pending list - and delete 'em
	while (chain->_pending.size())
	{
		Request *req = chain->_pending.front();
		chain->_pending.pop_front();

		char* unusedHash = getInstance()->hash(req);
		std::cout << unusedHash << std::endl;
		delete req; // Free Request
	}

	chain->_tails.clear();

	// Delete from attached map - and destroy blocks
	for (std::unordered_map<unsigned int, Block*>::iterator it = chain->_attached.begin(); it != chain->_attached.end(); ++it)
	{
		if (it->second != NULL)
		{
			delete it->second; // Destroy the block
		}
	}

	chain->_attached.clear();
	chain->_recycledIds.clear(); // Clear recycled ids

	pthread_mutex_unlock(&(chain->_tailsMutex));
	pthread_mutex_unlock(&(chain->_attachedMutex));
	pthread_mutex_unlock(&(chain->_pendingMutex));

	delete chain; // Delete instance, destroy mutexes and cvs

	return NULL;
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

/**
 * @brief Attach block to the Chain
 * @details Update tails, update status
 * 
 * @param newTail Pointer to new Block
 */
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

	// Attach me to chain
	_attached[newTail->getId()] = newTail;

	// Virtual Size update
	_size++;

	pthread_mutex_unlock(&_tailsMutex);
	pthread_mutex_unlock(&_chainMutex);
}

/**
 * @description Looks for the lowest number available and returns it:
 *              - get lowest number from usedID list
				- get virtual size of chain - chooses smaller of the two
 *
 * @return the lowest ID available
 */
int Chain::getLowestID()
{
	pthread_mutex_lock(&_recycledIdsMutex);

	if (_recycledIds.empty())
	{
		return _expected_size+1;
	}

	_recycledIds.sort(); // Sort to get smallest at the front
	int smallestUsedId = _recycledIds.front();
	_recycledIds.remove(smallestUsedId); // Erase from used list

	pthread_mutex_unlock(&_recycledIdsMutex);

	return smallestUsedId;
}

/**
 * @return random longest tip
 */
Block* Chain::getRandomDeepest()
{
	pthread_mutex_lock(&_tailsMutex);

	long index = rand() % _tails.at(_maxHeight).size();

	BlockMap highestLevel = _tails.at(_maxHeight);
	Block* randomLeaf = highestLevel[index];

	pthread_mutex_unlock(&_tailsMutex);
	return randomLeaf;
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
 * @brief Force block to be attached to longest chain when its time comes
 * 
 * @param blockNum Unique block id
 * @return -2 if block does not exist, 1 if succeed, -1 if error occurred
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

/**
 * @brief Block current thread until block with given number is attached to the chain
 *
 * @param blockNum unique block id
 * @return -2 if block does not exist, 0 if succeed, -1 if error occurred
 */
int Chain::attachNow(int blockNum)
{
	if (!isInitiated() || _isClosing)
	{
		return FAIL;
	}

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

/**
 * @return block status, or -1 in case of illegal input
 * Statuses:
 * -2	NOT_FOUND
 * 0	PENDING
 * 1	ATTACHED
 * 2	PROCESSING
 */
int Chain::wasAdded(int blockNum)
{
	if (!isInitiated())
	{
		return FAIL;
	}

	if (_status.find(blockNum) == _status.end())
	{
		return NOT_FOUND;
	}

	return _status[blockNum];
}

/**
 * @return chain virtual size, i.e. number of blocks attached from last init
 */
int Chain::chainSize()
{
	return (isInitiated() ? _size : FAIL);
}

/**
 * @brief	Randomly select longest chain and prune all forks.
 * 			After pruneChain() returns, the chain is a single list.
 *
 * 	@return 0 if succeed, -1 if error occurred
 */
int Chain::pruneChain()
{
	if (!isInitiated() || _isClosing)
	{
		return FAIL;
	}

	// Save random longest chain
	Block* deepestBlock = getRandomDeepest();

	// Bubble up on random longest chain and mark not to prune it
	while (deepestBlock != NULL)
	{
		deepestBlock->setPruneFlag(false);
		deepestBlock = deepestBlock->getPrevBlock();
	}

	pthread_mutex_lock(&_tailsMutex);
	int heightPos = 0;
	for (BlockHeightMap::iterator tailsIterator = _tails.begin(); tailsIterator != _tails.end(); tailsIterator++)
	{
		for (BlockVector::iterator heightIterator = _tails.at(heightPos).begin(); heightIterator != _tails.at(heightPos).end();)
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
	pthread_mutex_unlock(&_tailsMutex);

	pthread_mutex_lock(&_attachedMutex);
	for (std::unordered_map<unsigned int, Block*>::iterator blockIterator = _attached.begin(); blockIterator != _attached.end();)
	{
		if (blockIterator->second != NULL && blockIterator->second->getPruneFlag())
		{
			_recycledIds.push_back(blockIterator->second->getId()); // Reuse later
			delete blockIterator->second; // Finally destory the block
			blockIterator = _attached.erase(blockIterator);
		}
		else
		{
			blockIterator++;
		}
	}
	pthread_mutex_unlock(&_attachedMutex);
	return SUCESS;
}

/**
 * @brief Create closing thread.
 */
void Chain::closeChain()
{
	_isClosing = true;
	pthread_create(&_closingThread, NULL, Chain::closeChainLogic, this);
}

/**
 * @brief Block calling thread until the chain is closed.
 */
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
 * @brief Hash given request's data, create block and attach it to the chain
 *
 * @return new block id
 */
int Chain::createBlock(Request *req)
{
	Block* cachedFather = req->father;

	// Save if current father is longest or not
	bool cachedLongest = cachedFather->getHeight() == Chain::getInstance()->getMaxHeight();
	bool rehash;
	char* blockHash;
	int rehashCount = 0;
	do
	{
		blockHash = hash(req);

		if ((_toLongestFlags[req->blockNum] && !cachedLongest) || cachedFather == NULL)
		{
			/* Continue rehashing if to_longest() was called for this blockNum
			   and currently it is not the deepest */
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

/**
 * @return Hash for given request
 */
char* Chain::hash(Request *req)
{
	int nonce = generate_nonce(req->blockNum, req->father->getId());
	return generate_hash(req->data, (size_t)req->dataLength, nonce);
}