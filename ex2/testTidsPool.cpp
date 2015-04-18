#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>

#include <cstdlib>
#include <list>
#include <iostream>

#include "uthreads.h"
#include "Thread.hpp"
#include "Scheduler.hpp"
#include "SignalManager.hpp"

int main(int argc, char const *argv[])
{
	uthread_init(100);
	printf("New thread %d\n", uthread_spawn(NULL, ORANGE));
	printf("New thread %d\n", uthread_spawn(NULL, ORANGE));
	printf("New thread %d\n", uthread_spawn(NULL, ORANGE));

	uthread_terminate(1);
	printf("New thread %d\n", uthread_spawn(NULL, ORANGE));
	uthread_terminate(2);
	printf("New thread %d\n", uthread_spawn(NULL, ORANGE));
	uthread_terminate(3);
	printf("New thread %d\n", uthread_spawn(NULL, ORANGE));

	uthread_terminate(1);
	uthread_terminate(2);
	uthread_terminate(3);
	printf("New thread %d\n", uthread_spawn(NULL, ORANGE));
	printf("New thread %d\n", uthread_spawn(NULL, ORANGE));
	printf("New thread %d\n", uthread_spawn(NULL, ORANGE));

	printf("New thread %d\n", uthread_spawn(NULL, ORANGE));
	uthread_terminate(0);
}