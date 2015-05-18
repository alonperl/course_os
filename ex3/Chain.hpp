#ifndef _CHAIN_H
#define _CHAIN_H

#include <vector>
#include <unordered_map>
#include <deque>
#include <list>
#include "hash.h"
#include "Block.hpp"
#include "AddRequest.hpp"

#define FAIL -1
#define EMPTY 0
#define SUCESS 0

class Chain
{
public:
	static void *staticDaemonRoutine(void* c);
	static bool isInitiated(void);
	static Chain *getInstance();
	static int initChain();

	/**
	 * @return the chain's max height
	 */
	int getMaxHeight(void);
	
	void pushBlock(Block* newBlock);
	void deleteBlock(Block* toDelete);

	/**
	 * @return the lowest ID available
	 */
	int  getLowestID();
	
	void *daemonRoutine(void* c);
	Block* getRandomDeepest();

	int  addRequest(char *data, int length);
	int  toLongest(int blockNum);
	int  attachNow(int blockNum);
	int  wasAdded(int blockNum);
	int  chainSize();
	int  pruneChain();
	void closeChain();
	// TODO @eran
	static void *closeChainLogic(void *c);
	int  returnOnClose();
void printChain();
private:
	static bool s_initiated;
	static Chain *s_instance;
	static pthread_t s_daemonThread;
	pthread_t _closingThread;

	Chain();
	~Chain();

	pthread_mutex_t _chainMutex;
	pthread_mutex_t _tailsMutex;
	pthread_mutex_t _attachedMutex;

	pthread_mutex_t _usedIDListMutex;
	pthread_mutex_t _statusMutex;
	pthread_mutex_t _pendingMutex;

	pthread_cond_t _pendingCV;
	pthread_cond_t _attachedCV;

	int _maxHeight;
	int _expected_size;
	int _size;

	std::deque<AddRequest*> _pending;

	std::unordered_map<unsigned int, Block*> _attached;
	std::unordered_map<int, vector<Block*> > _tails;
	// std::vector<Block*> _deepestTails;
	std::list<int> _usedIDList;

	bool _daemonWorkFlag;
	bool _isClosing;

	/**
	 * @return block status, or -1 in case of illegal input
	 * Statuses:
	 * -2	NOT_FOUND
	 * 0	PENDING
	 * 1	ATTACHED
	 * 2	PROCESSING
	 */
	int getBlockStatus(int blockNum);

	std::unordered_map<int, int> _status;
	pthread_mutex_t _toLongestMutex;
	std::unordered_map<int, bool>_toLongestFlags;

	void createBlock(AddRequest *req);
	char* hash(AddRequest *req);

};

#endif