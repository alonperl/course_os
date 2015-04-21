/**
 * @file PriorityQueue.hpp
 * @author  griffonn ednussi
 * @version 1.0
 * @date 18 April 2015
 * 
 * @brief A list decorator that acts as a priority queue with simple
 * interface
 *
 * @section LICENSE
 * This program is a free software. You can freely redistribute it.
 *
 * @section DESCRIPTION
 * PQ takes care of sorting the elements inside it with given 
 * implementation of comparator.
 */
#include <list>
#include "Thread.hpp"

/**
 * @brief Decoration class for a sorted list that provides an option to use it
 * as a priority queue.
 */
class PriorityQueue
{
	public:
		PriorityQueue();
		~PriorityQueue();
	
		/**
		 * @brief Decorator for PQ size
		 */
		int size();
	
		/**
		 * @brief Delete top thread from queue
		 */
		void pop();
	
		/**
		 * @brief Get top thread in the queue
		 * @return the top thread in the queue
		 */
		Thread *top();
	
		/**
		 * @brief Pushes in a thread into the queue
		 * @param Thread *thread - the pointer of a thread to be pushed
		 */
		void push(Thread *thread);
	
		/**
		 * @brief Erase specific thread within the queue
		 * @param Thread *thread - the pointer of a thread to be erased
		 */
		void erase(Thread *thread);
	
	private:
		/**
		 * @brief Comparator for threads
		 * @details Compares by priority, then by time waiting in ready PQ
		 * 
		 * @param t1 first thread
		 * @param t2 second thread
		 * 
		 * @return -1 if first thread is "less" then second, 1 if second is
		 * "less" then first, 0 if they are the same (nearly impossible happen
		 * in this implementation).
		 */
		static bool _compare(Thread *t1, Thread *t2);

		std::list<Thread*> *_ready;
};