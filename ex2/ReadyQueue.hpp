#include <list>
#include "thread.hpp"

class ReadyQueue
{
public:
	int size();

	void pop();
	Thread *top();
	void push(Thread *thread);
	void erase(Thread *thread);

	std::list<Thread*> ready;
};