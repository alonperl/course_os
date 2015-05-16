#include <iostream>
#include "Chain.hpp"

#define GENESIS_BLOCK_NUM 0
// Hello Eran
// TODO write right numbers
#define STATUS_PENDING 2
#define STATUS_ATTACHED 1
#define NOT_FOUND -2

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
	pthread_cond_init(&_finishedClosing, NULL);

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
		_tails.erase(newTail.getPrevBlock());
	}
	//___________________________________________________________
	//___________________________________________________________
	*/
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

	Chain::s_instance = new Chain();
	Chain::s_initiated = true;

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
	if (!isInitiated() || _isClosing)
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
	if (!isInitiated() || _isClosing)
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
	// TODO WasAdded should work when closing?
	if (!isInitiated() || _isClosing)
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
	Block *deepestBlock = getRandomDeepest();

	// only in case we didn't reach the gensis block
	// or we got to a part of a chain we pruned before - keep running
	while (deepestBlock->getPrevBlock() != NULL || deepestBlock->getPruneFlag())
	{
		// TODO refactor flag check, maybe use _atomic_flag
		deepestBlock->setPruneFlag(false);
		deepestBlock = deepestBlock->getPrevBlock();
	}
	// by now we marked everyone not to prune
	Block* blockToPrune;
	int counter = 0;

	//TODO MEGA - is vector rearranging after erase??

	//Delete from tails vector
	for (std::vector<Block*>::iterator it = _tails.begin(); it != _tails.end(); ++it)
	{
		blockToPrune = *it;
		if (blockToPrune->getPruneFlag())
		{
			_tails.erase(_tails.begin() + counter);
		}
		counter++;
	}

	//Delete from deepest tails vector
	counter = 0;
	for (std::vector<Block*>::iterator it = _deepestTails.begin(); it != _tails.end(); ++it)
	{
		blockToPrune = *it;
		if (blockToPrune->getPruneFlag())
		{
			_tails.erase(_tails.begin() + counter);
		}
		counter++;
	}

	//Delete from attached map - nad add id to list
	for (auto it = _attached.begin(); it != _attached.end(); ++it)
	{
		blockToPrune = *it;
		if (blockToPrune->getPruneFlag())
		{
			_usedIDList.push_back(blockToPrune->getId()); //adds tp usedIDList
			_tails.erase(_tails.begin() + counter);
		}
		~Block(blockToPrune); //TODO: destory the block
		counter++;
	}

	blockToPrune = NULL;
	pthread_mutex_unlock(&_tailsMutex);
	pthread_mutex_unlock(&_deepestTailsMutex);
	pthread_mutex_unlock(&_attachedMutex);
	return SUCESS;
}

void *Chain::closeChainLogic(void *ptr)
{
	(void)ptr; //Suppress warnings.
	pthread_mutex_lock(&(Chain::getInstance()->_pendingMutex));
	std::deque<AddRequest*>::iterator it = Chain::getInstance()->_pending.begin();
	// print out what's in pending list - and delete 'em
	while (it != Chain::getInstance()->_pending.end())
	{
		//TODO - should First hash the data - and than print it
		std::cout << (*it)->data; //TODO: probebly should print /n enter
		_pending.erase(it);
		*it++;
	}
	// _pending.clear(); TODO maybe add to be sure

	Block *blockToDelete;
	//Delete everything on tails and deepest vectors

	//Delete from tails vector
	_tails.clear();
	//Delete from deepest tails vector
	_deepestTails.clear();
	//Delete from attached map - and destroy blocks
	for (auto it = _attached.begin(); it != _attached.end(); ++it)
	{
		blockToDelete = *it;
		_tails.erase(_tails.begin() + counter);
		~Block(blockToPrune); //TODO: destory the block
		counter++;
	}
	_usedIDList.clear();
	_workers.clear();
	~Chain();


	pthread_mutex_unlock(&(Chain::getInstance()->_pendingMutex));
	return NULL;
}


void Chain::closeChain()
{
	_isClosing = true;
	pthread_t closingThread;
	pthread_create(&closingThread, NULL, Chain::closeChainLogic, this);
	pthread_join(closingThread, NULL);
	pthread_cond_signal(&_finishedClosing);
}

int Chain::returnOnClose()
{
	if (!isInitiated())
	{
		return FAIL;
	}

	if( _isClosing != true)
	{
		return CLOSE_CHAIN_WASNT_CALLED;
	}
	pthread_cond_wait(&_finishedClosing, &_pendingMutex);

	return SUCESS;
}
