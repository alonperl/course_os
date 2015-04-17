#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>

#include <cstdlib>
#include <list>
#include <iostream>

#include "uthreads.h"
#include "Thread.hpp"

template <typename Type, typename Compare>
class PQueue
{
	public:
		Type get();
		void set(Type t);

	private:
		std::list<Type*> list;
		Compare comp;
};

template <typename Type, typename Compare>
void PQueue<Type, Compare>::set(Type t)
{
	list.push_front(t);
	list.sort(comp);
}

template <typename Type, typename Compare>
Type PQueue<Type, Compare>::get()
{
	return list.front();
}

struct CompareThreads
{
	bool operator()(Thread *t1, Thread *t2)
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
};

int main()
{
	PQueue<Thread*, CompareThreads> pq;
	Thread *t = new Thread(NULL, ORANGE, 0);
	pq.set(t);
	printf("%d", pq.get()->getTid());
	return 0;
}