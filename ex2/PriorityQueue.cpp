#include "PriorityQueue.hpp"

bool compareThreads(Thread *t1, Thread *t2)
{
	if (t1->getPriority() == t2->getPriority())
	{
		if (t1->getReadyFrom().tv_sec < t2->getReadyFrom().tv_sec)
			return true; /* Less than. */
		else if (t1->getReadyFrom().tv_sec > t2->getReadyFrom().tv_sec)
			return false; /* Greater than. */
		else if (t1->getReadyFrom().tv_usec < t2->getReadyFrom().tv_usec)
			return true; /* Less than. */
		else if (t1->getReadyFrom().tv_usec > t2->getReadyFrom().tv_usec)
			return false; /* Greater than. */
		else
			return false; /* Equal. Cannot happen. */
	}

	return t1->getPriority() < t2->getPriority();
}

PriorityQueue::~PriorityQueue()
{
	delete _ready;
}

void PriorityQueue::pop()
{
	if (!_ready.empty())
	{
		_ready.pop_front();
		_ready.sort(compareThreads);
	}
}

Thread *PriorityQueue::top()
{
	if (!_ready.empty())
	{
		return _ready.front();
	}

	return NULL;
}

void PriorityQueue::push(Thread *thread)
{
	_ready.push_back(thread);
	_ready.sort(compareThreads);
}

int PriorityQueue::size()
{
	return _ready.size();
}

void PriorityQueue::erase(Thread *thread)
{
	for (std::list<Thread*>::iterator it = _ready.begin(); it != _ready.end(); ++it)
	{
		if ((*it)->getTid() == thread->getTid())
		{
			_ready.erase(it);
			_ready.sort(compareThreads);
			break;
		}
	}
}