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

using namespace std;

int main(int argc, char const *argv[])
{
	cout << "Top " << Scheduler::getInstance()->getTidsPool()->top() << endl;

	Scheduler::getInstance()->getTidsPool()->push(1);
	Scheduler::getInstance()->getTidsPool()->push(2);

	cout << "Top " << Scheduler::getInstance()->getTidsPool()->top() << endl;
	cout << "Pop " << Scheduler::getInstance()->getTidsPool()->pop() << endl;
	cout << "Top " << Scheduler::getInstance()->getTidsPool()->top() << endl;
	cout << "Pop " << Scheduler::getInstance()->getTidsPool()->pop() << endl;
	cout << "Top " << Scheduler::getInstance()->getTidsPool()->top() << endl;
	return 0;
}