#ifndef _CHAIN_H
#define _CHAIN_H

#include <vector>
#include "Block.hpp"
#include <unordered_map>
#include <deque>
#include <list>
#include <atomic>
#include "hash.h"
#include "AddRequest.hpp"
#include "Worker.h"

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

	/**
	 * @return iterator to the chain's tails
	 */
	std::vector<Block*>::iterator getTails(void);
	
	void pushBlock(Block *newBlock);
	void deleteBlock(Block *toDelete);

	/**
	 * @return the lowest ID available
	 */
	int  getLowestID();

	bool isDaemonWorking();
	bool isPendingEmpty();
	
	void *daemonRoutine(void* c);
	Block *getRandomDeepest();

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

	Block *getTip();

private:
	static bool s_initiated;
	static Chain *s_instance;
	static pthread_t daemonThread;

	Chain();
	~Chain();

	pthread_mutex_t _usedIDListMutex;
	pthread_mutex_t _deepestTailsMutex;
	pthread_mutex_t _tailsMutex;
	pthread_mutex_t _attachedMutex;

	pthread_mutex_t _pendingMutex;

	pthread_cond_t _pendingCV;

	int _maxHeight;
	int _size;

	std::deque<AddRequest*> _pending;
	
	std::unordered_map<unsigned int, Block*> _attached;
	std::vector<Block*> _tails;
	std::vector<Block*> _deepestTails;
	std::list<int> _usedIDList;

	std::vector<Worker*> _workers;

	bool _daemonWorkFlag;
	std::atomic_bool _isClosing;
	std::atomic_bool _isClosed;

	Block *_tip;
	Block *_genesis;
};

#endif