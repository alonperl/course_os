#ifndef _STATES_H
#define _STATES_H

#include "thread.hpp"

class States
{
public:
	int ready(Thread *t);
	int block(Thread *t);
	Thread *getThread(int tid);
	Thread *running();
};

#endif