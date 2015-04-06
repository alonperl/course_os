#include <cstdlib>
#include "uthreads.h"
#include "statesManager.hpp"

StatesManager *statesManager;

void signalHandler(int sig)
{
	// TODO: what to do with sig?
	statesManager->switchThreads(READY);
}

int main(int argc, char const *argv[])
{
	uthread_init(80);
	
	return 0;
}
void f(){};
int uthread_init(int quantum_usecs)
{
	statesManager = StatesManager::getInstance();

	if (statesManager->getTotalQuantums() != 0)//TODO: null and not 0
	{
		return -1;
	}

	statesManager->setQuantum(quantum_usecs);

	uthread_spawn(f, ORANGE);

	signal(SIGVTALRM, StatesManager::staticSignalHandler);
	setitimer(ITIMER_VIRTUAL, statesManager->getQuantum(), NULL);

	return 0;
}

/* Create a new thread whose entry point is f */
int uthread_spawn(void (*f)(void), Priority pr)
{
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
		return -1;
	}

	if (thread != NULL)
	{
		statesManager->ready(thread);
	}

	statesManager->incrementTotalThreadsNum();

	return 0;
}

/* Terminate a thread */
int uthread_terminate(int tid)
{
	if (tid > statesManager->getTotalThreadsNum() || tid < 0)
	{
		return -1;
	}

	if (tid == 0)
	{
		// cleaning
		exit(0);
	}

	Thread *thread = statesManager->getThread(tid);
	switch(thread->getState())
	{
	case READY:
		//pq.fuck_it(tid);
		break;
	case RUNNING:
		//stop it or wait till quantum stop?
		break;
	case BLOCKED:
		//blocked.remove(tid);
		break;
	}

	statesManager->terminatedTids.push(thread->getTid());

	delete thread;

	statesManager->decrementTotalThreadsNum();

}

/* Suspend a thread */
int uthread_suspend(int tid)
{
	statesManager->postponeSignals();

	if (tid > statesManager->getTotalThreadsNum() || tid < 0)
	{
		return -1;
	}

	Thread *thread = statesManager->getThread(tid);
	if (thread->getState() != BLOCKED)
	{
		State oldState = thread->getState();
		statesManager->block(thread);

		if (oldState == RUNNING)
		{
			// TODO: code repetition from switchThreads
			Thread nextThread = readyQueue.top();
			readyQueue.pop();

			nextThread.setState(RUNNING);
			// TODO: problems expected
			running = &nextThread;

			siglongjmp(*(running->getEnv()), CONTINUING);
		}
		//TODO what to do if running
	}

	statesManager->unblockSignals();
	if statesManager->hasTimerSignalTriggered()
	{
		statesManager->switchThreads(READY);
	}

	return 0;
}

/* Resume a thread */
int uthread_resume(int tid)
{
	if (tid > statesManager->getTotalThreadsNum() || tid < 0)
	{
		return -1;
	}

	Thread *thread = statesManager->getThread(tid);
	if (thread->getState() == BLOCKED)
	{
		//TODO what to do if running
		statesManager->ready(thread);
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