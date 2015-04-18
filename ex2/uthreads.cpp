/**
 * @file uthreads.c
 * @author  griffonn ednussi
 * @version 1.0
 * @date 18 April 2015
 * 
 * @brief User level threads library - contains library functions
 *
 * @section LICENSE
 * This program is a free software. You can freely redistribute it.
 *
 * @section DESCRIPTION
 * Implementing the given uthread.h library and all ot its functions which
 * enables us to work with the threads, create main thread, delete, suspend and
 * block threads, and get some information about them.
 */
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

/* Definitions */
#define RESUMING 1
#define MAIN 0

/* Error messages */
#define LIBERR "thread library error: "
#define LIBERR_INVALID_QUANTUM ": invalid quantum\n"
#define LIBERR_INIT_CALLED ": cannot call init more then once\n"
#define LIBERR_MAX_THREAD_NUM ": maximum threads reached\n"
#define LIBERR_INVALID_TID ": no such thread\n"
#define LIBERR_THREAD_CREATION_FAILED ": cannot create thread object\n"
#define LIBERR_SCHEDULER_CREATION_FAILED ": cannot create scheduler object\n"
#define LIBERR_SUSPEND_ONLY_THREAD ": cannot suspend main thread\n"

/**
 * @brief Initiate the library
 * @details Initializes Scheduler, creates main thread, registers virtual alarm 
 * signal and starts virtual times
 * 
 * @param quantum_usecs time for each run per thread
 * @return -1 iff something failed, 0 otherwise
 */
int uthread_init(int quantum_usecs)
{
	if (quantum_usecs <= 0)
	{
		std::cout << LIBERR << __FUNCTION__ << LIBERR_INVALID_QUANTUM;
		return FAIL;
	}

	Scheduler *scheduler;

	try
	{
		scheduler = Scheduler::getInstance();
	}
	catch (int e)
	{
		std::cout << LIBERR << __FUNCTION__ << LIBERR_SCHEDULER_CREATION_FAILED;
		return FAIL;
	}


	// If init was called before, scheduler will contain at least main thread
	if (scheduler->getTotalThreadsNum() > 0)
	{
		std::cout << LIBERR << __FUNCTION__ << LIBERR_INIT_CALLED;
		return FAIL;
	}

	scheduler->setQuantum(quantum_usecs);

	// Spawn main thread
	uthread_spawn(NULL, ORANGE);

	// It starts its "work" rightaway
	scheduler->getThread(MAIN)->incrementQuantums();
	scheduler->incrementTotalQuantums();

	scheduler->runNext();

	// Start the timer
	signal(SIGVTALRM, SignalManager::staticSignalHandler);
	setitimer(ITIMER_VIRTUAL, scheduler->getQuantum(), NULL);

	scheduler = NULL;

	return 0;
}

/**
 * @brief Create a new thread
 * 
 * @param f New thread start routine
 * @param pr Priority
 * 
 * @return Thread ID iff succeeded, -1 otherwise
 */
int uthread_spawn(void (*f)(void), Priority pr)
{
	SignalManager::postponeSignals();

	Scheduler *scheduler = Scheduler::getInstance();

	if (scheduler->getTotalThreadsNum() >= MAX_THREAD_NUM)
	{
		std::cout << LIBERR << __FUNCTION__ << LIBERR_MAX_THREAD_NUM;
		scheduler = NULL;
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
		std::cout << LIBERR << __FUNCTION__ << LIBERR_THREAD_CREATION_FAILED;
		return FAIL;
	}

	if (thread != NULL)
	{
		scheduler->ready(thread);
		(*scheduler->getThreadsMap())[newTid] = thread;
	}

	scheduler->incrementTotalThreadsNum();

	// Set handler back
	SignalManager::unblockSignals();

	// If the quantum has ended till now, switch threads now.
	if (SignalManager::hasTimerSignalTriggered())
	{
		scheduler->switchThreads(READY);
	}

	scheduler = NULL;

	return newTid;
}

/* Terminate a thread */
int uthread_terminate(int tid)
{
	SignalManager::postponeSignals();

	Scheduler *scheduler = Scheduler::getInstance();

	if (!scheduler->isValidTid(tid))
	{
		std::cout << LIBERR << __FUNCTION__ << LIBERR_INVALID_TID;
		scheduler = NULL;
		SignalManager::unblockSignals();
		return FAIL;
	}

	// Terminating main
	if (tid == 0)
	{
		std::map<unsigned int, Thread*>::iterator threadIter = scheduler->getThreadsMap()->begin();
		for (; threadIter != scheduler->getThreadsMap()->end(); ++threadIter)
		{
			if (threadIter->first != 0)
			{
				delete threadIter->second;
			}
		}

		scheduler->destroy();

		exit(0);
	}

	Thread *thread = scheduler->getThread(tid);

	bool selfDestroy = false;

	switch(thread->getState())
	{
		case READY:
			// Remove from Ready queue
			scheduler->getReadyQueue()->erase(thread);
			break;

		case RUNNING:
			// Stop current thread and run next ready thread
			selfDestroy = true;
			scheduler->runNext();
			break;

		case BLOCKED:
			// Remove from blocked
			scheduler->getBlockedMap()->erase(tid);
			break;

		default:
			break;
	}

	scheduler->getThreadsMap()->erase(thread->getTid());
	scheduler->getTidsPool()->push(thread->getTid());

	delete thread;

	thread = NULL;

	scheduler->decrementTotalThreadsNum();

	// If the quantum has ended till now and thread did not kill itself
	// , switch threads now.
	if (SignalManager::hasTimerSignalTriggered() && !selfDestroy)
	{
		scheduler->switchThreads(READY);
	}

	if (selfDestroy)
	{
		scheduler->getRunning()->incrementQuantums();
		scheduler->incrementTotalQuantums();

		scheduler = NULL;

		// Terminated last running thread, must switch to next
		siglongjmp(*(scheduler->getRunning()->getEnv()), 1);
	}

	scheduler = NULL;

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
		std::cout << LIBERR << __FUNCTION__ << LIBERR_INVALID_TID;
		scheduler = NULL;
		return FAIL;
	}

	if (scheduler->getTotalThreadsNum() == 1)
	{
		std::cout << LIBERR << __FUNCTION__ << LIBERR_SUSPEND_ONLY_THREAD;
		scheduler = NULL;
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
			siglongjmp(*(scheduler->running->getEnv()), RESUMING);*/
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

	scheduler = NULL;

	return 0;
}

/* Resume a thread */
int uthread_resume(int tid)
{
	SignalManager::postponeSignals();

	Scheduler *scheduler = Scheduler::getInstance();

	if (!scheduler->isValidTid(tid))
	{
		std::cout << LIBERR << __FUNCTION__ << LIBERR_INVALID_TID;
		scheduler = NULL;
		return FAIL;
	}

	Thread *thread = scheduler->getThread(tid);

	// If the thread is not blocked, do nothing.
	if (thread->getState() == BLOCKED)
	{
		// printf("%d resumed %d\n", uthread_get_tid(), tid);
		scheduler->ready(thread);
		scheduler->getBlockedMap()->erase(tid);
	}

	thread = NULL;

	// Set handler back
	SignalManager::unblockSignals();

	// If the quantum has ended till now, switch threads now.
	if (SignalManager::hasTimerSignalTriggered())
	{
		scheduler->switchThreads(READY);
	}

	scheduler = NULL;

	return 0;
}


/* Get the id of the calling thread */
int uthread_get_tid()
{
	return Scheduler::getInstance()->getRunning()->getTid();
}

/* Get the total number of library quantums */
int uthread_get_total_quantums()
{
	return Scheduler::getInstance()->getTotalQuantums();
}

/* Get the number of thread quantums */
int uthread_get_quantums(int tid)
{
	Scheduler *scheduler = Scheduler::getInstance();

	if (!scheduler->isValidTid(tid))
	{
		std::cout << LIBERR << __FUNCTION__ << LIBERR_INVALID_TID;
		scheduler = NULL;
		return FAIL;
	}
	
	int quantums = scheduler->getThread(tid)->getQuantums();

	scheduler = NULL;

	return quantums;
}

unsigned int getMinTid()
{
	return 	Scheduler::getInstance()->getMinTid();
}
