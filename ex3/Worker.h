#ifndef _WORKER_H
#define _WORKER_H

#include <pthread.h>
#include <memory>
#include "hash.h"
#include "AddRequest.hpp"
#include "Block.hpp"

#define NOT_FINISHED -1;
#define FINISHED 0;

class Worker {
public:
    Worker(AddRequest *pRequest);
    ~Worker();
    static char* hash(const AddRequest *pRequest);
    void act();
//    int finished;

    const AddRequest* req;
    int blockNum;
    char* blockHash;
    std::shared_ptr<Block> blockFather;
    bool _toLongestFlag;
private:
//    pthread_t _worker;
    pthread_mutex_t _toLongestFlagMutex;
};


#endif
