#ifndef _STATES_MANAGER_H
#define _STATES_MANAGER_H

#include <queue>
#include <cstdlib>
#include <map>
#include "thread.hpp"

struct ThreadComparator
{
	bool operator()(Thread &t1, Thread &t2);
};

class StatesManager
{

	public:
		std::priority_queue<unsigned int, std::vector<unsigned int>, std::greater<unsigned int> > terminatedTids;
		std::priority_queue<Thread, std::vector<Thread>, ThreadComparator> readyQueue;
	  	std::map<unsigned int, Thread*> blockedMap;
	  	
		static StatesManager *getInstance();

		static void staticSignalHandler(int sig);

		void switchThreads(State destination);

		int ready(Thread *thread);
		int block(Thread *thread);
		int run(Thread *thread);

		Thread *getThread(int tid);

		Thread *running;

		int getTotalQuantums();
		itimerval *getQuantum();
		int getTotalThreadsNum();
		unsigned int getMinTid();

		void ignoreSignals();
		void postponeSignals();
		void unblockSignals();
		bool hasTimerSignalTriggered();
		void stopTimer();
		void startTimer();

		void setQuantum(int quantum);
		void incrementTotalThreadsNum();
		void decrementTotalThreadsNum();
	private:
		StatesManager();

		sigset_t blockedSignals;
		sigset_t pendingSignals;

		void signalHandler(int sig);

		static StatesManager *instance;
	    static bool instanceFlag;

	    struct itimerval quantum;

		int totalQuantums;
		int totalThreadsNum;
};

#endif