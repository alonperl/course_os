#include <list>
#include "Thread.hpp"

class PriorityQueue
{
public:
	PriorityQueue();
	~PriorityQueue();

	int size();

	void pop();
	Thread *top();
	void push(Thread *thread);
	void erase(Thread *thread);

private:
	std::list<Thread*> _ready;
};