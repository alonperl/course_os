#ifndef _CHAIN_H
#define _CHAIN_H

#include <vector>
#include <unordered_map>
#include <deque>
#include <list>
#include "Block.hpp"
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
	
	void pushBlock(Block* newBlock);
	void deleteBlock(Block* toDelete);

	/**
	 * @return the lowest ID available
	 */
	int  getLowestID();

	bool isDaemonWorking();
	bool isPendingEmpty();
	
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
	pthread_cond_t _finishedClosingCV;


	int _maxHeight;
	int _size;

	std::deque<AddRequest*> _pending;
	
	std::unordered_map<unsigned int, Block*> _attached;
	std::vector<Block*> _tails;
	std::vector<Block*> _deepestTails;
	std::list<int> _usedIDList;

	std::vector<Worker*> _workers;

	bool _daemonWorkFlag;
	bool _isClosing;
	bool _isClosed;

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
};

#endif