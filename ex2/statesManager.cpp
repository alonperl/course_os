#include <setjmp.h>
#include "statesManager.hpp"

#define MEGA 100000
#define CONTINUING 1

bool StatesManager::instanceFlag = false;
StatesManager *StatesManager::instance = NULL;

StatesManager::StatesManager() {
	totalThreadsNum = 0;
	totalQuantums = -1;
}

bool StatesManager::isValidTid(int tid) {
	if (threadsMap.find(tid) == threadsMap.end() || tid < 0) {
		return false;
	}

	return true;
}

StatesManager *StatesManager::getInstance() {
	if (!instanceFlag) {
		instance = new StatesManager();
		instanceFlag = true;
		return instance;
	} else {
		return instance;
	}
}

/**
 * @brief Finds thread by tid wherever it is
 * 
 * @param tid 
 * @return pointer to the thread or NULL if such thread does not exist
 */
Thread *StatesManager::getThread(int tid) {
	if (!isValidTid(tid))

	{
		return NULL;
	}

	return threadsMap[tid];
}

int StatesManager::ready(Thread *thread) {
	if (thread->getState() == READY) {
		return SUCCESS;
	}

	thread->setState(READY);
	thread->setReadyFrom();

	readyQueue.push(thread);

	return SUCCESS;
}

int StatesManager::block(Thread *thread) {
	if (thread->getState() == BLOCKED) {
		return SUCCESS;
	}

	if (thread->getState() == READY)
	{
		readyQueue.erase(thread);
	}

	thread->setState(BLOCKED);
	blockedMap[thread->getTid()] = thread;
	return SUCCESS;
}

unsigned int StatesManager::getMinTid()
{
	if (!terminatedTids.empty()) {
		unsigned int newTid = terminatedTids.top();
		terminatedTids.pop();
		return newTid;
	}

	return totalThreadsNum;
}

int StatesManager::getTotalQuantums() {
	return totalQuantums;
}

int StatesManager::getTotalThreadsNum() {
	return totalThreadsNum;
}

itimerval *StatesManager::getQuantum() {
	return &quantum;
}

void StatesManager::setQuantum(int quantumUsec) {
	suseconds_t usec = quantumUsec % MEGA;
	time_t sec = (quantumUsec - usec) / MEGA;

	quantum.it_value.tv_sec = sec; /* first time interval, seconds part */
	quantum.it_value.tv_usec = usec; /* first time interval, microseconds part */
	quantum.it_interval.tv_sec = sec; /* following time intervals, seconds part */
	quantum.it_interval.tv_usec = usec; /* following time intervals, microseconds part */
}

void StatesManager::incrementTotalThreadsNum() {
	totalThreadsNum++;
}

void StatesManager::incrementTotalQuantums()
{
	totalQuantums++;
}

void StatesManager::decrementTotalThreadsNum() {
	totalThreadsNum--;
}

void StatesManager::staticSignalHandler(int sig) {
	(*instance).signalHandler(sig);
}

void StatesManager::signalHandler(int sig) {
	switchThreads(READY);
}

void StatesManager::runNext() {
	Thread *nextThread = readyQueue.top();
	readyQueue.pop();

	nextThread->setState(RUNNING);
	running = nextThread;

	printf("Next to run: %d\n", running->getTid());

}

void StatesManager::switchThreads(State destination) {
	SignalManager::postponeSignals();
	SignalManager::stopTimer();

	if (readyQueue.size() == 0) {
		running->incrementQuantums();
		incrementTotalQuantums();
		SignalManager::startTimer(staticSignalHandler, getQuantum());
		return;
	}

	// Save current thread
	int retVal = sigsetjmp(*(running->getEnv()), 1);
	if (retVal == CONTINUING) {
		// Set handler back
		SignalManager::unblockSignals();
		// Reset timer
		SignalManager::startTimer(staticSignalHandler, getQuantum());
		return;
	}

	Thread *prevThread = running;
	runNext();

	switch (destination) {
	case READY:
		// Move running to ready queue
		ready(prevThread);
		break;

	case BLOCKED:
		// Move running to block list
		block(prevThread);
		prevThread->setState(destination);
		break;
	}

	running->incrementQuantums();
	incrementTotalQuantums();

	// Set handler back
	SignalManager::unblockSignals();
	SignalManager::startTimer(staticSignalHandler, getQuantum());
	siglongjmp(*(running->getEnv()), CONTINUING);
}
