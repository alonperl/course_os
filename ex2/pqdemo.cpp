#include <queue>
#include <cstdlib>
#include <cstdio>
#include "thread.hpp"

 void work()
 {
 	int k = 0;
 	k++;
 	printf("%d\n", k);
 }


struct ThreadComparator
{
	bool operator()(Thread *t1, Thread *t2);
};

bool ThreadComparator::operator()(Thread *t1, Thread *t2)
{
    if (t1->getPriority() == t2->getPriority())
    {
    	if (t1->getReadyFrom().tv_sec < t2->getReadyFrom().tv_sec)
    	        return true;				/* Less than. */
    	    else if (t1->getReadyFrom().tv_sec > t2->getReadyFrom().tv_sec)
    	        return false;				/* Greater than. */
    	    else if (t1->getReadyFrom().tv_usec < t2->getReadyFrom().tv_usec)
    	        return true;				/* Less than. */
    	    else if (t1->getReadyFrom().tv_usec > t2->getReadyFrom().tv_usec)
    	        return false;				/* Greater than. */
    	    else
    	        return false;				/* Equal. Cannot happen. */ 
    }

    return t1->getPriority() < t2->getPriority();
}

 int main(int argc, char const *argv[])
 {
	std::priority_queue<Thread*, std::vector<Thread*>, ThreadComparator> pq;

	Thread *th = new Thread(work, GREEN, 2);
	pq.push(new Thread(work, RED, 0));
	pq.push(new Thread(work, ORANGE, 1));
	pq.push(th);

	printf("pq size is %d\n", pq.size());

	// Remove thread 2 from pq
	printf("Removing thread %d from pq...\n", th->getTid());
	th->resetReadyFrom();
	pq.pop();

	while(!pq.empty())
	{
		Thread *t = pq.top();
		printf("Thread %d in pq with priority %d\n", t->getTid(), t->getPriority());
		pq.pop();
	}
 	return 0;
 }