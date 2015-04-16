#include <vector>
#include "thread.hpp"

class ReadyQueue
{
public:
	ReadyQueue();
	~ReadyQueue();

	void pop();
	Thread *top();
	void push(Thread *thread);
	
	std::vector<Thread*> red;
	std::vector<Thread*> orange;
	std::vector<Thread*> green;
};