/**
 * @file blockchain.cpp
 * @author  griffonn ednussi
 * @version 1.0
 * @date 18 May 2015
 * 
 * @brief Implementation of blockchain library function.
 *
 * @section LICENSE
 * This program is a free software. You can freely redistribute it.
 *
 * @section DESCRIPTION
 * Blockchain is a data structure that stores hashed data in a form of 
 * backlinked tree. Implemented as a singleton, this is the requirements
 * for exercise library.
 */
#include "blockchain.h"
#include "Chain.hpp"

/**
 * @brief This function initiates the Block chain, and creates the genesis Block.
 * 		  The genesis Block does not hold any transaction data   
 * 		  or hash. This function should be called prior to any other functions as
 *		  a necessary precondition for their success.
 *
 * @return On success 0, otherwise -1.
 */
int init_blockchain()
{
	return Chain::initChain();
}

/**
 * @brief Ultimately, the function adds the hash of the data to the Block chain.
 * 		  The father Block should be determined before this function returns. The
 *		  father Block should be the last Block of the 
 * 		  current longest chain (arbitrary longest chain if there is more than one).
 *
 * @param data is the data to hash
 * @param length is the length of the data
 *
 * @return On success, the function returns the lowest available block_num (> 0),
 * 		   which is assigned from now on to this individual piece of data.
 * 		   On failure, -1 will be returned.
 */
int add_block(char *data , int length)
{
	return Chain::getInstance()->addRequest(data, length);
}

/**
 * @brief Without blocking, enforce the policy that this block_num should be 
 *		  attached to the longest chain at the time of attachment of 
 *        the Block. 
 *
 * @param The block_num is the assigned value that was previously returned by 
 *		  add_block.
 *
 * @return If block_num doesn't exist, return -2.
 *		   In case of other errors, return -1.
 *		   In case of success return 0.
 *		   In case block_num is already attached return 1.
 */
int to_longest(int block_num)
{
	return Chain::getInstance()->toLongest(block_num);
}

/**
 * @brief This function blocks all other Block attachments, until block_num is 
 *		  added to the chain. 
 *
 * @param block_num is the assigned value that was previously returned by add_block.
 *
 * @return If block_num doesn't exist, return -2;
 *         In case of other errors, return -1; In case of success or if it is 
 *		   already attached return 0.
 */
int attach_now(int block_num)
{
	return Chain::getInstance()->attachNow(block_num);
}

/**
 * @brief Without blocking, check whether block_num was added to the chain.
 *
 * @param The block_num is the assigned value that was previously returned by 
 *		  add_block.
 *
 * @return 1 if true and 0 if false. If the block_num doesn't exist, return -2; 
 *		   In case of other errors, return -1.
 */
int was_added(int block_num)
{
	return Chain::getInstance()->wasAdded(block_num);
}

/**
 * @brief Return how many Blocks were attached to the chain since init_blockchain.
 * @return On success, the number of Blocks, otherwise -1.
 */
int chain_size()
{
	return Chain::getInstance()->chainSize();
}

/**
 * @brief Search throughout the tree for sub-chains that are not the longest chain,
 *        detach them from the tree, free the blocks, and reuse the block_nums.
 * @return On success 0, otherwise -1.
 */
int prune_chain()
{
	return Chain::getInstance()->pruneChain();
}

/**
 * @brief Close the recent blockchain and reset the system, so that it is possible
 *		  to call init_blockchain again.
 *        All pending Blocks are hashed and printed to terminal.
 *        Calls to library methods which try to alter the state of the Blockchain
 *		  are prohibited while closing the Blockchain.
 *        In case of a system error, the function causes the process to exit.
 */
void close_chain()
{
	return Chain::getInstance()->closeChain();
}

/**
 * @brief The function blocks and waits for close_chain to finish.
 * @return If closing was successful, it returns 0.
 *         If close_chain was not called it should return -2. 
 *         In case of other error, it should return -1.
 */
int return_on_close()
{
	return Chain::getInstance()->returnOnClose();
}