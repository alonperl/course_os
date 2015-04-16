#include <deque>
#include "thread.hpp"

class ReadyQueue
{
public:
	ReadyQueue();
	~ReadyQueue();

	void pop();
	Thread *top();
	void push(Thread *thread);
	
	std::deque<Thread*> red;
	std::deque<Thread*> orange;
	std::deque<Thread*> green;
};