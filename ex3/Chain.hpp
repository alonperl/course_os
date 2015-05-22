/**
 * @file Chain.cpp
 * @author  griffonn ednussi
 * @version 1.0
 * @date 18 May 2015
 * 
 * @brief Definition of blockchain data structure.
 *
 * @section LICENSE
 * This program is a free software. You can freely redistribute it.
 *
 * @section DESCRIPTION
 * Blockchain is a data structure that stores hashed data in a form of 
 * backlinked tree. Implemented as a singleton, this is the requirements
 * for exercise library.
 */
#ifndef _CHAIN_H
#define _CHAIN_H

#include <vector>
#include <unordered_map>
#include <deque>
#include <list>
#include "hash.h"
#include "Block.hpp"
#include "Request.hpp"

#define FAIL -1
#define EMPTY 0
#define SUCESS 0

typedef std::deque<Request*> RequestQueue;
typedef std::unordered_map<unsigned int, Block*> BlockMap;
typedef std::vector<Block*> BlockVector;
typedef std::unordered_map<unsigned int, BlockVector> BlockHeightMap;

/**
 * @brief Blockchain is a data structure that stores hashed data in a form of 
 * backlinked tree. Implemented as a singleton, this is the requirements
 * for exercise library.
 */
class Chain
{
public:
	/* Singleton instance getter */
	/**
	 * @return Chain instance if exists, throw FAIL otherwise
	 */
	static Chain *getInstance();

	/**
	 * @brief Create chain instance, init hash library, create genesis
	 *
	 * @return -1 if already initiated, 0 otherwise
	 */
	static int initChain();

	/**
	 * @brief Static handler for Daemon Thread
	 *
	 * @param pChain Pointer to the chain it is called for
	 */
	static void *staticDaemonRoutine(void* c);

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
	void *daemonRoutine(void*pChain);

	/**
	 * @return true if was initiated
	 */
	static bool isInitiated(void);

	/**
	 * @brief Routine to close the chain: forces the daemon to finish,
	 * 		  removes all the chain data, resets instance pointer.
	 */
	static void *closeChainLogic(void *c);

	/**
	 * @return the chain's max height
	 */
	int getMaxHeight(void);

	/**
	 * @brief Attach block to the Chain
	 * @details Update tails, update status
	 *
	 * @param newTail Pointer to new Block
	 */
	void pushBlock(Block* newBlock);

	/**
	 * @description Looks for the lowest number available and returns it:
	 *              - get lowest number from usedID list
					- get virtual size of chain - chooses smaller of the two
	 *
	 * @return the lowest ID available
	 */
	int  getLowestID();

	/**
	 * @return random longest tip
	 */
	Block* getRandomDeepest();

	/**
	 * @brief Add new data to pending list for daemon to process
	 *
	 * @param data New block data
	 * @param length Data length
	 *
	 * @return New block ID
	 */
	int  addRequest(char *data, int length);

	/**
	 * @brief Force block to be attached to longest chain when its time comes
	 *
	 * @param blockNum Unique block id
	 * @return -2 if block does not exist, 1 if succeed, -1 if error occurred
	 */
	int  toLongest(int blockNum);

	/**
	 * @brief Block current thread until block with given number is attached to the chain
	 *
	 * @param blockNum unique block id
	 * @return -2 if block does not exist, 0 if succeed, -1 if error occurred
	 */
	int  attachNow(int blockNum);

	/**
	 * @return block status, or -1 in case of illegal input
	 * Statuses:
	 * -2	NOT_FOUND
	 * 0	PENDING
	 * 1	ATTACHED
	 * 2	PROCESSING
	 */
	int  wasAdded(int blockNum);

	/**
	 * @return chain virtual size, i.e. number of blocks attached from last init
	 */
	int  chainSize();

	/**
	 * @brief	Randomly select longest chain and prune all forks.
	 * 			After pruneChain() returns, the chain is a single list.
	 *
	 * 	@return 0 if succeed, -1 if error occurred
	 */
	int  pruneChain();

	/**
	 * @brief	Randomly select longest chain and prune all forks.
	 * 			After pruneChain() returns, the chain is a single list.
	 *
	 * 	@return 0 if succeed, -1 if error occurred
	 */

	/**
	 * @brief Create closing thread.
	 */
	void closeChain();

	/**
	 * @brief Block calling thread until the chain is closed.
	 */
	int  returnOnClose();

private:
	/* Singleton instance */
	static Chain *s_instance;
	static bool s_initiated;

	/* Daemon thread */
	static pthread_t s_daemonThread;

	/* Close Routine thread */
	pthread_t _closingThread;

	/**
	 * @brief Chain Constructor
	 */
	Chain();

	/**
	 * @brief Chain Destructor
	 */
	~Chain();

	pthread_mutex_t _chainMutex;
	pthread_mutex_t _tailsMutex;
	pthread_mutex_t _attachedMutex;

	pthread_mutex_t _recycledIdsMutex;
	pthread_mutex_t _statusMutex;
	pthread_mutex_t _pendingMutex;

	pthread_mutex_t _toLongestMutex;

	pthread_cond_t _pendingCV;
	pthread_cond_t _attachedCV;

	int _maxHeight;
	int _expected_size;
	int _size;

	bool _daemonWorkFlag;
	bool _isClosing;

	/* Queue of incoming data for new blocks */
	RequestQueue _pending;

	/* BlockNum -> Block* map of already attached blocks */
	BlockMap _attached;

	/* Level -> vector<Block*> map */
	BlockHeightMap _tails;

	/* List of ids that can be reused by new blocks */
	std::list<int> _recycledIds;

	/* BlockNum->Status map */
	std::unordered_map<int, int> _status;

	/* BlockNum->_toLongestFlag map */
	std::unordered_map<int, bool> _toLongestFlags;

	/**
	 * @brief Hash given request's data, create block and attach it to the chain
	 *
	 * @return new block id
	 */
	int _createBlock(Request *req);

	/**
	 * @return Hash for given request
	 */
	char* _hash(Request *req);

	/**
	 * @return true iff block number is valid
	 */
	bool _isValidId(int blockNum);

};

#endif