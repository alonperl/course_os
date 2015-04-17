#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include <map>
#include <queue>
#include <setjmp.h>
#include "Thread.hpp"
#include "SignalManager.hpp"
#include "ReadyQueue.hpp"

class Scheduler
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

		static Scheduler *getInstance(int quantum_usecs);

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
		Scheduler(int quantum_usecs);

		void setQuantum(int quantum);

		static Scheduler *instance;
	    static bool instanceFlag;

	    struct itimerval quantum;

		int totalQuantums;
		int totalThreadsNum;
};

#endif