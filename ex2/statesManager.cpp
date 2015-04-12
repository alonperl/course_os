#include "statesManager.hpp"
#include <setjmp.h>

#define MEGA 100000
#define CONTINUING 1

bool StatesManager::instanceFlag = false;
StatesManager *StatesManager::instance = NULL;

StatesManager::StatesManager()
{
	totalThreadsNum = 0;
	totalQuantums = 0;

	sigaddset(&blockedSignals, SIGVTALRM);
}

bool StatesManager::isValidTid(int tid)
{
	if (tid > getTotalThreadsNum() || tid < 0)
	{
		return false;
	}

	return true;
}

StatesManager *StatesManager::getInstance()
{
	if (!instanceFlag)
	{
		instance = new StatesManager();
		instanceFlag = true;
        return instance;
    }
    else
    {
        return instance;
    }
}


/**
 * @brief Finds thread by tid wherever it is
 * 
 * @param tid 
 * @return pointer to the thread or NULL if such thread does not exist
 */
Thread *StatesManager::getThread(int tid)
{
	if (!isValidTid(tid))
	{
		return NULL;
	}

	return threadsMap[tid];
}

int StatesManager::ready(Thread *thread)
{
	printf("TID %d: entered ready function\n", thread->getTid());
	if (thread->getState() == READY)
	{
		return SUCCESS;
	}

	thread->setState(READY);
	thread->setReadyFrom();

	readyQueue.push(thread);

	printf("%d entered ready queue\n", thread->getTid());
	// TODO: if running, should we wait till end of cycle
	// TODO: When readying running thread - need to reset timer, save its pos etc
	return SUCCESS;
}

int StatesManager::block(Thread *thread)
{
	if (thread->getState() == BLOCKED)
	{
		return SUCCESS;
	}

	thread->setState(BLOCKED);
	blockedMap[thread->getTid()] = thread;
	// TODO: When blocking running thread - need to reset timer, save its pos etc
	return SUCCESS;
}

unsigned int StatesManager::getMinTid() // TODO: make it O(1)
{
	if (!terminatedTids.empty())
	{
		unsigned int newTid = terminatedTids.top();
		terminatedTids.pop();
		return newTid;
	}

	return totalThreadsNum;
}

int StatesManager::getTotalQuantums()
{
	return totalQuantums;
}

int StatesManager::getTotalThreadsNum()
{
	return totalThreadsNum;
}

itimerval *StatesManager::getQuantum()
{
	return &quantum;
}

void StatesManager::setQuantum(int quantumUsec)
{
	suseconds_t usec = quantumUsec % MEGA;
	time_t sec = (quantumUsec - usec) / MEGA;

	quantum.it_value.tv_sec = sec;  	/* first time interval, seconds part */
	quantum.it_value.tv_usec = usec;    /* first time interval, microseconds part */
	quantum.it_interval.tv_sec = sec;  	/* following time intervals, seconds part */
	quantum.it_interval.tv_usec = usec; /* following time intervals, microseconds part */
}

void StatesManager::incrementTotalThreadsNum()
{
	totalThreadsNum++;
}

void StatesManager::decrementTotalThreadsNum()
{
	totalThreadsNum--;
}

void StatesManager::staticSignalHandler(int sig)
{
	(*instance).signalHandler(sig);
}

void StatesManager::signalHandler(int sig)
{
	switchThreads(READY);
}

void StatesManager::runNext()
{
	Thread *nextThread = readyQueue.top();
	nextThread->setState(RUNNING);
	running = nextThread;
	readyQueue.pop();
}

void StatesManager::switchThreads(State destination)
{
	stopTimer();
	printf("Entered switchTThreads\n");

	if (totalThreadsNum == 1)
	{
		startTimer();
		return;
	}

	//TODO: check if is the only one of its pripority - and is the highest one - if it's the highest just increase number of quantom running

	// Save current thread
	printf("Trying...\n");
	int retVal = sigsetjmp(*(running->getEnv()), 1);
	printf("OK\n");
	if (retVal == CONTINUING)
	{
		// Reset timer
		startTimer();
		return;
	}

	switch(destination)
	{
		case READY:
			// Move running to ready queue
			ready(running);
			running->setState(destination);
			running->incrementQuantums();
			running->setReadyFrom();
			break;

		case BLOCKED:
			// Move running to block list
			block(running);
			running->setState(destination);
			running->incrementQuantums();
			break;
	}

	runNext();

	startTimer();
	siglongjmp(*(running->getEnv()), CONTINUING);
}

bool ThreadComparator::operator()(Thread *t1, Thread *t2)
{
    if (t1->getPriority() == t2->getPriority())
    {
    	if (t1->getReadyFrom().tv_sec < t2->getReadyFrom().tv_sec)
    	        return true;				/* Less than. */
    	    else if (t1->getReadyFrom().tv_sec > t2->getReadyFrom().tv_sec)
    	        return false;				/* Greater than. */
    	    else if (t1->getReadyFrom().tv_usec < t2->getReadyFrom().tv_usec)
    	        return true;				/* Less than. */
    	    else if (t1->getReadyFrom().tv_usec > t2->getReadyFrom().tv_usec)
    	        return false;				/* Greater than. */
    	    else
    	        return false;				/* Equal. Cannot happen. */ 
    }

    return t1->getPriority() < t2->getPriority();
}

void StatesManager::ignoreSignals()
{
	signal(SIGVTALRM, SIG_IGN);
}

void StatesManager::postponeSignals()
{
	sigprocmask(SIG_BLOCK, &blockedSignals, NULL);
}

void StatesManager::unblockSignals()
{
	sigprocmask(SIG_UNBLOCK, &blockedSignals, NULL);
}

bool StatesManager::hasTimerSignalTriggered()
{
	sigemptyset(&pendingSignals);
	sigpending(&pendingSignals);
	return sigismember(&pendingSignals, SIGVTALRM);
}

void StatesManager::stopTimer()
{
	ignoreSignals();
	struct itimerval reset = {0, 0};
    setitimer(ITIMER_VIRTUAL, &reset, NULL);
}

void StatesManager::startTimer()
{
	signal(SIGVTALRM, staticSignalHandler);
	setitimer(ITIMER_VIRTUAL, getQuantum(), NULL);
}
