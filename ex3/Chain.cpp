/**
 * @file Chain.cpp
 * @author  griffonn ednussi
 * @version 1.0
 * @date 18 May 2015
 * 
 * @brief An implementation of blockchain data structure.
 *
 * @section LICENSE
 * This program is a free software. You can freely redistribute it.
 *
 * @section DESCRIPTION
 * Blockchain is a data structure that stores hashed data in a form of 
 * backlinked tree. Implemented as a singleton, this is the requirements
 * for exercise library.
 */
#include "Chain.hpp"
#include <limits.h>
#include <iostream>

#define GENESIS_BLOCK_NUM 0
#define CLOSE_CHAIN_NOT_CALLED -2

#define LOCK(x) if (pthread_mutex_lock(&x)) { exit(FAIL); }
#define UNLOCK(x) if (pthread_mutex_unlock(&x)) { exit(FAIL); }

#define MUTEX_INIT(x) if (pthread_mutex_init(&x, NULL)) { exit(FAIL); }
#define COND_INIT(x) if (pthread_cond_init(&x, NULL)) { exit(FAIL); }
#define MUTEX_DESTROY(x) if (pthread_mutex_destroy(&x)) { exit(FAIL); }
#define COND_DESTROY(x) if (pthread_cond_destroy(&x)) { exit(FAIL); }

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
	MUTEX_INIT(_recycledIdsMutex)
	MUTEX_INIT(_statusMutex)
	MUTEX_INIT(_pendingMutex)

	MUTEX_INIT(_chainMutex)
	MUTEX_INIT(_tailsMutex)
	MUTEX_INIT(_attachedMutex)
	
	MUTEX_INIT(_toLongestMutex)

	COND_INIT(_pendingCV)
	COND_INIT(_attachedCV)


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
	MUTEX_DESTROY(_recycledIdsMutex)
	MUTEX_DESTROY(_statusMutex)
	MUTEX_DESTROY(_pendingMutex)
	MUTEX_DESTROY(_chainMutex)
	MUTEX_DESTROY(_tailsMutex)
	MUTEX_DESTROY(_attachedMutex)
	MUTEX_DESTROY(_toLongestMutex)

	COND_DESTROY(_pendingCV)
	COND_DESTROY(_attachedCV)

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

	if (pthread_create(&s_daemonThread, NULL, Chain::staticDaemonRoutine, s_instance))
	{
		exit(FAIL);
	}

	// Create genesis block and insert to chain
	Block* genesisBlock = new Block(GENESIS_BLOCK_NUM, NULL, EMPTY, NULL);
	getInstance()->pushBlock(genesisBlock);
	getInstance()->_status[EMPTY] = ATTACHED;

	getInstance()->_size = EMPTY;

	genesisBlock = NULL;

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
		LOCK(_pendingMutex)

		// No requests to process
		if (_pending.empty())
		{
			if (_isClosing)
			{
				UNLOCK(_pendingMutex)
				_daemonWorkFlag = false;
				return NULL;
			}
			// Wait for "hey! someone pending" signal
			if (pthread_cond_wait(&_pendingCV, &_pendingMutex))
			{
				exit(FAIL);
			}
		}

		if (_isClosing)
		{
			UNLOCK(_pendingMutex)
			_daemonWorkFlag = false;
			return NULL;
		}

		if (_pending.size())
		{
			// Get new request
			Request *newReq = _pending.front();
			_pending.pop_front();
			// Release the queue
			UNLOCK(_pendingMutex)

			// Hash and attach
			blockId = _createBlock(newReq);

			// Delete processed request
			delete newReq;

			// Update status
			LOCK(_statusMutex)
			_status[blockId] = ATTACHED;
			UNLOCK(_statusMutex)

			// Send signal to anyone waiting for attaching
			if (pthread_cond_signal(&_attachedCV))
			{
				exit(FAIL);
			}
		}
		else // No requests to process
		{
			// Unlock _pending
			UNLOCK(_pendingMutex)
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
		if (pthread_cond_signal(&(chain->_pendingCV)))
		{
			exit(FAIL);
		}
	}
	
	if (pthread_join(s_daemonThread, NULL))
	{
		exit(FAIL);
	}

	s_instance = NULL;
	s_initiated = NULL;


	LOCK((chain->_pendingMutex))
	LOCK((chain->_attachedMutex))
	LOCK((chain->_tailsMutex))

	// Print out what's in pending list - and delete 'em
	char* unusedHash;
	while (chain->_pending.size())
	{
		Request *req = chain->_pending.front();
		chain->_pending.pop_front();

		unusedHash = getInstance()->_hash(req);
		std::cout << unusedHash << std::endl;
		free(unusedHash);

		delete req; // Free Request
	}
	unusedHash = NULL;

	chain->_pending.clear();
	// delete &(chain->_pending);

	chain->_tails.clear();
	// delete &(chain->_tails);

	// Delete from attached map - and destroy blocks
	for (BlockMap::iterator it = chain->_attached.begin(); it != chain->_attached.end(); ++it)
	{
		if (it->second != NULL)
		{
			delete it->second; // Destroy the block
		}
	}

	chain->_attached.clear();
	// delete &(chain->_attached);
	
	chain->_recycledIds.clear(); // Clear recycled ids
	// delete &(chain->_recycledIds);

	UNLOCK((chain->_tailsMutex))
	UNLOCK((chain->_attachedMutex))
	UNLOCK((chain->_pendingMutex))

	close_hash_generator();

	delete chain; // Delete instance, destroy mutexes and cvs
	chain = NULL;
	
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
	// throw FAIL;
	return NULL;
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
	LOCK(_chainMutex)
	LOCK(_tailsMutex)
	
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

	UNLOCK(_tailsMutex)
	UNLOCK(_chainMutex)
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
	LOCK(_recycledIdsMutex)

	if (_recycledIds.empty())
	{
		UNLOCK(_recycledIdsMutex)
		return _expected_size + 1;
	}

	_recycledIds.sort(); // Sort to get smallest at the front
	int smallestUsedId = _recycledIds.front();
	_recycledIds.remove(smallestUsedId); // Erase from used list

	UNLOCK(_recycledIdsMutex)

	return smallestUsedId;
}

/**
 * @return random longest tip
 */
Block* Chain::getRandomDeepest()
{
	LOCK(_tailsMutex)

	long index = rand() % _tails.at(_maxHeight).size();

	BlockVector highestLevel = _tails.at(_maxHeight);
	Block* randomLeaf = highestLevel[index];

	UNLOCK(_tailsMutex)
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
	LOCK(_pendingMutex)
	_pending.push_back(new Request(data, length, newId, father));
	UNLOCK(_pendingMutex)

	// Update status
	LOCK(_statusMutex)
	_status[newId] = PENDING;
	UNLOCK(_statusMutex)

	// Update expected size
	_expected_size++;

	// Signal daemon that it has more work
	if (pthread_cond_signal(&_pendingCV))
	{
		exit(FAIL);
	}

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
	if (!isInitiated() || _isClosing || !_isValidId(blockNum))
	{
		return FAIL;
	}

	LOCK(_statusMutex)
	if (_status.find(blockNum) != _status.end() && _status[blockNum] == ATTACHED)
	{
		UNLOCK(_statusMutex)
		return ATTACHED;
	}
	UNLOCK(_statusMutex)

	LOCK(_toLongestMutex)
	_toLongestFlags[blockNum] = true;
	UNLOCK(_toLongestMutex)

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
	if (!isInitiated() || _isClosing || !_isValidId(blockNum))
	{
		return FAIL;
	}

	LOCK(_statusMutex)
	
	int blockStatus;

	try
	{
		blockStatus = _status.at(blockNum);
	}
	catch (const std::out_of_range& e)
	{
		UNLOCK(_statusMutex)
		return NOT_FOUND;
	}

	switch (blockStatus)
	{
		case PENDING:
			LOCK(_pendingMutex)
			for (RequestQueue::iterator it = _pending.begin(); it != _pending.end(); ++it)
			{
				if ((*it)->blockNum == blockNum)
				{
					int blockId = _createBlock(*it);

					// Update status
					_status[blockId] = ATTACHED;

					delete *it;
					_pending.erase(it);

					UNLOCK(_pendingMutex)
					UNLOCK(_statusMutex)
					return SUCESS;
				}
			}

			UNLOCK(_pendingMutex)
			if (pthread_cond_wait(&_attachedCV, &_statusMutex))
			{
				exit(FAIL);
			}
			UNLOCK(_statusMutex)
			return SUCESS;

		case ATTACHED:
			UNLOCK(_statusMutex)
			return SUCESS;

		default:
			UNLOCK(_statusMutex)
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
	if (!isInitiated() || !_isValidId(blockNum))
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

	LOCK(_tailsMutex)
	int heightPos = 0;
	for (BlockHeightMap::iterator tailsIterator = _tails.begin(); 
		 tailsIterator != _tails.end(); tailsIterator++)
	{
		for (BlockVector::iterator heightIterator = _tails.at(heightPos).begin(); 
			 heightIterator != _tails.at(heightPos).end(); )
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
	UNLOCK(_tailsMutex)

	LOCK(_attachedMutex)
	for (BlockMap::iterator blockIterator = _attached.begin();
		 blockIterator != _attached.end(); )
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
	UNLOCK(_attachedMutex)
	return SUCESS;
}

/**
 * @brief Create closing thread.
 */
void Chain::closeChain()
{
	_isClosing = true;
	if (pthread_create(&_closingThread, NULL, Chain::closeChainLogic, this))
	{
		exit(FAIL);
	}
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
int Chain::_createBlock(Request *req)
{
	Block* cachedFather = req->father;

	// Save if current father is longest or not
	bool cachedLongest = cachedFather->getHeight() == Chain::getInstance()->getMaxHeight();
	bool rehash;
	char* blockHash;
	int rehashCount = 0;
	do
	{
		// TODO MEMORY LEAK _ IF REHASH FIRST MUST FREE
		blockHash = _hash(req);

		if ((_toLongestFlags[req->blockNum] && !cachedLongest) || cachedFather == NULL)
		{
			/* Continue rehashing if to_longest() was called for this blockNum
			   and currently it is not the deepest */
			rehash = true;
			rehashCount++;
			free(blockHash);

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
								req->father->getHeight() + 1, req->father);

	// Attach block to chain
	Chain::getInstance()->pushBlock(newBlock);

	newBlock = NULL;
	cachedFather = NULL;
	blockHash = NULL;

	return req->blockNum;
}

/**
 * @return Hash for given request
 */
char* Chain::_hash(Request *req)
{
	int nonce = generate_nonce(req->blockNum, req->father->getId());
	return generate_hash(req->data, (size_t)req->dataLength, nonce);
}

/**
 * @return true iff block number is valid
 */
bool Chain::_isValidId(int blockNum)
{
	return blockNum >= 0 && blockNum < INT_MAX;
}