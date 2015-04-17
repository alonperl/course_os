#ifndef _STATES_MANAGER_H
#define _STATES_MANAGER_H

#include <queue>
#include <cstdlib>
#include <map>
#ifndef _THREAD_H
#include "thread.hpp"
#endif
#ifndef _SIGNAL_MANAGER
#include "signalManager.hpp"
#endif
#include "ReadyQueue.hpp"

class StatesManager
{

	public:
		std::priority_queue<unsigned int, std::vector<unsigned int>, std::greater<unsigned int> > terminatedTids;
		
		ReadyQueue readyQueue;
	  	std::map<unsigned int, Thread*> blockedMap;

	  	// General threads map
	  	std::map<unsigned int, Thread*> threadsMap;

	  	// Current running thread
		Thread *running;

		static StatesManager *getInstance();

		void switchThreads(State destination);

		int ready(Thread *thread);
		int block(Thread *thread);
		int run(Thread *thread);

		Thread *getThread(int tid);


		int getTotalQuantums();
		int getTotalThreadsNum();
		unsigned int getMinTid();
		itimerval *getQuantum();

		void runNext();

		void setQuantum(int quantum);
		void incrementTotalThreadsNum();
		void decrementTotalThreadsNum();
		void incrementTotalQuantums();

		bool isValidTid(int tid);

	private:
		StatesManager();

		static StatesManager *instance;
	    static bool instanceFlag;

	    struct itimerval quantum;

		int totalQuantums;
		int totalThreadsNum;
};

#endif
