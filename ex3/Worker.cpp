#include "Worker.h"
#include "Chain.hpp"

#define HASH_LENGTH 128

Worker::Worker(AddRequest *pRequest) {
    pthread_mutex_init(&_toLongestFlagMutex, NULL);
    _toLongestFlag = false;
    finished = NOT_FINISHED;
    blockFather = (void*)pRequest->father;
    req = pRequest;
}

/**
 * @brief Create thread for hash calculation of this worker's
 *        request, wait for it to finish, check if the father was
 *        changed or pruned, recalculate if needed
 */
void Worker::act()
{
    do
    {
        if (_toLongestFlag)
        {
            blockFather = Chain::getInstance()->getRandomDeepest();
            _toLongestFlag = false; // TODO What if someone calls to_longest twice
        }

        blockHash = hash(req);
    } while (_toLongestFlag);

    // _toLongestFlag was false till now, so from now on toLongest(this) will not act on this block
    pthread_mutex_lock(&_toLongestFlagMutex);
    // Create block
    Block* newBlock = new Block(blockNum, (char*)blockHash, HASH_LENGTH,
                               ((Block*)blockFather)->getHeight()+1, (Block*)blockFather);

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
char* Worker::hash(void *pRequest)
{
    AddRequest *req = (AddRequest*) pRequest;

    // TODO rehashing here too?
    // Save father id
    int originalFatherId = ((Block*)req->father)->getId();
    int fatherId;

    char* calculatedHash;
    // Calculate hash
    do
    {
        int nonce = generate_nonce(req->blockNum, ((Block*)req->father)->getId());
        calculatedHash = generate_hash(req->data, (size_t)req->dataLength, nonce);

        // If the father was changed meanwhile, update it and recalculate the hash
        fatherId = ((Block*)req->father)->getId();
    } while (originalFatherId != fatherId);

    return calculatedHash;
}