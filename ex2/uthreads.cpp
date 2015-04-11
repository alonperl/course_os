#include <cstdlib>
#include "uthreads.h"
#include "statesManager.hpp"

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
	printf("%d\n", uthread_get_tid());
	usleep(4);
	kill(0, 26);
}

int main(int argc, char const *argv[])
{
	printf("Entering main\n");
	uthread_init(1000000);
	printf("Inited\n");

	uthread_spawn(work, ORANGE);
	printf("Spawned 1\n");
	uthread_spawn(work, ORANGE);
	printf("Spawned 2\n");
	uthread_spawn(work, ORANGE);
	printf("Spawned 3\n");

	siglongjmp(*(statesManager->getThread(MAIN)->getEnv()), 1);

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

	uthread_spawn(f, ORANGE);

	signal(SIGVTALRM, StatesManager::staticSignalHandler);
	setitimer(ITIMER_VIRTUAL, statesManager->getQuantum(), NULL);

	statesManager->run(MAIN);

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
		return FAIL;
	}

	if (thread != NULL)
	{
		printf("Thread not null and its state is %d\n", thread->getState());
		statesManager->ready(thread);
		statesManager->threadsMap[newTid] = thread;
	}

	statesManager->incrementTotalThreadsNum();

	return 0;
}

/* Terminate a thread */
int uthread_terminate(int tid)
{
	if (tid > statesManager->getTotalThreadsNum() || tid < 0)
	{
		return FAIL;
	}

	if (tid == 0)
	{
		// TODO: cleaning
		exit(0);
	}

	Thread *thread = statesManager->getThread(tid);
	
	switch(thread->getState())
	{
	case READY:
		//pq.fuck_it(tid);
		break;

	case RUNNING:
		// Stop current thread and get next ready thread to running without waiting till quantum end
		break;

	case BLOCKED:
		//blocked.remove(tid);
		break;
	}

	statesManager->threadsMap.erase(thread->getTid());
	statesManager->terminatedTids.push(thread->getTid());

	// TODO: how to properly remove object?
	delete thread;

	statesManager->decrementTotalThreadsNum();

}

/* Suspend a thread */
int uthread_suspend(int tid)
{
	statesManager->postponeSignals();

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
			Thread nextThread = statesManager->readyQueue.top();
			statesManager->readyQueue.pop();

			nextThread.setState(RUNNING);
			// TODO: problems expected
			statesManager->running = &nextThread;

			statesManager->unblockSignals();
			siglongjmp(*(statesManager->running->getEnv()), CONTINUING);
		}
		//TODO what to do if running
	}

	// Set handler back
	statesManager->unblockSignals();

	// If the quantum has ended till now, switch threads now.
	if (statesManager->hasTimerSignalTriggered())
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