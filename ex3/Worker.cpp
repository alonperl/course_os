//
// Created by griffonn on 14/05/2015.
//

#include "Worker.h"
#include "Chain.hpp"

void *hash(void *pRequest);

Worker::Worker(AddRequest * pRequest) {
    finished = NOT_FINISHED;
    blockFather = (void*)pRequest->father;
    pthread_create(&_worker, NULL, hash, pRequest);	// master thread created
    finished = pthread_join(_worker, &blockHash);
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
void *hash(void *pRequest)
{
    AddRequest *req = (AddRequest*) pRequest;
    int id = req->blockNum;

    // Save father id
    int originalFatherId = ((Block*)req->father)->getId();
    int fatherId;

    void* calculatedHash;
    // Calculate hash
    do
    {
        int nonce = generate_nonce(req->blockNum, ((Block*)req->father)->getId());
        calculatedHash = (void*)generate_hash(req->data, (size_t)req->dataLength, nonce);

        // If the father was changed meanwhile, update it and recalculate the hash
        fatherId = ((Block*)req->father)->getId();
    } while (originalFatherId != fatherId);

    return calculatedHash;
}