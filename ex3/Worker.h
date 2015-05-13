#ifndef COURSE_OS_WORKER_H
#define COURSE_OS_WORKER_H

#include <pthread.h>
#include "AddRequest.hpp"

#define NOT_FINISHED -1;
#define FINISHED 0;

class Worker {
public:
    Worker(AddRequest *pRequest);
    int finished;

    int blockNum;
    void* blockHash;
    void* blockFather;
private:
    pthread_t _worker;
};


#endif //COURSE_OS_WORKER_H
