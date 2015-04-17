#ifndef _THREAD_H
#define _THREAD_H

#include <cstdlib>
#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include <sys/time.h>
#ifndef _UTHREADS_H
#include "uthreads.h"
#endif

#define SUCCESS 0;
#define FAIL -1;

typedef enum State { 
	READY, 
	RUNNING, 
	BLOCKED, 
	NEW 
} State;

typedef unsigned long address_t;

class Thread
{
public:
	Thread(void (*f)(void), Priority pr, unsigned int newTid);
	~Thread();
	
	unsigned int getTid();

	address_t getProgramCounter();
	Priority getPriority();
	
	State getState();
	unsigned int getQuantums();
	struct timeval getReadyFrom();
	sigjmp_buf *getEnv();

	void setReadyFrom();
	void resetReadyFrom();
	void setState(State st);

	void incrementQuantums();

private:
	sigjmp_buf threadEnv;
	char stack[STACK_SIZE];
	address_t programCounter;
	Priority priority;
	State state;

	struct timeval readyFrom;
	unsigned int quantums;
	unsigned int tid;

	bool _isValidPriority(Priority pr);
};

#endif