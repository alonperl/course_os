#include "Worker.h"
#include "Chain.hpp"

#define HASH_LENGTH 128

Worker::Worker(AddRequest *pRequest)
{
    pthread_mutex_init(&_toLongestFlagMutex, NULL);
    _toLongestFlag = false;
    blockFather = pRequest->father;
//    finished = NOT_FINISHED;
    req = pRequest;
}

Worker::~Worker()
{
    delete req;
    free(blockHash);
    blockFather = NULL;
}

/**
 * @brief Create thread for hash calculation of this worker's
 *        request, wait for it to finish, check if the father was
 *        changed or pruned, recalculate if needed
 */
void Worker::act()
{
    Block* cachedFather(blockFather);
    // Save if current father is longest or not
    bool cachedLongest = cachedFather.get()->getHeight() == Chain::getInstance()->getMaxHeight();
    bool rehash = false;

    do
    {
        blockHash = hash(req);

        /* If to_longest was called for this block while hashing
         * or all other shared_ptrs to father's block were reset
         * this means we need to find new father and rehash. */
        // TODO maybe not unique but ==2 (cachedFather and blockFather)
        if ((_toLongestFlag && !cachedLongest) || cachedFather.unique())
        {
            rehash = true;
            cachedFather = Chain::getInstance()->getRandomDeepest();
            blockFather = cachedFather;
            cachedLongest = cachedFather.get()->getHeight() == Chain::getInstance()->getMaxHeight();
        }
    } while (rehash);

    // _toLongestFlag was false till now, so from now on toLongest(this) will not act on this block
    pthread_mutex_lock(&_toLongestFlagMutex);
    // Create block
    Block* newBlock(new Block(blockNum, blockHash, HASH_LENGTH,
                               blockFather->getHeight()+1, blockFather));

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
        calculatedHash = generate_hash(req->data, (size_t)req->dataLength, nonce);

        // If the father was changed meanwhile, update it and recalculate the hash
        fatherId = req->father->getId();
    } while (originalFatherId != fatherId);

    return calculatedHash;
}