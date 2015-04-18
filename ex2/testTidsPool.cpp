#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>

#include <cstdlib>
#include <list>
#include <iostream>

#include "Thread.hpp"
#include "Scheduler.hpp"
#include "SignalManager.hpp"

int main(int argc, char const *argv[])
{
	Scheduler *s = Scheduler::getInstance();
	
	s->setQuantum(100);

	printf("Top %dn", s->getTidsPool()->top());

	s->getTidsPool()->push(1);
	s->getTidsPool()->push(2);

	printf("Top %d\n", s->getTidsPool()->top());
	s->getTidsPool()->pop();
	printf("Pop\n");
	printf("Top %d\n", s->getTidsPool()->top());
	s->getTidsPool()->pop();
	printf("Pop\n");
	printf("Top %d\n", s->getTidsPool()->top());
	return 0;
}