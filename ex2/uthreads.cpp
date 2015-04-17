#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>

#include <cstdlib>
#include <list>
#include <iostream>

#include "uthreads.h"
#include "Scheduler.hpp"
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


Scheduler *gScheduler;

int main()
{
	/* code */
	return 0;
}

int uthread_init(int quantum_usecs)
{
	if (quantum_usecs <= 0)
	{
		std::cerr << LIBERR << __FUNCTION__ << LIBERR_INVALID_QUANTUM;
		return FAIL;
	}

	gScheduler = Scheduler::getInstance(quantum_usecs);

	// If init was called before, gScheduler will contain at least main thread
	if (gScheduler->getTotalThreadsNum() > 0)
	{
		std::cerr << LIBERR << __FUNCTION__ << LIBERR_INIT_CALLED;
		return FAIL;
	}

	uthread_spawn(NULL, ORANGE);
	gScheduler->threadsMap[MAIN]->incrementQuantums();
	gScheduler->incrementTotalQuantums();

	gScheduler->runNext();

	signal(SIGVTALRM, SignalManager::staticSignalHandler);
	setitimer(ITIMER_VIRTUAL, gScheduler->getQuantum(), NULL);

	return 0;
}

/* Create a new thread whose entry point is f */
int uthread_spawn(void (*f)(void), Priority pr)
{
	SignalManager::postponeSignals();

	if (gScheduler->getTotalThreadsNum() >= MAX_THREAD_NUM)
	{
		std::cerr << LIBERR << __FUNCTION__ << LIBERR_MAX_THREAD_NUM;
		return FAIL;
	}
	
	Thread *thread;
	unsigned int newTid = gScheduler->getMinTid();

	try
	{
		thread = new Thread(f, pr, newTid);
	}
	catch (int e)
	{
		std::cerr << LIBERR << __FUNCTION__ << LIBERR_THREAD_CREATION_FAILED;
		return FAIL;
	}

	if (thread != NULL)
	{
		gScheduler->ready(thread);
		gScheduler->threadsMap[newTid] = thread;
	}

	gScheduler->incrementTotalThreadsNum();

	// Set handler back
	SignalManager::unblockSignals();

	// If the quantum has ended till now, switch threads now.
	if (SignalManager::hasTimerSignalTriggered())
	{
		gScheduler->switchThreads(READY);
	}

	return newTid;
}

/* Terminate a thread */
int uthread_terminate(int tid)
{
	SignalManager::postponeSignals();

	if (!gScheduler->isValidTid(tid))
	{
		std::cerr << LIBERR << __FUNCTION__ << LIBERR_INVALID_TID;
		SignalManager::unblockSignals();
		return FAIL;
	}

	// Terminating main
	if (tid == 0)
	{
		std::map<unsigned int, Thread*>::iterator threadsIterator = gScheduler->threadsMap.begin();
		for (; threadsIterator != gScheduler->threadsMap.end(); ++threadsIterator)
		{
			if (threadsIterator->first != 0)
			{
				delete threadsIterator->second;
			}
		}

		exit(0);
	}

	Thread *thread = gScheduler->getThread(tid);

	bool selfDestroy = false;

	switch(thread->getState())
	{
		case READY:
			// Remove from Ready queue
			gScheduler->readyQueue.erase(thread);
			break;

		case RUNNING:
			// Stop current thread and run next ready thread
			selfDestroy = true;
			gScheduler->runNext();
			break;

		case BLOCKED:
			// Remove from blocked
			gScheduler->blockedMap.erase(tid);
			break;

		default:
			break;
	}

	gScheduler->threadsMap.erase(thread->getTid());
	gScheduler->terminatedTids.push(thread->getTid());

	delete thread;

	gScheduler->decrementTotalThreadsNum();

	// If the quantum has ended till now and thread did not kill itself
	// , switch threads now.
	if (SignalManager::hasTimerSignalTriggered() && !selfDestroy)
	{
		gScheduler->switchThreads(READY);
	}

	if (selfDestroy)
	{
		gScheduler->running->incrementQuantums();
		gScheduler->incrementTotalQuantums();

		// Terminated last running thread, must switch to next
		// TODO after f ends, g gets into running, but does not work (g q = 8)
		siglongjmp(*(gScheduler->running->getEnv()), 1);
	}

	return 0;
}

/* Suspend a thread */
int uthread_suspend(int tid)
{
	SignalManager::postponeSignals();

	// If got invalid tid or if there if only one existing thread, cannot suspend
	if (!gScheduler->isValidTid(tid))
	{
		std::cerr << LIBERR << __FUNCTION__ << LIBERR_INVALID_TID;
		return FAIL;
	}

	if (gScheduler->getTotalThreadsNum() == 1)
	{
		std::cerr << LIBERR << __FUNCTION__ << LIBERR_SUSPEND_ONLY_THREAD;
		return FAIL;
	}

	Thread *thread = gScheduler->getThread(tid);

	if (thread->getState() != BLOCKED)
	{
		// printf("%d suspending %d\n", uthread_get_tid(), tid);
		if (thread->getState() == RUNNING)
		{
			// Get next ready thread and set it as current
			gScheduler->switchThreads(BLOCKED);
			/*gScheduler->runNext();

			SignalManager::unblockSignals();
			siglongjmp(*(gScheduler->running->getEnv()), CONTINUING);*/
		}
		else
		{
			gScheduler->block(thread);
		}
	}

	// Set handler back
	SignalManager::unblockSignals();

	// If the quantum has ended till now, switch threads now.
	if (SignalManager::hasTimerSignalTriggered())
	{
		gScheduler->switchThreads(READY);
	}

	return 0;
}

/* Resume a thread */
int uthread_resume(int tid)
{
	SignalManager::postponeSignals();

	if (!gScheduler->isValidTid(tid))
	{
		std::cerr << LIBERR << __FUNCTION__ << LIBERR_INVALID_TID;
		return FAIL;
	}

	Thread *thread = gScheduler->getThread(tid);

	// If the thread is not blocked, do nothing.
	if (thread->getState() == BLOCKED)
	{
		// printf("%d resumed %d\n", uthread_get_tid(), tid);
		gScheduler->ready(thread);
		gScheduler->blockedMap.erase(tid);
	}

	thread = NULL;

	// Set handler back
	SignalManager::unblockSignals();

	// If the quantum has ended till now, switch threads now.
	if (SignalManager::hasTimerSignalTriggered())
	{
		gScheduler->switchThreads(READY);
	}

	return 0;
}


/* Get the id of the calling thread */
int uthread_get_tid()
{
	// SignalManager::postponeSignals();
	return gScheduler->running->getTid();
	// SignalManager::unblockSignals();
	// if (SignalManager::hasTimerSignalTriggered())
	// {
	// 	gScheduler->switchThreads(READY);
	// }
}

/* Get the total number of library quantums */
int uthread_get_total_quantums()
{
	return gScheduler->getTotalQuantums();
}

/* Get the number of thread quantums */
int uthread_get_quantums(int tid)
{
	if (!gScheduler->isValidTid(tid))
	{
		std::cerr << LIBERR << __FUNCTION__ << LIBERR_INVALID_TID;
		return FAIL;
	}
	
	// SignalManager::postponeSignals();
	return gScheduler->getThread(tid)->getQuantums();
	// SignalManager::unblockSignals();
	// if (SignalManager::hasTimerSignalTriggered())
	// {
	// 	gScheduler->switchThreads(READY);
	// }
}

unsigned int getMinTid()
{
	return gScheduler->getMinTid();
}
