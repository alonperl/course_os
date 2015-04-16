#include "ReadyQueue.hpp"

bool compareThreads(Thread *t1, Thread *t2) {
	if (t1->getPriority() == t2->getPriority()) {
		if (t1->getReadyFrom().tv_sec > t2->getReadyFrom().tv_sec)
			return true; /* Less than. */
		else if (t1->getReadyFrom().tv_sec < t2->getReadyFrom().tv_sec)
			return false; /* Greater than. */
		else if (t1->getReadyFrom().tv_usec > t2->getReadyFrom().tv_usec)
			return true; /* Less than. */
		else if (t1->getReadyFrom().tv_usec < t2->getReadyFrom().tv_usec)
			return false; /* Greater than. */
		else
			return false; /* Equal. Cannot happen. */
	}

	// TODO: Recheck conditions:
	return t1->getPriority() > t2->getPriority();
}

void ReadyQueue::pop()
{
	if (!ready.empty())
	{
		ready.pop_front();
		ready.sort(compareThreads);
	}
}

Thread *ReadyQueue::top()
{
	if (!ready.empty())
	{
		return ready.front();
	}
}

void ReadyQueue::push(Thread *thread)
{
	ready.push_back(thread);
	ready.sort(compareThreads);
}

int ReadyQueue::size()
{
	return ready.size();
}

void ReadyQueue::erase(Thread *thread)
{
	for (std::list<Thread*>::iterator it=ready.begin(); it != ready.end(); ++it)
	{
		if ((*it)->getTid() == thread->getTid())
		{
			ready.erase(it);
			ready.sort(compareThreads);
			break;
		}
	}
}