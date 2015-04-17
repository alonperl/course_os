#include <list>
#include "Thread.hpp"

class ReadyQueue
{
public:
	int size();

	void pop();
	Thread *top();
	void push(Thread *thread);
	void erase(Thread *thread);

private:
	std::list<Thread*> _ready;
};