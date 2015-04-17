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

	Scheduler *scheduler = Scheduler::getInstance();

	// If init was called before, scheduler will contain at least main thread
	if (scheduler->getTotalThreadsNum() > 0)
	{
		std::cerr << LIBERR << __FUNCTION__ << LIBERR_INIT_CALLED;
		return FAIL;
	}

	scheduler->setQuantum(quantum_usecs);

	uthread_spawn(NULL, ORANGE);
	scheduler->threadsMap[MAIN]->incrementQuantums();
	scheduler->incrementTotalQuantums();

	scheduler->runNext();

	signal(SIGVTALRM, SignalManager::staticSignalHandler);
	setitimer(ITIMER_VIRTUAL, scheduler->getQuantum(), NULL);

	return 0;
}

/* Create a new thread whose entry point is f */
int uthread_spawn(void (*f)(void), Priority pr)
{
	SignalManager::postponeSignals();

	Scheduler *scheduler = Scheduler::getInstance();

	if (scheduler->getTotalThreadsNum() >= MAX_THREAD_NUM)
	{
		std::cerr << LIBERR << __FUNCTION__ << LIBERR_MAX_THREAD_NUM;
		return FAIL;
	}
	
	Thread *thread;
	unsigned int newTid = scheduler->getMinTid();

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
		scheduler->ready(thread);
		scheduler->threadsMap[newTid] = thread;
	}

	scheduler->incrementTotalThreadsNum();

	// Set handler back
	SignalManager::unblockSignals();

	// If the quantum has ended till now, switch threads now.
	if (SignalManager::hasTimerSignalTriggered())
	{
		scheduler->switchThreads(READY);
	}

	return newTid;
}

/* Terminate a thread */
int uthread_terminate(int tid)
{
	SignalManager::postponeSignals();

	Scheduler *scheduler = Scheduler::getInstance();

	if (!scheduler->isValidTid(tid))
	{
		std::cerr << LIBERR << __FUNCTION__ << LIBERR_INVALID_TID;
		SignalManager::unblockSignals();
		return FAIL;
	}

	// Terminating main
	if (tid == 0)
	{
		std::map<unsigned int, Thread*>::iterator threadsIterator = scheduler->threadsMap.begin();
		for (; threadsIterator != scheduler->threadsMap.end(); ++threadsIterator)
		{
			if (threadsIterator->first != 0)
			{
				delete threadsIterator->second;
			}
		}

		exit(0);
	}

	Thread *thread = scheduler->getThread(tid);

	bool selfDestroy = false;

	switch(thread->getState())
	{
		case READY:
			// Remove from Ready queue
			scheduler->readyQueue.erase(thread);
			break;

		case RUNNING:
			// Stop current thread and run next ready thread
			selfDestroy = true;
			scheduler->runNext();
			break;

		case BLOCKED:
			// Remove from blocked
			scheduler->blockedMap.erase(tid);
			break;

		default:
			break;
	}

	scheduler->threadsMap.erase(thread->getTid());
	scheduler->terminatedTids.push(thread->getTid());

	delete thread;

	scheduler->decrementTotalThreadsNum();

	// If the quantum has ended till now and thread did not kill itself
	// , switch threads now.
	if (SignalManager::hasTimerSignalTriggered() && !selfDestroy)
	{
		scheduler->switchThreads(READY);
	}

	if (selfDestroy)
	{
		scheduler->running->incrementQuantums();
		scheduler->incrementTotalQuantums();

		// Terminated last running thread, must switch to next
		// TODO after f ends, g gets into running, but does not work (g q = 8)
		siglongjmp(*(scheduler->running->getEnv()), 1);
	}

	return 0;
}

/* Suspend a thread */
int uthread_suspend(int tid)
{
	SignalManager::postponeSignals();

	Scheduler *scheduler = Scheduler::getInstance();

	// If got invalid tid or if there if only one existing thread, cannot suspend
	if (!scheduler->isValidTid(tid))
	{
		std::cerr << LIBERR << __FUNCTION__ << LIBERR_INVALID_TID;
		return FAIL;
	}

	if (scheduler->getTotalThreadsNum() == 1)
	{
		std::cerr << LIBERR << __FUNCTION__ << LIBERR_SUSPEND_ONLY_THREAD;
		return FAIL;
	}

	Thread *thread = scheduler->getThread(tid);

	if (thread->getState() != BLOCKED)
	{
		// printf("%d suspending %d\n", uthread_get_tid(), tid);
		if (thread->getState() == RUNNING)
		{
			// Get next ready thread and set it as current
			scheduler->switchThreads(BLOCKED);
			/*scheduler->runNext();

			SignalManager::unblockSignals();
			siglongjmp(*(scheduler->running->getEnv()), CONTINUING);*/
		}
		else
		{
			scheduler->block(thread);
		}
	}

	// Set handler back
	SignalManager::unblockSignals();

	// If the quantum has ended till now, switch threads now.
	if (SignalManager::hasTimerSignalTriggered())
	{
		scheduler->switchThreads(READY);
	}

	return 0;
}

/* Resume a thread */
int uthread_resume(int tid)
{
	SignalManager::postponeSignals();

	Scheduler *scheduler = Scheduler::getInstance();

	if (!scheduler->isValidTid(tid))
	{
		std::cerr << LIBERR << __FUNCTION__ << LIBERR_INVALID_TID;
		return FAIL;
	}

	Thread *thread = scheduler->getThread(tid);

	// If the thread is not blocked, do nothing.
	if (thread->getState() == BLOCKED)
	{
		// printf("%d resumed %d\n", uthread_get_tid(), tid);
		scheduler->ready(thread);
		scheduler->blockedMap.erase(tid);
	}

	thread = NULL;

	// Set handler back
	SignalManager::unblockSignals();

	// If the quantum has ended till now, switch threads now.
	if (SignalManager::hasTimerSignalTriggered())
	{
		scheduler->switchThreads(READY);
	}

	return 0;
}


/* Get the id of the calling thread */
int uthread_get_tid()
{
	return Scheduler::getInstance()->running->getTid();
}

/* Get the total number of library quantums */
int uthread_get_total_quantums()
{
	return scheduler->getTotalQuantums();
}

/* Get the number of thread quantums */
int uthread_get_quantums(int tid)
{
	Scheduler *scheduler = Scheduler::getInstance();

	if (!scheduler->isValidTid(tid))
	{
		std::cerr << LIBERR << __FUNCTION__ << LIBERR_INVALID_TID;
		return FAIL;
	}
	
	return scheduler->getThread(tid)->getQuantums();
}

unsigned int getMinTid()
{
	return 	Scheduler::getInstance()->getMinTid();
}
