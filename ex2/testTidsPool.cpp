#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>

#include <cstdlib>
#include <list>
#include <iostream>

#include "Thread.hpp"
#include "Scheduler.hpp"

int main(int argc, char const *argv[])
{
	printf("Top %dn", Scheduler::getInstance()->getTidsPool()->top());

	Scheduler::getInstance()->getTidsPool()->push(1);
	Scheduler::getInstance()->getTidsPool()->push(2);

	printf("Top %d\n", Scheduler::getInstance()->getTidsPool()->top());
	Scheduler::getInstance()->getTidsPool()->pop();
	printf("Pop\n");
	printf("Top %d\n", Scheduler::getInstance()->getTidsPool()->top());
	Scheduler::getInstance()->getTidsPool()->pop();
	printf("Pop\n");
	printf("Top %d\n", Scheduler::getInstance()->getTidsPool()->top());
	return 0;
}