#include <vector>
#include "Block.hpp"
#include <unordered_map>
#include <deque>
#include <list>
#include "hash.h"

#define FAIL -1
#define EMPTY 0
#define SUCESS 0

class Chain
{
public:
	static void *staticDaemonRoutine(void* c);
	static  void *hash(void *block_ptr);

	static bool isInitiated(void);
	static Chain *getInstance();

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

	int  initChain();
	int  addRequest(char *data, int length);
	int  toLongest(int blockNum);
	int  attachNow(int blockNum);
	int  wasAdded(int blockNum);
	int  chainSize();
	int  pruneChain();
	void closeChain();
	int  returnOnClose();

private:
	static bool s_initiated;
	static Chain *s_instance;
	Chain();
	~Chain();
	
	pthread_mutex_t _usedIDListMutex;
	pthread_mutex_t _deepestTailsMutex;
	pthread_mutex_t _attachedMutex;
	pthread_mutex_t _pendingMutex;

	pthread_cond_t _pendingBlocksCV;

	pthread_t daemonThread;

	int _maxHeight;
	int _size;

	std::deque<Block*> _pending;
	
	std::unordered_map<unsigned int, Block*> _attached;
	std::vector<Block*> _tails;
	std::vector<Block*> _deepestTails;
	std::list<int> _usedIDList;

	std::vector<pthread_t*> _workers;

	bool _daemonWorkFlag;
	bool _isClosed;

	Block *_tip;
	Block *_genesis;
};