#include "ReadyQueue.hpp"

void ReadyQueue::pop()
{
	if (!red.empty())
	{
		red.pop_front();
		return;
	}

	if (!orange.empty())
	{
		orange.pop_front();
		return;
	}

	if (!green.empty())
	{
		green.pop_front();
		return;
	}
}

Thread *ReadyQueue::top()
{
	if (!red.empty())
	{
		return red.front();
	}

	if (!orange.empty())
	{
		return orange.front();
	}

	if (!green.empty())
	{
		return green.front();
	}
}

void ReadyQueue::push(Thread *thread)
{
	switch (thread->getPriority())
	{
		case RED:
			red.push_back(thread);
			break;
	
		case ORANGE:
			orange.push_back(thread);
			break;
	
		case GREEN:
			green.push_back(thread);
			break;
	}
}

int ReadyQueue::size()
{
	return red.size() + orange.size() + green.size();
}

std::deque<Thread*>::iterator ReadyQueue::iterator(Priority priority)
{
	switch (priority)
	{
		case RED:
			return red.begin();
	
		case ORANGE:
			return orange.begin();
	
		case GREEN:
			return green.begin();
	}
}

void ReadyQueue::erase(Thread *thread)
{
	// std::deque<Thread*>::iterator iterator = iterator(thread->getPriority());

	// for (; (*iterator)->getTid() != thread->getTid(); iterator++);
	int i;
	switch (thread->getPriority())
	{
		case RED:
			for (i = 0; red[i]->getTid() == thread->getTid(); ++i)
			{
				red.erase(iterator);
				break;
			}
			break;
	
		case ORANGE:
			for (i = 0; orange[i]->getTid() == thread->getTid(); ++i)
			{
				orange.erase(iterator);
				break;
			}
			break;
	
		case GREEN:
			for (i = 0; green[i]->getTid() == thread->getTid(); ++i)
			{
				green.erase(iterator);
				break;
			}
			break;
	}
}