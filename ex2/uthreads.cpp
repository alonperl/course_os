#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>
#include <deque>
#include <list>
#include <assert.h>
#include <iostream>

#include "uthreads.h"
#include "statesManager.hpp"
#include "signalManager.hpp"

#define CONTINUING 1
#define MAIN 0

StatesManager *statesManager;
using namespace std;
void work()
{
	int i = 0;
	// for(i = 0; i < 10000; i++)
	for(;; i++)
	{
		if (i == 0)
		{
			printf("Thread %d starts its work.\n", uthread_get_tid());
		}
		if (i % 1000 == 0)
		{
			printf(".", i);
		}
	 	 if (i == 1000000000)
	 	 {
	 	 	printf("Thread %d finished.\n", uthread_get_tid());
	 	 	uthread_terminate(uthread_get_tid());
	 	 }
	}
}

/*int main(int argc, char const *argv[])
{
	printf("Entering main\n");
	uthread_init(10000);
	printf("Inited\n");

	int i;
	for (i = 0; i < 30; ++i)
	{
		uthread_spawn(work, ORANGE);
	}

	for (i = 1; i <= 30; ++i)
	{
		printf("suspending %d from main\n", i);
		uthread_suspend(i);
	}

	Thread *t;
	std::map<unsigned int, Thread*>::iterator threadsIterator = statesManager->threadsMap.begin();
	for (; threadsIterator != statesManager->threadsMap.end(); ++threadsIterator)
	{
		if (threadsIterator->first != 0)// threadsIterator->second->getState() == BLOCKED)
		{
			printf("Tid %d\tState %d\n", threadsIterator->second->getTid(), threadsIterator->second->getState());
		}
	}

	for (i = 1; i <= 30; ++i)
	{
		printf("resuming %d from main\n", i);
		uthread_resume(i);
	}

	printf("Finished main\n");
	uthread_terminate(MAIN);

	return 0;
}*/

void f (void)
{
	int i = 1;
	int j = 0;
	while(1)
	{
		if (i == uthread_get_quantums(uthread_get_tid()))
		{
			cout << "f" << "  q:  " << i << endl;
			if (i == 3 && j == 0)
			{
				j++;
				cout << "          f suspend by f" << endl;
				uthread_suspend(uthread_get_tid());
			}
			if (i == 6 && j == 1)
			{
				j++;
				cout << "          g resume by f" << endl;
				uthread_resume(2);
			}
			if (i == 8 && j == 2)
			{
				j++;
				cout << "          **f end**" << endl;
				uthread_terminate(uthread_get_tid());
				return;
			}
			i++;
		}
	}
}

void g (void)
{
	int i = 1;
	int j = 0;
	while(1)
	{
		// printf("i: %d, gq: %d\n", i, uthread_get_quantums(uthread_get_tid()));
		if (i == uthread_get_quantums(uthread_get_tid()))
		{
			cout << "g" << "  q:  " << i << endl;
			if (i == 11 && j == 0)
			{
				j++;
				cout << "          **g end**" << endl;
				uthread_terminate(uthread_get_tid());
				return;
			}
			i++;
		}
	}
}

int main(void)
{
	if (uthread_init(100) == -1)
	{
		return 0;
	}

	int i = 1;
	int j = 0;
	while(1)
	{
		printf(".");
		if (i == uthread_get_quantums(uthread_get_tid()))
		{
			cout << "m" << "  q:  " << i << endl;
			if (i == 3 && j == 0)
			{
				j++;
				cout << "          spawn f at (1) " << uthread_spawn(f, RED) << endl;
				cout << "          spawn g at (2) " << uthread_spawn(g, RED) << endl;
			}
			if (i == 6 && j == 1)
			{
				j++;
				cout << "          g suspend by main" << endl;
				uthread_suspend(2);
				cout << "          g suspend again by main" << endl;
				uthread_suspend(2);
			}
			if (i == 9 && j == 2)
			{
				j++;
				cout << "          f resume by main" << endl;
				uthread_resume(1);
				cout << "          f resume again by main" << endl;
				uthread_resume(1);
			}
			if (i == 13 && j == 3)
			{
				j++;
				cout << "          spawn f at (1) " << uthread_spawn(f, RED) << endl;
				cout << "          f suspend by main" << endl;
				uthread_suspend(1);
			}
			if (i == 17 && j == 4)
			{
				j++;
				cout << "          spawn g at (2) " << uthread_spawn(g, RED) << endl;
				cout << "          f terminate by main" << endl;
				uthread_terminate(1);
				cout << "          spawn f at (1) " << uthread_spawn(f, RED) << endl;
				cout << "          f suspend by main" << endl;
				uthread_suspend(1);
			}
			if (i == 20 && j == 5)
			{
				j++;
				cout << "          ******end******" << endl;
				cout << "total quantums:  " << uthread_get_total_quantums() << endl;
				uthread_terminate(0);
				return 0;
			}
			i++;
		}
	}
	cout << "end" << endl;
	return 0;
}

int uthread_init(int quantum_usecs)
{
	statesManager = StatesManager::getInstance();

	// If init was called before, statesManager will contain at least main thread
	if (statesManager->getTotalThreadsNum() > 0)
	{
		return FAIL;
	}

	statesManager->setQuantum(quantum_usecs);

	uthread_spawn(NULL, ORANGE);

	statesManager->runNext();

	signal(SIGVTALRM, StatesManager::staticSignalHandler);
	setitimer(ITIMER_VIRTUAL, statesManager->getQuantum(), NULL);

	return 0;
}

/* Create a new thread whose entry point is f */
int uthread_spawn(void (*f)(void), Priority pr)
{
	SignalManager::postponeSignals();

	if (statesManager->getTotalThreadsNum() >= MAX_THREAD_NUM)
	{
		return FAIL;
	}
	
	Thread *thread;
	unsigned int newTid = statesManager->getMinTid();

	try
	{
		thread = new Thread(f, pr, newTid);
	}
	catch (int e)
	{
		return FAIL;
	}

	if (thread != NULL)
	{
		statesManager->ready(thread);
		statesManager->threadsMap[newTid] = thread;
	}

	statesManager->incrementTotalThreadsNum();

	// Set handler back
	SignalManager::unblockSignals();

	// If the quantum has ended till now, switch threads now.
	if (SignalManager::hasTimerSignalTriggered())
	{
		statesManager->switchThreads(READY);
	}

	return newTid;
}

/* Terminate a thread */
int uthread_terminate(int tid)
{
	SignalManager::postponeSignals();

	if (!statesManager->isValidTid(tid))
	{
		SignalManager::unblockSignals();
		return FAIL;
	}
	if (tid == 0)
	{
		int totalThreadNum = statesManager->getTotalThreadsNum();
		unsigned int existingThreads[totalThreadNum - 1];
		int counter = 0;

		std::map<unsigned int, Thread*>::iterator threadsIterator = statesManager->threadsMap.begin();
		for (; threadsIterator != statesManager->threadsMap.end(); ++threadsIterator)
		{
			if (threadsIterator->first != 0)
			{
				existingThreads[counter++] = threadsIterator->first;
			}
		}

		counter = 0;
		for (; counter  < totalThreadNum - 1; counter ++)
		{
			uthread_terminate(existingThreads[counter]);
		}
	}

	Thread *thread = statesManager->getThread(tid);

	bool selfDestroy = false;

	switch(thread->getState())
	{
		case READY:
			// Remove from Ready queue
			statesManager->readyQueue.erase(thread);
			break;

		case RUNNING:
			// Stop current thread and run next ready thread
			selfDestroy = true;
			statesManager->runNext();
			break;

		case BLOCKED:
			// Remove from blocked
			statesManager->blockedMap.erase(tid);
			break;
	}

	statesManager->threadsMap.erase(thread->getTid());
	statesManager->terminatedTids.push(thread->getTid());

	delete thread;

	statesManager->decrementTotalThreadsNum();

	// If it is the main thread, exit
	if (tid == 0)
	{
		exit(0);
	}

	// If the quantum has ended till now and thread did not kill itself
	// , switch threads now.
	if (SignalManager::hasTimerSignalTriggered() && !selfDestroy)
	{
		statesManager->switchThreads(READY);
	}

	if (selfDestroy)
	{
		// Terminated last running thread, must switch to next
		// TODO after f ends, g gets into running, but does not work (g q = 8)
		// Set handler back
		SignalManager::unblockSignals();
		siglongjmp(*(statesManager->running->getEnv()), 1);
	}

	return 0;
}

/* Suspend a thread */
int uthread_suspend(int tid)
{
	SignalManager::postponeSignals();

	// If got invalid tid or if there if only one existing thread, cannot suspend
	if (!statesManager->isValidTid(tid) || statesManager->getTotalThreadsNum() == 1)
	{
		return FAIL;
	}

	Thread *thread = statesManager->getThread(tid);

	if (thread->getState() != BLOCKED)
	{
		if (thread->getState() == RUNNING)
		{
			// Get next ready thread and set it as current
			statesManager->switchThreads(BLOCKED);
			/*statesManager->runNext();

			SignalManager::unblockSignals();
			siglongjmp(*(statesManager->running->getEnv()), CONTINUING);*/
		}

		statesManager->block(thread);
	}

	// Set handler back
	SignalManager::unblockSignals();

	// If the quantum has ended till now, switch threads now.
	if (SignalManager::hasTimerSignalTriggered())
	{
		statesManager->switchThreads(READY);
	}

	return 0;
}

/* Resume a thread */
int uthread_resume(int tid)
{
	SignalManager::postponeSignals();

	if (!statesManager->isValidTid(tid))
	{
		return FAIL;
	}

	Thread *thread = statesManager->getThread(tid);

	// If the thread is not blocked, do nothing.
	if (thread->getState() == BLOCKED)
	{
		statesManager->ready(thread);
		statesManager->blockedMap.erase(tid);
	}

	thread = NULL;

	// Set handler back
	SignalManager::unblockSignals();

	// If the quantum has ended till now, switch threads now.
	if (SignalManager::hasTimerSignalTriggered())
	{
		statesManager->switchThreads(READY);
	}

	return 0;
}


/* Get the id of the calling thread */
int uthread_get_tid()
{
	// SignalManager::postponeSignals();
	return statesManager->running->getTid();
	// SignalManager::unblockSignals();
	// if (SignalManager::hasTimerSignalTriggered())
	// {
	// 	statesManager->switchThreads(READY);
	// }
}

/* Get the total number of library quantums */
int uthread_get_total_quantums()
{
	return statesManager->getTotalQuantums();
}

/* Get the number of thread quantums */
int uthread_get_quantums(int tid)
{
	// SignalManager::postponeSignals();
	return statesManager->getThread(tid)->getQuantums();
	// SignalManager::unblockSignals();
	// if (SignalManager::hasTimerSignalTriggered())
	// {
	// 	statesManager->switchThreads(READY);
	// }
}

unsigned int getMinTid()
{
	return statesManager->getMinTid();
}
