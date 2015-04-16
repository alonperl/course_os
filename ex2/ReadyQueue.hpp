#include <list>
#include "thread.hpp"

struct ThreadComparator
{
	bool operator()(Thread *t1, Thread *t2);
};

class ReadyQueue
{
public:
	int size();

	void pop();
	Thread *top();
	void push(Thread *thread);
	void erase(Thread *thread);

	// std::list<Thread*>::iterator iterator(Priority priority);

	std::list<Thread*> ready;
};