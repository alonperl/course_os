#include "Worker.h"
#include "Chain.hpp"
#include <iostream>

#define HASH_LENGTH 128

Worker::Worker(AddRequest *pRequest)
{
    pthread_mutex_init(&_toLongestFlagMutex, NULL);
    _toLongestFlag = false;
    blockNum = pRequest->blockNum;
//    finished = NOT_FINISHED;
    req = pRequest;
}

Worker::~Worker()
{
    delete req;
    // free(blockHash);
}

/**
 * @brief Create thread for hash calculation of this worker's
 *        request, wait for it to finish, check if the father was
 *        changed or pruned, recalculate if needed
 */
void Worker::act()
{
    Block* cachedFather = req->father;
    // Save if current father is longest or not
    bool cachedLongest = cachedFather->getHeight() == Chain::getInstance()->getMaxHeight();
    bool rehash = false;

    do
    {
        blockHash = hash(req);

        /* If to_longest was called for this block while hashing
         * or all other shared_ptrs to father's block were reset
         * this means we need to find new father and rehash. */
        // TODO maybe not unique but ==2 (cachedFather and blockFather)
        if ((_toLongestFlag && !cachedLongest) || cachedFather == NULL)
        {
            std::cout << "TLF: " << _toLongestFlag << ", CachedLongest: " << cachedLongest << "\n";
            rehash = true;
            cachedFather = Chain::getInstance()->getRandomDeepest();
            cachedLongest = cachedFather->getHeight() == Chain::getInstance()->getMaxHeight();
        }
        else
        {
            rehash = false;
        }
        std::cout<<"Rehash? "<<rehash<<"\n";
    } while (rehash);

    // TODO lock more
    // _toLongestFlag was false till now, so from now on toLongest(this) will not act on this block
    pthread_mutex_lock(&_toLongestFlagMutex);
    // Create block
    Block* newBlock = new Block(req->blockNum, blockHash, HASH_LENGTH,
                               req->father->getHeight()+1, req->father);

    // Attach block to chain
    Chain::getInstance()->pushBlock(newBlock);
    pthread_mutex_unlock(&_toLongestFlagMutex);
    // Self-destroy
    delete this;
}

/**
 * @brief Hashing routine
 * @details This function is passed as start_routine of workers.
 *          Its job is to calculate the hash of given block (and
 *          possibly rehash if its father was changed)
 *
 * @param block_ptr Desired block
 * @return NULL
 */
char* Worker::hash(const AddRequest *pRequest)
{
    AddRequest *req = (AddRequest*) pRequest;

    // TODO rehashing here too?
    // Save father id
    int originalFatherId = req->father->getId();
    int fatherId;

    char* calculatedHash;
    // Calculate hash
    do
    {
        int nonce = generate_nonce(req->blockNum, req->father->getId());
        // calculatedHash = generate_hash(req->data, (size_t)req->dataLength, nonce);
        calculatedHash = "a";
        // If the father was changed meanwhile, update it and recalculate the hash
        fatherId = req->father->getId();
        std::cout<<"A\n";
    } while (originalFatherId != fatherId);

    return calculatedHash;
}