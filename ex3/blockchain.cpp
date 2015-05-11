#include "blockchain.h"
#include "Chain.hpp"
#include <pthred.h>

/*
 *       A multi threaded blockchain database manager
 *       Author: OS, os@cs.huji.ac.il
 */

#define GENESIS_BLOCK_NUM 0

pthread_t daemonThread;

int maintain_chain()
{
	while(true)
	{
		//TODO logic of the deamon thread
	}

}

Block *genesis_Block_creator()
{
	Block genesisBlock = new Block(GENESIS_BLOCK_NUM, NULL, NULL, Chain::getMaxHeight());
	return &genesisBlock;
}

/*
 * DESCRIPTION: This function initiates the Block chain, and creates the genesis Block.  The genesis Block does not hold any transaction data   
 *      or hash.
 *      This function should be called prior to any other functions as a necessary precondition for their success (all other functions should   
 *      return with an error otherwise).
 * RETURN VALUE: On success 0, otherwise -1.
 */
int init_blockchain()
{
	if (Chain::initiated())
	{
		return FAIL;
	}
	Chain::create(); // TODO make sure chain is singelton 
	init_hash_generator();
	pthread_create(&daemonThread, NULL, maintain_chain)	// master thread craeted
	Chain::pushBlock(genesis_Block_creator()); //create genesis block and insert to chain
}

/*
 * DESCRIPTION: Ultimately, the function adds the hash of the data to the Block chain.
 *      Since this is a non-blocking package, your implemented method should return as soon as possible, even before the Block was actually  
 *      attached to the chain.
 *      Furthermore, the father Block should be determined before this function returns. The father Block should be the last Block of the 
 *      current longest chain (arbitrary longest chain if there is more than one).
 *      Notice that once this call returns, the original data may be freed by the caller.
 * RETURN VALUE: On success, the function returns the lowest available block_num (> 0),
 *      which is assigned from now on to this individual piece of data.
 *      On failure, -1 will be returned.
 */
int add_block(char *data , int length)
{
	if (!Chain::initiated())
	{
		return FAIL;
	}

	// finds smallest available ID
	int chainSize = Chain::getSize();



	// TODO: searches for father
	Block *father = Chain::get

	// TODO: enetering the deamon list - the data and the father
	
	// TODO: looks for the lowest number available and returns it:
	// get lowest number from usedID list
	// get size of list - chose the smaller of the two
	int BlockID = Chain::getLowestID();

	return BlockID;
}

/*
 * DESCRIPTION: Without blocking, enforce the policy that this block_num should be attached to the longest chain at the time of attachment of 
 *      the Block. For clearance, this is opposed to the original add_block that adds the Block to the longest chain during the time that 
 *      add_block was called.
 *      The block_num is the assigned value that was previously returned by add_block. 
 * RETURN VALUE: If block_num doesn't exist, return -2; In case of other errors, return -1; In case of success return 0; In case block_num is 
 *      already attached return 1.
*/
int to_longest(int block_num)
{
	//WHAT IF WE HAD INTERRUPT AFTER BLOCK WAS FOUND
	// AND THAN HE CAN"T FIND IT ANYMORE SINCE IT WAS ADDED??

	if (!Chain::initiated())
	{
		return FAIL;
	}

	// check if was added 
	// finds the request in deamon list and changes parameters
	// 

}

/*
 * DESCRIPTION: This function blocks all other Block attachments, until block_num is added to the chain. The block_num is the assigned value 
 *      that was previously returned by add_block.
 * RETURN VALUE: If block_num doesn't exist, return -2;
 *      In case of other errors, return -1; In case of success or if it is already attached return 0.
*/
int attach_now(int block_num)
{
	if (!Chain::initiated())
	{
		return FAIL;
	}
}

/*
 * DESCRIPTION: Without blocking, check whether block_num was added to the chain.
 *      The block_num is the assigned value that was previously returned by add_block.
 * RETURN VALUE: 1 if true and 0 if false. If the block_num doesn't exist, return -2; In case of other errors, return -1.
*/
int was_added(int block_num)
{
	if (!Chain::initiated())
	{
		return FAIL;
	}
}

/*
 * DESCRIPTION: Return how many Blocks were attached to the chain since init_blockchain.
 *      If the chain was closed (by using close_chain) and then initialized (init_blockchain) again this function should return 
 *      the new chain size.
 * RETURN VALUE: On success, the number of Blocks, otherwise -1.
*/
int chain_size()
{
	return Chain::initiated() ? Chain::getSize() : FAIL;
}

/*
 * DESCRIPTION: Search throughout the tree for sub-chains that are not the longest chain,
 *      detach them from the tree, free the blocks, and reuse the block_nums.
 * RETURN VALUE: On success 0, otherwise -1.
*/
int prune_chain()
{
	if (!Chain::initiated())
	{
		return FAIL;
	}
}

/*
 * DESCRIPTION: Close the recent blockchain and reset the system, so that it is possible to call init_blockchain again. Non-blocking.
 *      All pending Blocks should be hashed and printed to terminal (stdout).
 *      Calls to library methods which try to alter the state of the Blockchain are prohibited while closing the Blockchain. e.g.: Calling   
 *      chain_size() is ok, a call to prune_chain() should fail.
 *      In case of a system error, the function should cause the process to exit.
*/
void close_chain()
{
	if (!Chain::initiated())
	{
	return;
	}
}

/*
 * DESCRIPTION: The function blocks and waits for close_chain to finish.
 * RETURN VALUE: If closing was successful, it returns 0.
 *      If close_chain was not called it should return -2. In case of other error, it should return -1.
*/

int return_on_close()
{
	if (!Chain::initiated())
	{
		return FAIL;
	}
}