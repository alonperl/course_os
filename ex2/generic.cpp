#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>

#include <cstdlib>
#include <list>
#include <iostream>

#include "uthreads.h"
#include "Thread.hpp"

template <class Type, bool (*compare)(const Type *t1, const Type *t2)>
class PQueue
{
	template <Type, (*compare)(const Type *t1, const Type *t2)>
	Type get();

	template <Type, (*compare)(const Type *t1, const Type *t2)>
	void set(Type t);

	std::list<Type*> list;
};

template <class Type, bool (*compare)(const Type *t1, const Type *t2)>
void PQueue<Type, compare>::set(Type t)
{
	list.push_front(t);
}

template <class Type, bool (*compare)(const Type *t1, const Type *t2)>
Type PQueue<Type, compare>::get(Type t)
{
	return list.front();
}

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

int main()
{
	PQueue<Thread*, compareThreads> pq;
	Thread *t = new Thread(NULL, ORANGE, )
	pq.set(t);
	printf("%d", pq.get()->getTid());
	return 0;
}