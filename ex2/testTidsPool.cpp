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
	
	

	printf("Terminate 1\n");
	uthread_terminate(1);
	

	printf("New thread %d\n", uthread_spawn(NULL, ORANGE));
	
	
	printf("Terminate 2\n");
	uthread_terminate(2);
	
	
	printf("New thread %d\n", uthread_spawn(NULL, ORANGE));
	
	
	printf("Terminate 3\n");
	uthread_terminate(3);
	
	
	printf("New thread %d\n", uthread_spawn(NULL, ORANGE));
	

	printf("Terminate 1\n");
	uthread_terminate(1);
	
	
	printf("Terminate 2\n");
	uthread_terminate(2);
	
	
	printf("Terminate 3\n");
	uthread_terminate(3);
	
	
	printf("New thread %d\n", uthread_spawn(NULL, ORANGE));
	
	
	printf("New thread %d\n", uthread_spawn(NULL, ORANGE));
	
	
	printf("New thread %d\n", uthread_spawn(NULL, ORANGE));
	

	printf("New thread %d\n", uthread_spawn(NULL, ORANGE));
	

	std::map<unsigned int, Thread*>::iterator it = Scheduler::getInstance()->getThreadsMap()->begin();
	for (; it != Scheduler::getInstance()->getThreadsMap()->end(); it++)
	{
		printf("%d in map\n", it->second->getTid());
	}
	uthread_terminate(0);
}