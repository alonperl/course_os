#ifndef _STATES_MANAGER_H
#define _STATES_MANAGER_H

#include <map>
#include <queue>
#include "thread.hpp"
#include "signalManager.hpp"
#include "ReadyQueue.hpp"

class StatesManager
{

	public:
		// Min-queue of freed tids for second use
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

		void runNext();

		Thread *getThread(int tid);

		int getTotalQuantums();
		int getTotalThreadsNum();
		itimerval *getQuantum();

		unsigned int getMinTid();

		void incrementTotalQuantums();
		void incrementTotalThreadsNum();
		void decrementTotalThreadsNum();

		bool isValidTid(int tid);

	private:
		StatesManager(int quantum_usecs);

		void setQuantum(int quantum);

		static StatesManager *instance;
	    static bool instanceFlag;

	    struct itimerval quantum;

		int totalQuantums;
		int totalThreadsNum;
};

#endif
