/**
 * @file Scheduler.cpp
 * @author  griffonn ednussi
 * @version 1.0
 * @date 18 April 2015
 * 
 * @brief A scheduler class which manages context switch between threads and
 * holds all the states' data.
 *
 * @section LICENSE
 * This program is a free software. You can freely redistribute it.
 *
 * @section DESCRIPTION
 * Scheduler takes care of context switch between threads, counts overall 
 * library running time, and also takes care of reusing TIDs of terminated
 * threads. Implemented as a singleton pattern.
 */
#include "Scheduler.hpp"

#define MEGA 100000
#define RESUMING 1

/* Default: no instance created yet */
bool Scheduler::s_instanceFlag = false;
Scheduler *Scheduler::s_instance = NULL;

/**
 * @brief Private constructor
 */
Scheduler::Scheduler()
{
	_readyQueue = new PriorityQueue();
	_blockedMap = new std::map<unsigned int, Thread*>();
	_threadsMap = new std::map<unsigned int, Thread*>();

	_totalThreadsNum = 0;
	_totalQuantums = 0;
}

/**
 * @brief Cleanup Scheduler and its members' resources
 */
void Scheduler::destroy()
{
	delete _readyQueue;
	delete _blockedMap;
	delete _threadsMap;

	_running = NULL;

	delete s_instance;
	s_instance = NULL;
}

/**
 *  @brief Checks if a given tid number is valid
 *  @param int tid - the tid number to check
 *  @return bool - true if the tid number is vaild - false otherwise
 */
bool Scheduler::isValidTid(int tid)
{
	if (getThreadsMap()->find(tid) == getThreadsMap()->end() || tid < 0)
	{
		return false;
	}

	return true;
}

/**
 * @brief Singleton instance getter
 * @return Scheduler instance iff created already, otherwise creates
 *         and returns it.
 */
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
 *  @brief Get a pointer to a given thread by tid num
 *  @param int tid - the tid of the wanted thread
 */
Thread *Scheduler::getThread(int tid)
{
	if (!isValidTid(tid))
	{
		return NULL;
	}

	return getThreadsMap()->at(tid);
}

/**
 *  @brief Puts given thread to the ready state
 *  @param Thread *thread - a pointer to the thread
 *  @return SUCCESS (0)
 */
void Scheduler::ready(Thread *thread)
{
	if (thread->getState() == READY)
	{
		return;
	}

	thread->setState(READY);
	thread->setTimeInReadyQueue();

	getReadyQueue()->push(thread);
}

/**
 *  @brief Blocks given thread 
 *  @param Thread *thread - a pointer to the thread
 *  @return SUCCESS (0)
 */
void Scheduler::block(Thread *thread)
{
	if (thread->getState() == BLOCKED)
	{
		return;
	}

	if (thread->getState() == READY)
	{
		getReadyQueue()->erase(thread);
	}

	thread->setState(BLOCKED);
	(*_blockedMap)[thread->getTid()] = thread;
}

/**
 * @return minimal unused TID
 */
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

/**
 *  @return the total amount of quantums runned
 */
int Scheduler::getTotalQuantums()
{
	return _totalQuantums;
}

/**
 *  @return the total amount of existing threads
 */
int Scheduler::getTotalThreadsNum()
{
	return _totalThreadsNum;
}

/**
 *  @return the initiated quantum
 */
itimerval *Scheduler::getQuantum()
{
	return &_quantum;
}

/**
 * @return pointer to terminated TIDs pool
 */
std::priority_queue<unsigned int, std::vector<unsigned int>,
					std::greater<unsigned int> > *Scheduler::getTidsPool()
{
	return &_tidsPool;
}

/**
 * @return pointer to ready PQ
 */
PriorityQueue *Scheduler::getReadyQueue()
{
	return _readyQueue;
}

/**
 * @return pointer to blocked threads map
 */
std::map<unsigned int, Thread*> *Scheduler::getBlockedMap()
{
	return _blockedMap;
}

/**
 * @return pointer to general threads map
 */
std::map<unsigned int, Thread*> *Scheduler::getThreadsMap()
{
	return _threadsMap;
}

/**
 * @return pointer to current running thread
 */
Thread* Scheduler::getRunning()
{
	return _running;
}

/**
 * @brief Set given thread as running
 * 
 * @param thread pointer to desired thread to run
 */
void Scheduler::setRunning(Thread *thread)
{
	if (thread != NULL)
	{
		_running = thread;
	}
}

/**
 *  @brief Sets the timer with the quantum
 *  @param int quantum - the quantum to set the timer with
 */
void Scheduler::setQuantum(int quantumUsec)
{
	suseconds_t usec = quantumUsec % MEGA;
	time_t sec = (quantumUsec - usec) / MEGA;

	_quantum.it_value.tv_sec = sec; /* first time interval, sec part */
	_quantum.it_value.tv_usec = usec; /* first time interval, ms part */
	_quantum.it_interval.tv_sec = sec; /* following time intervals, sec part */
	_quantum.it_interval.tv_usec = usec; /* following time intervals, ms part */
}

/**
 *  @brief Increase the amount of total quantums
 */
void Scheduler::incrementTotalQuantums()
{
	_totalQuantums++;
}

/**
 *  @brief Increase the amount of total threads
 */
void Scheduler::incrementTotalThreadsNum()
{
	_totalThreadsNum++;
}

/**
 *  @brief Decrease the amount of total threads
 */
void Scheduler::decrementTotalThreadsNum()
{
	_totalThreadsNum--;
}

/**
 *  @brief Sets the next thread in the order as running
 */
void Scheduler::runNext()
{
	Thread *nextThread = getReadyQueue()->top();
	getReadyQueue()->pop();

	nextThread->setState(RUNNING);
	setRunning(nextThread);
}

/**
 *  @brief Move current thread to the desired state and get next 
 *         ready thread to run.
 *  @param State destination - the desired state for previous thread
 */
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
	if (retVal == RESUMING)
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
	siglongjmp(*(getRunning()->getEnv()), RESUMING);
}