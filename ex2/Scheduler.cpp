#include "Scheduler.hpp"

#define MEGA 100000
#define CONTINUING 1

bool Scheduler::s_instanceFlag = false;
Scheduler *Scheduler::s_instance = NULL;

Scheduler::Scheduler()
{
	_totalThreadsNum = 0;
	_totalQuantums = 0;
}

void Scheduler::destroy()
{
	// delete &_readyQueue;
	// delete &_blockedMap;
	// delete &_threadsMap;

	delete &_quantum;

	_running = NULL;

	delete s_instance;
	s_instance = NULL;
}

bool Scheduler::isValidTid(int tid)
{
	if (getThreadsMap()->find(tid) == getThreadsMap()->end() || tid < 0)
	{
		return false;
	}

	return true;
}

Scheduler *Scheduler::getInstance()
{
	if (!s_instanceFlag)
	{
		s_instance = new Scheduler();
		s_instanceFlag = true;
		return s_instance;
	}
	else
	{
		return s_instance;
	}
}

/**
 * @brief Finds thread by tid wherever it is
 * 
 * @param tid 
 * @return pointer to the thread or NULL if such thread does not exist
 */
Thread *Scheduler::getThread(int tid)
{
	if (!isValidTid(tid))
	{
		return NULL;
	}

	return getThreadsMap()->at(tid);
}

int Scheduler::ready(Thread *thread)
{
	if (thread->getState() == READY)
	{
		return SUCCESS;
	}

	thread->setState(READY);
	thread->setReadyFrom();

	getReadyQueue()->push(thread);

	return SUCCESS;
}

int Scheduler::block(Thread *thread)
{
	if (thread->getState() == BLOCKED)
	{
		return SUCCESS;
	}

	if (thread->getState() == READY)
	{
		getReadyQueue()->erase(thread);
	}

	thread->setState(BLOCKED);
	_blockedMap[thread->getTid()] = thread;
	return SUCCESS;
}

unsigned int Scheduler::getMinTid()
{
	if (!getTidsPool()->empty())
	{
		// TODO as pq doesn't work good with top, change to list
		unsigned int newTid = getTidsPool()->top();
		getTidsPool()->pop();
		return newTid;
	}

	return _totalThreadsNum;
}

int Scheduler::getTotalQuantums()
{
	return _totalQuantums;
}

int Scheduler::getTotalThreadsNum()
{
	return _totalThreadsNum;
}

itimerval *Scheduler::getQuantum()
{
	return &_quantum;
}

std::priority_queue<unsigned int, std::vector<unsigned int>,
					std::greater<unsigned int> > *Scheduler::getTidsPool()
{
	return &_tidsPool;
}

PriorityQueue *Scheduler::getReadyQueue()
{
	return &_readyQueue;
}

std::map<unsigned int, Thread*> *Scheduler::getBlockedMap()
{
	return &_blockedMap;
}

std::map<unsigned int, Thread*> *Scheduler::getThreadsMap()
{
	return &_threadsMap;
}

Thread* Scheduler::getRunning()
{
	return _running;
}

void Scheduler::setRunning(Thread *thread)
{
	if (thread != NULL)
	{
		_running = thread;
	}
}

void Scheduler::setQuantum(int quantumUsec)
{
	suseconds_t usec = quantumUsec % MEGA;
	time_t sec = (quantumUsec - usec) / MEGA;

	_quantum.it_value.tv_sec = sec; /* first time interval, seconds part */
	_quantum.it_value.tv_usec = usec; /* first time interval, microseconds part */
	_quantum.it_interval.tv_sec = sec; /* following time intervals, seconds part */
	_quantum.it_interval.tv_usec = usec; /* following time intervals, microseconds part */
}

void Scheduler::incrementTotalQuantums()
{
	_totalQuantums++;
}

void Scheduler::incrementTotalThreadsNum()
{
	_totalThreadsNum++;
}

void Scheduler::decrementTotalThreadsNum()
{
	_totalThreadsNum--;
}

void Scheduler::runNext()
{
	Thread *nextThread = getReadyQueue()->top();
	getReadyQueue()->pop();

	nextThread->setState(RUNNING);
	setRunning(nextThread);
}

void Scheduler::switchThreads(State destination)
{
	SignalManager::postponeSignals();
	SignalManager::stopTimer();

	if (getReadyQueue()->size() == 0)
	{
		// Single thread exists, no need to switch
		getRunning()->incrementQuantums();
		incrementTotalQuantums();

		SignalManager::unblockSignals();
		SignalManager::startTimer(getQuantum());

		return;
	}

	// Save current thread
	int retVal = sigsetjmp(*(getRunning()->getEnv()), 1);
	if (retVal == CONTINUING)
	{
		// Set handler back
		SignalManager::unblockSignals();
		// Reset timer
		SignalManager::startTimer(getQuantum());
		return;
	}

	Thread *prevThread = getRunning();
	runNext();

	switch (destination)
	{
		case READY:
			// Move running to ready queue
			ready(prevThread);
			break;

		case BLOCKED:
			// Move running to block list
			block(prevThread);
			prevThread->setState(destination);
			break;

		default:
			break;
	}

	getRunning()->incrementQuantums();
	incrementTotalQuantums();

	// TODO: maybe it is redundant because anyway long jumps to set and there is unblock
	// Set handler back
	SignalManager::unblockSignals();
	SignalManager::startTimer(getQuantum());
	siglongjmp(*(getRunning()->getEnv()), CONTINUING);
}