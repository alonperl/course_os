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
	uthreads_init(100);
	uthreads_spawn(NULL, ORANGE);
	uthreads_spawn(NULL, ORANGE);
	uthreads_spawn(NULL, ORANGE);
	uthreads_spawn(NULL, ORANGE);
	uthreads_terminate(1);
	printf("New thread %d\n", uthreads_spawn(NULL, ORANGE));
	uthreads_terminate(2);
	printf("New thread %d\n", uthreads_spawn(NULL, ORANGE));
	uthreads_terminate(3);
	printf("New thread %d\n", uthreads_spawn(NULL, ORANGE));

	uthreads_terminate(0);
}