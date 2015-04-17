#include "Scheduler.hpp"

#define MEGA 100000
#define CONTINUING 1

bool Scheduler::instanceFlag = false;
Scheduler *Scheduler::instance = NULL;

Scheduler::Scheduler(int quantum_usecs) {
	setQuantum(quantum_usecs);
	totalThreadsNum = 0;
	totalQuantums = 0;
}

bool Scheduler::isValidTid(int tid) {
	if (threadsMap.find(tid) == threadsMap.end() || tid < 0) {
		return false;
	}

	return true;
}

Scheduler *Scheduler::getInstance(int quantum_usecs) {
	if (!instanceFlag) {
		instance = new Scheduler(quantum_usecs);
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
Thread *Scheduler::getThread(int tid) {
	if (!isValidTid(tid))

	{
		return NULL;
	}

	return threadsMap[tid];
}

int Scheduler::ready(Thread *thread) {
	if (thread->getState() == READY) {
		return SUCCESS;
	}

	thread->setState(READY);
	thread->setReadyFrom();

	readyQueue.push(thread);

	return SUCCESS;
}

int Scheduler::block(Thread *thread) {
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

unsigned int Scheduler::getMinTid()
{
	if (!terminatedTids.empty()) {
		unsigned int newTid = terminatedTids.top();
		terminatedTids.pop();
		return newTid;
	}

	return totalThreadsNum;
}

int Scheduler::getTotalQuantums() {
	return totalQuantums;
}

int Scheduler::getTotalThreadsNum() {
	return totalThreadsNum;
}

itimerval *Scheduler::getQuantum() {
	return &quantum;
}

void Scheduler::setQuantum(int quantumUsec) {
	suseconds_t usec = quantumUsec % MEGA;
	time_t sec = (quantumUsec - usec) / MEGA;

	quantum.it_value.tv_sec = sec; /* first time interval, seconds part */
	quantum.it_value.tv_usec = usec; /* first time interval, microseconds part */
	quantum.it_interval.tv_sec = sec; /* following time intervals, seconds part */
	quantum.it_interval.tv_usec = usec; /* following time intervals, microseconds part */
}

void Scheduler::incrementTotalQuantums()
{
	totalQuantums++;
}

void Scheduler::incrementTotalThreadsNum() {
	totalThreadsNum++;
}

void Scheduler::decrementTotalThreadsNum() {
	totalThreadsNum--;
}

void Scheduler::runNext() {
	Thread *nextThread = readyQueue.top();
	readyQueue.pop();

	nextThread->setState(RUNNING);
	running = nextThread;
}

void Scheduler::switchThreads(State destination) {
	SignalManager::postponeSignals();
	SignalManager::stopTimer();

	if (readyQueue.size() == 0) {
		// Single thread exists, no need to switch
		running->incrementQuantums();
		incrementTotalQuantums();

		SignalManager::unblockSignals();
		SignalManager::startTimer(getQuantum());

		return;
	}

	// Save current thread
	int retVal = sigsetjmp(*(running->getEnv()), 1);
	if (retVal == CONTINUING) {
		// Set handler back
		SignalManager::unblockSignals();
		// Reset timer
		SignalManager::startTimer(getQuantum());
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

		default:
			break;
	}

	running->incrementQuantums();
	incrementTotalQuantums();

	// TODO: maybe it is redundant because anyway long jumps to set and there is unblock
	// Set handler back
	SignalManager::unblockSignals();
	SignalManager::startTimer(getQuantum());
	siglongjmp(*(running->getEnv()), CONTINUING);
}