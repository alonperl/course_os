#ifndef _WORKER_H
#define _WORKER_H

#include <pthread.h>
#include <hash.h>
#include "AddRequest.hpp"

#define NOT_FINISHED -1;
#define FINISHED 0;

class Worker {
public:
    Worker(AddRequest *pRequest);
    static char* hash(void *pRequest);
    void act();
    int finished;

    AddRequest* req;
    int blockNum;
    void* blockHash;
    void* blockFather;
    bool _toLongestFlag;
private:
//    pthread_t _worker;
    pthread_mutex_t _toLongestFlagMutex;
};


#endif
