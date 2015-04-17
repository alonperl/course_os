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

		std::priority_queue<unsigned int, std::vector<unsigned int>,
							std::greater<unsigned int> > getTidsPool();

		ReadyQueue getReadyQueue();
		std::map<unsigned int, Thread*> getBlockedMap();
		std::map<unsigned int, Thread*> getThreadsMap();

		Thread* getRunning();
		void setRunning(Thread *thread);

		unsigned int getMinTid();

		void incrementTotalQuantums();
		void incrementTotalThreadsNum();
		void decrementTotalThreadsNum();

		bool isValidTid(int tid);

	private:
		Scheduler(int quantum_usecs);

		void setQuantum(int quantum);

		static Scheduler *s_instance;
	    static bool s_instanceFlag;

		// Min-queue of freed tids for second use
		std::priority_queue<unsigned int, std::vector<unsigned int>,
							std::greater<unsigned int> > _tidsPool;
		
		ReadyQueue _readyQueue;
	  	
	  	std::map<unsigned int, Thread*> _blockedMap;

	  	// General threads map
	  	std::map<unsigned int, Thread*> _threadsMap;

	  	// Current running thread
		Thread *_running;

	    struct itimerval _quantum;

		int _totalQuantums;
		int _totalThreadsNum;
};

#endif