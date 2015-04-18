#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>

#include <cstdlib>
#include <list>
#include <iostream>

#include "uthreads.h"
#include "Thread.hpp"
#include "Scheduler.hpp"
#include "SignalManager.hpp"

int main(int argc, char const *argv[])
{
	uthread_init(100);
	
	printf("New thread %d\n", uthread_spawn(NULL, ORANGE));
	printf("New thread %d\n", uthread_spawn(NULL, ORANGE));
	printf("New thread %d\n", uthread_spawn(NULL, ORANGE));
	
	printf("Next free tid %d\n", Scheduler::getInstance()->getMinTid());

	uthread_terminate(1);
	printf("Next free tid %d\n", Scheduler::getInstance()->getMinTid());

	printf("New thread %d\n", uthread_spawn(NULL, ORANGE));
	printf("Next free tid %d\n", Scheduler::getInstance()->getMinTid());
	
	uthread_terminate(2);
	printf("Next free tid %d\n", Scheduler::getInstance()->getMinTid());
	
	printf("New thread %d\n", uthread_spawn(NULL, ORANGE));
	printf("Next free tid %d\n", Scheduler::getInstance()->getMinTid());
	
	uthread_terminate(3);
	printf("Next free tid %d\n", Scheduler::getInstance()->getMinTid());
	
	printf("New thread %d\n", uthread_spawn(NULL, ORANGE));
	printf("Next free tid %d\n", Scheduler::getInstance()->getMinTid());

	uthread_terminate(1);
	printf("Next free tid %d\n", Scheduler::getInstance()->getMinTid());
	
	uthread_terminate(2);
	printf("Next free tid %d\n", Scheduler::getInstance()->getMinTid());
	
	uthread_terminate(3);
	printf("Next free tid %d\n", Scheduler::getInstance()->getMinTid());
	
	printf("New thread %d\n", uthread_spawn(NULL, ORANGE));
	printf("Next free tid %d\n", Scheduler::getInstance()->getMinTid());
	
	printf("New thread %d\n", uthread_spawn(NULL, ORANGE));
	printf("Next free tid %d\n", Scheduler::getInstance()->getMinTid());
	
	printf("New thread %d\n", uthread_spawn(NULL, ORANGE));
	printf("Next free tid %d\n", Scheduler::getInstance()->getMinTid());

	printf("New thread %d\n", uthread_spawn(NULL, ORANGE));
	printf("Next free tid %d\n", Scheduler::getInstance()->getMinTid());

	std::map<unsigned int, Thread*>::iterator it = Scheduler::getInstance()->getThreadsMap()->begin();
	for (; it != Scheduler::getInstance()->getThreadsMap()->end(); it++)
	{
		printf("%d in mapn", it->second->getTid());
	}
	uthread_terminate(0);
}