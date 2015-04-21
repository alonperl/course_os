/**
 * @file PriorityQueue.cpp
 * @author  griffonn ednussi
 * @version 1.0
 * @date 18 April 2015
 * 
 * @brief An implementation of priority queue as a list decorator.
 *
 * @section LICENSE
 * This program is a free software. You can freely redistribute it.
 *
 * @section DESCRIPTION
 * PQ takes care of sorting the elements inside it with given 
 * implementation of comparator.
 */
#include "PriorityQueue.hpp"

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
bool PriorityQueue::_compare(Thread *t1, Thread *t2)
{
	if (t1->getPriority() == t2->getPriority())
	{
		if (t1->timeInReadyQueue().tv_sec < t2->timeInReadyQueue().tv_sec)
		{
			return true; /* Less than. */
		}
		else if (t1->timeInReadyQueue().tv_sec > t2->timeInReadyQueue().tv_sec)
		{
			return false; /* Greater than. */
		}
		else if (t1->timeInReadyQueue().tv_usec < t2->timeInReadyQueue().tv_usec)
		{
			return true; /* Less than. */
		}
		else if (t1->timeInReadyQueue().tv_usec > t2->timeInReadyQueue().tv_usec)
		{
			return false; /* Greater than. */
		}
		else
		{
			return false; /* Equal. Cannot happen. */
		}
	}

	return t1->getPriority() < t2->getPriority();
}

/**
 * @brief PQ constructor
 */
PriorityQueue::PriorityQueue()
{
	_ready = new std::list<Thread*>();
}

/**
 * @brief PQ destructor
 */
PriorityQueue::~PriorityQueue()
{
	delete _ready;
}

/**
 * @brief Delete top thread from queue
 */
void PriorityQueue::pop()
{
	if (!_ready->empty())
	{
		_ready->pop_front();
		_ready->sort(_compare);
	}
}

/**
 * @brief Get top thread in the queue
 * @return the top thread in the queue
 */
Thread *PriorityQueue::top()
{
	if (!_ready->empty())
	{
		return _ready->front();
	}

	return NULL;
}

/**
 * @brief Pushes in a thread into the queue
 * @param Thread *thread - the pointer of a thread to be pushed
 */
void PriorityQueue::push(Thread *thread)
{
	_ready->push_back(thread);
	_ready->sort(_compare);
}

/**
 * @brief Decorator for PQ size
 */
int PriorityQueue::size()
{
	return _ready->size();
}

/**
 * @brief Erase specific thread within the queue
 * @param Thread *thread - the pointer of a thread to be erased
 */
void PriorityQueue::erase(Thread *thread)
{
	for (std::list<Thread*>::iterator it = _ready->begin(); it != _ready->end(); ++it)
	{
		if ((*it)->getTid() == thread->getTid())
		{
			_ready->erase(it);
			_ready->sort(_compare);
			break;
		}
	}
}