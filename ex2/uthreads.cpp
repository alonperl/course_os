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
#include "StatesManager.hpp"
#include "SignalManager.hpp"

#define CONTINUING 1
#define MAIN 0

#define LIBERR "thread library error: "

#define LIBERR_INVALID_QUANTUM ": invalid quantum\n"
#define LIBERR_INIT_CALLED ": cannot call init more then once\n"
#define LIBERR_MAX_THREAD_NUM ": maximum threads reached\n"
#define LIBERR_INVALID_TID ": no such thread\n"
#define LIBERR_THREAD_CREATION_FAILED ": cannot create thread object\n"
#define LIBERR_SUSPEND_ONLY_THREAD ": cannot suspend main thread\n"


StatesManager *statesManager;

using namespace std;

void f (void)
{
	int i = 1;
	while(1)
	{
		if (i == uthread_get_quantums(uthread_get_tid()))
		{
			cout << "red1" << endl;
			i++;
		}
		if (i == 4)
		{
			cout << "     red1 suspend" << endl;
			uthread_suspend(1);
		}
		if (i == 8)
		{
			cout << "     exit" << endl;
			uthread_terminate(0);
		}
	}
}

void g (void)
{
	int i = 1;
	while(1)
	{
		if (i == uthread_get_quantums(uthread_get_tid()))
		{
			cout << "red2" << endl;
			i++;
		}
		if (i == 8)
		{
			cout << "     red2 suspend" << endl;
			uthread_suspend(2);
		}
	}
}

void h (void)
{
	int i = 1;
	while(1)
	{
		if (i == uthread_get_quantums(uthread_get_tid()))
		{
			cout << "orange" << endl;
			i++;
		}
		if (i == 12)
		{
			cout << "     orange suspend" << endl;
			uthread_suspend(3);
		}
	}
}

void i (void)
{
	int i = 1;
	while(1)
	{
		if (i == uthread_get_quantums(uthread_get_tid()))
		{
			cout << "green1" << endl;
			i++;
		}
		if (i == 4)
		{
			cout << "     green1 suspend" << endl;
			uthread_suspend(4);
		}
	}
}

void j (void)
{
	int i = 1;
	while(1)
	{
		if (i == uthread_get_quantums(uthread_get_tid()))
		{
			cout << "green2" << endl;
			i++;
		}
		if (i == 8)
		{
			cout << "     green2 suspend" << endl;
			uthread_suspend(5);
		}
	}
}

int main(void)
{
	if (uthread_init(10) == -1)
	{
		return 0;
	}

	uthread_spawn(f,RED);
	uthread_spawn(g,RED);
	uthread_spawn(h,ORANGE);
	uthread_spawn(i,GREEN);
	uthread_spawn(j,GREEN);


	int i = 1;
	int j = 0;


	while(1)
	{
		if (i == uthread_get_quantums(uthread_get_tid()))
		{
			i++;
			cout << "main" <<  endl;
		}
		if (i == 30 && j == 0)
		{
			cout << "     resume green2" << endl;
			uthread_resume(5);
			j++;
		}
		if (i == 34 && j == 1)
		{
			cout << "     resume green1" << endl;
			uthread_resume(4);
			j++;
		}
		if (i == 38 && j == 2)
		{
			cout << "     resume orange" << endl;
			uthread_resume(3);
			j++;
		}
		if (i == 41 && j == 3)
		{
			cout << "     resume red2" << endl;
			uthread_resume(2);
			j++;
		}
		if (i == 44 && j == 4)
		{
			cout << "     resume red1" << endl;
			uthread_resume(1);
			j++;
		}
	}

	uthread_terminate(0);
	return 0;
}


int uthread_init(int quantum_usecs)
{
	if (quantum_usecs <= 0)
	{
		cerr << LIBERR << __FUNCTION__ << LIBERR_INVALID_QUANTUM;
		return FAIL;
	}

	statesManager = StatesManager::getInstance(quantum_usecs);

	// If init was called before, statesManager will contain at least main thread
	if (statesManager->getTotalThreadsNum() > 0)
	{
		cerr << LIBERR << __FUNCTION__ << LIBERR_INIT_CALLED;
		return FAIL;
	}

	uthread_spawn(NULL, ORANGE);
	statesManager->threadsMap[MAIN]->incrementQuantums();
	statesManager->incrementTotalQuantums();

	statesManager->runNext();

	signal(SIGVTALRM, SignalManager::staticSignalHandler);
	setitimer(ITIMER_VIRTUAL, statesManager->getQuantum(), NULL);

	return 0;
}

/* Create a new thread whose entry point is f */
int uthread_spawn(void (*f)(void), Priority pr)
{
	SignalManager::postponeSignals();

	if (statesManager->getTotalThreadsNum() >= MAX_THREAD_NUM)
	{
		cerr << LIBERR << __FUNCTION__ << LIBERR_MAX_THREAD_NUM;
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
		cerr << LIBERR << __FUNCTION__ << LIBERR_THREAD_CREATION_FAILED;
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
		cerr << LIBERR << __FUNCTION__ << LIBERR_INVALID_TID;
		SignalManager::unblockSignals();
		return FAIL;
	}

	// Terminating main
	if (tid == 0)
	{
		std::map<unsigned int, Thread*>::iterator threadsIterator = statesManager->threadsMap.begin();
		for (; threadsIterator != statesManager->threadsMap.end(); ++threadsIterator)
		{
			if (threadsIterator->first != 0)
			{
				delete threadsIterator->second;
			}
		}

		exit(0);
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

		default:
			break;
	}

	statesManager->threadsMap.erase(thread->getTid());
	statesManager->terminatedTids.push(thread->getTid());

	delete thread;

	statesManager->decrementTotalThreadsNum();

	// If the quantum has ended till now and thread did not kill itself
	// , switch threads now.
	if (SignalManager::hasTimerSignalTriggered() && !selfDestroy)
	{
		statesManager->switchThreads(READY);
	}

	if (selfDestroy)
	{
		statesManager->running->incrementQuantums();
		statesManager->incrementTotalQuantums();

		// Terminated last running thread, must switch to next
		// TODO after f ends, g gets into running, but does not work (g q = 8)
		siglongjmp(*(statesManager->running->getEnv()), 1);
	}

	return 0;
}

/* Suspend a thread */
int uthread_suspend(int tid)
{
	SignalManager::postponeSignals();

	// If got invalid tid or if there if only one existing thread, cannot suspend
	if (!statesManager->isValidTid(tid))
	{
		cerr << LIBERR << __FUNCTION__ << LIBERR_INVALID_TID;
		return FAIL;
	}

	if (statesManager->getTotalThreadsNum() == 1)
	{
		cerr << LIBERR << __FUNCTION__ << LIBERR_SUSPEND_ONLY_THREAD;
		return FAIL;
	}

	Thread *thread = statesManager->getThread(tid);

	if (thread->getState() != BLOCKED)
	{
		// printf("%d suspending %d\n", uthread_get_tid(), tid);
		if (thread->getState() == RUNNING)
		{
			// Get next ready thread and set it as current
			statesManager->switchThreads(BLOCKED);
			/*statesManager->runNext();

			SignalManager::unblockSignals();
			siglongjmp(*(statesManager->running->getEnv()), CONTINUING);*/
		}
		else
		{
			statesManager->block(thread);
		}
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
		cerr << LIBERR << __FUNCTION__ << LIBERR_INVALID_TID;
		return FAIL;
	}

	Thread *thread = statesManager->getThread(tid);

	// If the thread is not blocked, do nothing.
	if (thread->getState() == BLOCKED)
	{
		// printf("%d resumed %d\n", uthread_get_tid(), tid);
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
	if (!statesManager->isValidTid(tid))
	{
		cerr << LIBERR << __FUNCTION__ << LIBERR_INVALID_TID;
		return FAIL;
	}
	
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
