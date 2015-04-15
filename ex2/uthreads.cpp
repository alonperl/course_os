#include <cstdlib>
#include "uthreads.h"
#include "statesManager.hpp"
#include "signalManager.hpp"

#include <unistd.h>

#define CONTINUING 1
#define MAIN 0

StatesManager *statesManager;

void signalHandler(int sig)
{
	// TODO: what to do with sig?
	statesManager->switchThreads(READY);
}

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
	 	 if (i == 1000000)
	 	 {
	 	 	printf("Thread %d finished.\n", uthread_get_tid());
	 	 	printf("Gonna Fucking Termintae\n");
	 	 	printf("TID: %d\n",uthread_get_tid() );
	 	 	uthread_terminate(uthread_get_tid());
	 	 	return;
	 	 }
	}
}

void workMain()
{
	int i = 0;
	// for(i = 0; i < 10000; i++)
	for(;; i++)
	{
		if (i == 0)
		{
			printf("Main thread 0 starts its work.\n", uthread_get_tid());
		}
		if (i % 1000 == 0)
		{
			printf(",", i);
		}
	 	 if (i == 10000000)
	 	 {
	 	 	printf("Thread %d finished.\n", uthread_get_tid());
	 	 	uthread_terminate(uthread_get_tid());
	 	 	return;
	 	 }
	}
}

int main(int argc, char const *argv[])
{
	printf("Entering main\n");
	uthread_init(500);
	printf("Inited\n");

	int i;
	for (i = 0; i < 30; ++i)
	{
		uthread_spawn(work, ORANGE);
	}

	while(1);

	printf("Finished main\n");
	return 0;
}

// TODO: what is the entry point for main thread?
void f(){while(1);}

int uthread_init(int quantum_usecs)
{
	statesManager = StatesManager::getInstance();

	if (statesManager->getTotalQuantums() != 0)//TODO: null and not 0
	{
		return FAIL;
	}

	statesManager->setQuantum(quantum_usecs);

	uthread_spawn(workMain, ORANGE);

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
		thread = new Thread(f, pr, newTid);//TODO: make sure to delete
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

	return 0;
}

/* Terminate a thread */
int uthread_terminate(int tid)
{
	printf("FUCKING ENTERED TERMINATE\n");
	SignalManager::postponeSignals();
	printf("HOPE TO DELETE THREAD %d\n", tid);

	if (tid > statesManager->getTotalThreadsNum() || tid < 0)
	{
		SignalManager::unblockSignals();
		return FAIL;
	}

	if (tid == 0)
	{
		// TODO: cleaning
		exit(0);
	}


	Thread *thread = statesManager->getThread(tid);
	printf("Got pointer to TID: %d\n", tid);
	
	bool selfDestroy = false;

	switch(thread->getState())
	{
	case READY:
		// Remove from Ready queue
		thread->resetReadyFrom();
		statesManager->readyQueue.pop();
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

	// TODO: how to properly remove object?
	printf("DELETING THREAD %d\n", tid);
	delete thread;

	statesManager->decrementTotalThreadsNum();

	// Set handler back
	SignalManager::unblockSignals();

	// If the quantum has ended till now and thread did not kill itself
	// , switch threads now.
	if (SignalManager::hasTimerSignalTriggered() && !selfDestroy)
	{
		statesManager->switchThreads(READY);
	}

	printf("Gonna do siglongjmp to TID: %d \n", statesManager->running->getTid());
	siglongjmp(*(statesManager->running->getEnv()), 1);
	return 0;
}

/* Suspend a thread */
int uthread_suspend(int tid)
{
	SignalManager::postponeSignals();

	// If got invalid tid or if there if only one existing thread, cannot suspend
	if (tid > statesManager->getTotalThreadsNum() || tid < 0 
		|| statesManager->getTotalThreadsNum() == 1)
	{
		return FAIL;
	}

	Thread *thread = statesManager->getThread(tid);

	if (thread->getState() != BLOCKED)
	{
		State oldState = thread->getState();
		statesManager->block(thread);

		if (oldState == RUNNING)
		{
			// TODO: code repetition from switchThreads
			Thread *nextThread = statesManager->readyQueue.top();
			statesManager->readyQueue.pop();

			nextThread->setState(RUNNING);
			// TODO: problems expected
			statesManager->running = nextThread;

			SignalManager::unblockSignals();
			siglongjmp(*(statesManager->running->getEnv()), CONTINUING);
		}
		//TODO what to do if running
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

	if (tid > statesManager->getTotalThreadsNum() || tid < 0)
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
	return statesManager->running->getTid();
}

/* Get the total number of library quantums */
int uthread_get_total_quantums()
{
	return statesManager->getTotalThreadsNum();
}

/* Get the number of thread quantums */
int uthread_get_quantums(int tid)
{
	return statesManager->getThread(tid)->getQuantums();
}

unsigned int getMinTid()
{
	return statesManager->getMinTid();
}
