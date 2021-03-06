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
#define SINGLE_THREAD 1
#define MAIN 0
#define SUCCESS 0
#define FAIL -1

/* Error messages */
#define LIBERR "thread library error: "
#define LIBERR_INVALID_QUANTUM ": invalid quantum\n"
#define LIBERR_INIT_CALLED ": cannot call init more then once\n"
#define LIBERR_MAX_THREAD_NUM ": maximum threads reached\n"
#define LIBERR_INVALID_TID ": no such thread\n"
#define LIBERR_THREAD_CREATION_FAILED ": cannot create thread object\n"
#define LIBERR_SCHEDULER_CREATION_FAILED ": cannot create scheduler object\n"
#define LIBERR_SUSPEND_ONLY_THREAD ": cannot suspend main thread\n"

#ifndef _SYSERR_MESSAGES
#define SYSERR_SIGNAL "couldn't set signal handler\n"
#define SYSERR_SETITIMER "couldn't set timer\n"
#endif

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
	if (signal(SIGVTALRM, SignalManager::staticSignalHandler) == SIG_ERR)
	{
		SignalManager::systemErrorHandler(SYSERR_SIGNAL);
	}

	if (setitimer(ITIMER_VIRTUAL, scheduler->getQuantum(), NULL) == FAIL)
	{
		SignalManager::systemErrorHandler(SYSERR_SETITIMER);
	}

	scheduler = NULL;

	return SUCCESS;
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

/**
 *  @brief Terminate a thread
 *  @param int tid - the tid of the thread to terminate
 *  @return 0 iff succeeds, -1 otherwise. Exits with 0 status if main thread
 *  terminates
 */
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
	if (tid == MAIN)
	{
		std::map<unsigned int, Thread*>::iterator threadIter = 
			scheduler->getThreadsMap()->begin();

		for (; threadIter != scheduler->getThreadsMap()->end(); ++threadIter)
		{
			if (threadIter->first != MAIN)
			{
				delete threadIter->second;
			}
		}

		scheduler->destroy();

		exit(SUCCESS);
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

		// Terminated last running thread, must switch to next
		// As the program jumps elsewhere on siglongjmp, there is still one
		// pointer (here - to scheduler) that cannot be freed...
		siglongjmp(*(scheduler->getRunning()->getEnv()), 1);
	}

	scheduler = NULL;

	return SUCCESS;
}

/**
 *  @brief Suspends a thread
 *  @param int tid - the tid of the thread to suspend
 *  @return 0 iff succeseed to suspend, -1 otherwise
 */
int uthread_suspend(int tid)
{
	SignalManager::postponeSignals();

	Scheduler *scheduler = Scheduler::getInstance();

	// If got invalid tid or if there if only one existing thread, can't suspend
	if (!scheduler->isValidTid(tid))
	{
		std::cout << LIBERR << __FUNCTION__ << LIBERR_INVALID_TID;
		scheduler = NULL;
		return FAIL;
	}

	if (scheduler->getTotalThreadsNum() == SINGLE_THREAD)
	{
		std::cout << LIBERR << __FUNCTION__ << LIBERR_SUSPEND_ONLY_THREAD;
		scheduler = NULL;
		return FAIL;
	}

	Thread *thread = scheduler->getThread(tid);

	if (thread->getState() != BLOCKED)
	{
		if (thread->getState() == RUNNING)
		{
			// Get next ready thread and set it as current
			scheduler->switchThreads(BLOCKED);
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

	return SUCCESS;
}

/**
 *  @brief Resume a thread
 *  @param int tid - the tid of the thread to resume
 *  @return 0 if succeseed, -1 otherwise
 */
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

	return SUCCESS;
}


/**
 *  @return the ID of the calling thread
 */
int uthread_get_tid()
{
	return Scheduler::getInstance()->getRunning()->getTid();
}

/**
 * @return the total number of library quantums
 */
int uthread_get_total_quantums()
{
	return Scheduler::getInstance()->getTotalQuantums();
}

/**
 * @param tid Thread ID
 * @return the number of calling thread quantums
 */
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

/**
 *  @return the minimal available TID
 */
unsigned int getMinTid()
{
	return 	Scheduler::getInstance()->getMinTid();
}
