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
	
	//~Chain();

	/**
	 * @return the chain's max height
	 */
	int		getMaxHeight(void);

	/**
	 * @return the chain's size
	 */

	int		getSize(void);

	/**
	 * @return iterator to the chain's tails
	 */
	std::vector<Block*>::iterator getTails(void);
	
	/**
	 *
	 */
	void	pushBlock(Block *newTail);

	void	deleteBlock(Block *toDelete);

	/**
	 * @return the lowest ID available
	 */
	int		getLowestID();
	bool	getDaemonWorkFlag();
	bool	isPendingBlocksEmpty();
	
	static void	*staticDaemonRoutine(void* c);
	
	void	*maintainChain(void* c);
	static 	void *hash(void *block_ptr);
	Block	*getRandomDeepest();

	//funcs that blockchain call
	int		initiateBlockchain();
	int		addBlock(char *data, int length);
	int		toLongest(int blockNum);
	int		attachNow(int blockNum);
	int		wasAdded(int blockNum);
	int		chainSize();
	int		pruneChain();
	void	closeChain();
	int		returnOnClose();




	/**
	 * @return true if was initiated
	 */ 
	static	bool initiated(void);
	static	void create();
	static	Chain *getInstance();

private:
	static	bool s_initiated;
	static	Chain *s_instance;

	Chain();
	
	pthread_mutex_t	_usedIDListMutex;
	pthread_mutex_t	_deepestTailsMutex;
	pthread_mutex_t	_blocksInChainMutex;
	pthread_mutex_t	_pendingBlocksMutex;

	pthread_cond_t	_pendingBlocksCV;

	int	_maxHeight;
	int	_size;

	std::unordered_map<unsigned int, Block*> _blocksInChain;
	std::vector<Block*> _allTails;
	std::vector<Block*> _deepestTails;
	std::list<int> _usedIDList;
	std::deque<Block*> _pendingBlocks;

	std::vector<pthread_t*> _workers;

	bool	_daemonWorkFlag;
	bool	_isClosed;

	Block	*_tip;
	Block	*_root;
};