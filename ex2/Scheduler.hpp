/**
 * @file Scheduler.hpp
 * @author  griffonn ednussi
 * @version 1.0
 * @date 18 April 2015
 * 
 * @brief Implementation of a scheduler class which manages context 
 * switch between threads and holds all the states' data.
 *
 * @section LICENSE
 * This program is a free software. You can freely redistribute it.
 *
 * @section DESCRIPTION
 * Scheduler takes care of context switch between threads, counts overall 
 * library running time, and also takes care of reusing TIDs of terminated
 * threads. Implemented as a singleton pattern.
 */
#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include <map>
#include <queue>
#include <setjmp.h>
#include "Thread.hpp"
#include "SignalManager.hpp"
#include "PriorityQueue.hpp"

/**
 * @brief Scheduler provides functions for context switch, TID reuse, and
 * some utility function, as total quantum count.
 */
class Scheduler
{
	public:
		/**
		 * @brief Singleton instance getter
		 * @return Scheduler instance iff created already, otherwise creates
		 *         and returns it.
		 */
		static Scheduler *getInstance();

		/**
		 *  @brief Move current thread to the desired state and get next 
		 *         ready thread to run.
		 *  @param State destination - the desired state for previous thread
		 */
		void switchThreads(State destination);

		/**
		 *  @brief Puts given thread to the ready state
		 *  @param Thread *thread - a pointer to the thread
		 *  @return SUCCESS (0)
		 */
		void ready(Thread *thread);

		/**
		 *  @brief Blocks given thread 
		 *  @param Thread *thread - a pointer to the thread
		 *  @return SUCCESS (0)
		 */
		void block(Thread *thread);

		/**
		 *  @brief Sets the next thread in the order as running
		 */
		void runNext();

		/**
		 *  @brief Get a pointer to a given thread by tid num
		 *  @param int tid - the tid of the wanted thread
		 */
		Thread *getThread(int tid);

		/**
		 *  @return the total amount of quantums runned
		 */
		int getTotalQuantums();

		/**
		 *  @return the total amount of existing threads
		 */
		int getTotalThreadsNum();
		
		/**
		 *  @return the initiated quantum
		 */
		itimerval *getQuantum();

		/**
		 * @return pointer to terminated TIDs pool
		 */
		std::priority_queue<unsigned int, std::vector<unsigned int>,
							std::greater<unsigned int> > *getTidsPool();

		/**
		 * @return pointer to ready PQ
		 */
		PriorityQueue *getReadyQueue();

		/**
		 * @return pointer to blocked threads map
		 */
		std::map<unsigned int, Thread*> *getBlockedMap();

		/**
		 * @return pointer to general threads map
		 */
		std::map<unsigned int, Thread*> *getThreadsMap();

		/**
		 * @return pointer to current running thread
		 */
		Thread* getRunning();

		/**
		 * @brief Set given thread as running
		 * 
		 * @param thread pointer to desired thread to run
		 */
		void setRunning(Thread *thread);

		/**
		 * @return minimal unused TID
		 */
		unsigned int getMinTid();

		/**
		 *  @brief Increase the amount of total quantums
		 */
		void incrementTotalQuantums();

		/**
		 *  @brief Increase the amount of total threads
		 */
		void incrementTotalThreadsNum();

		/**
		 *  @brief Decrease the amount of total threads
		 */
		void decrementTotalThreadsNum();

		/**
		 *  @brief Checks if a given tid number is valid
		 *  @param int tid - the tid number to check
		 *  @return bool - true if the tid number is vaild - false otherwise
		 */
		bool isValidTid(int tid);

		/**
		 *  @brief Sets the timer with the quantum
		 *  @param int quantum - the quantum to set the timer with
		 */
		void setQuantum(int quantum);

		/**
		 * @brief Cleanup Scheduler and its members' resources
		 */
		void destroy();

	private:
		/**
		 * @brief Private constructor
		 */
		Scheduler();

		static Scheduler *s_instance;

		/* Indicates if Scheduler was initiated already */
	    static bool s_instanceFlag;

		/* Min-queue of freed tids for second use */
		std::priority_queue<unsigned int, std::vector<unsigned int>,
							std::greater<unsigned int> > _tidsPool;
		
		PriorityQueue *_readyQueue;

	  	std::map<unsigned int, Thread*> *_blockedMap;

	  	// General threads map
	  	std::map<unsigned int, Thread*> *_threadsMap;

	  	// Current running thread
		Thread *_running;

		/* Single quantum time val */
	    struct itimerval _quantum;

		int _totalQuantums;
		int _totalThreadsNum;
};

#endif