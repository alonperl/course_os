#include "ReadyQueue.hpp"

void ReadyQueue::pop()
{
	if (!ready.empty())
	{
		ready.pop_front();
	}
}

Thread *ReadyQueue::top()
{
	if (!ready.empty())
	{
		return red.front();
	}
}

void ReadyQueue::push(Thread *thread)
{
	ready.push_back(thread);
}

int ReadyQueue::size()
{
	return ready.size();
}

void ReadyQueue::erase(Thread *thread)
{
	ready.remove_if(thread->getTid() == getTid());
}