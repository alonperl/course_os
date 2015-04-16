#include <deque>
#include "thread.hpp"

class ReadyQueue
{
public:
	int size();

	void pop();
	Thread *top();
	void push(Thread *thread);
	void erase(Thread *thread);

	std::deque<Thread*>::iterator iterator(Priority priority);

	std::deque<Thread*> red;
	std::deque<Thread*> orange;
	std::deque<Thread*> green;
};