//TODO: move this code to a scheduler object
//TODO: use exceptions, check return values, handle input errors.
//TODO: check errno after each system call.
//
// interesting links:
// regarding quantum count for a thread that blocked itself: http://moodle.cs.huji.ac.il/cs14/mod/forum/discuss.php?d=4285
// regarding blocking signals:
// http://moodle.cs.huji.ac.il/cs14/mod/forum/discuss.php?d=4444
// http://moodle.cs.huji.ac.il/cs14/mod/forum/discuss.php?d=4568
// regarding errors: http://moodle.cs.huji.ac.il/cs14/mod/forum/discuss.php?d=4484
//

#include "uthreads.h"
#include "Thread.h"

#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include <unordered_map>
#include <list>
#include <cstdlib>

using namespace std;

#define MICROS_PER_SECOND 1000000
#define SUCCESS 0
#define FAILED -1
#define MAIN_THREAD_ID 0
#define MIN_FREE_ID 1
// a macro to block all signals
#define BLOCK_SIGS sigprocmask(SIG_BLOCK, &blockSignalSet, NULL)
// a macro to unblock all signals
#define UNBLOCK_SIGS sigprocmask(SIG_UNBLOCK, &blockSignalSet, NULL)
int total = 0;
sigset_t blockSignalSet;
list<Thread*> readyRed;
list<Thread*> readyOrange;
list<Thread*> readyGreen;
list<Thread*> suspended;
Thread * running;
unordered_map<unsigned int, Thread*> threadMap;
unsigned int freeId = MIN_FREE_ID;
const struct itimerval reset;
struct itimerval tv;

void setTimer();
void switchThreads(int signum);
Thread * moveThread(int tid, Status status);
int moveThread(Thread * thread, Status status);

list<Thread*>* getList(Priority priority)
{
	switch (priority)
	{
		case GREEN:
		{
			return &readyGreen;
		}
		case ORANGE:
		{
			return &readyOrange;
		}
		default:
		{
			return &readyRed;
		}
	}
}

/*
 * Returns only if the ready queue of given priority is empty,
 * otherwise jump (siglongjmp) to the first-in-line thread with given priority.
 * Note that GREEN ready queue always contains the main thread, therefore will not return.
 * @param priority
 */
void goToNextReady(Priority priority)
{
	list <Thread*> * readyList = getList(priority);

	if (readyList->empty())
	{
		return;
	}

	Thread * nextThread = readyList->front();
	readyList->pop_front();
	moveThread(nextThread, RUNNING);
	siglongjmp(running->_env,1);
}

/*
 * This function will not return.
 * Jumps to the first-in-line thread with the highest priority
 */
void goToNextReady()
{
	goToNextReady(RED);
	goToNextReady(ORANGE);
	goToNextReady(GREEN);
}

// TODO: I think this should have a return value
// TODO: maybe this could be replaced by a macro
void removeTimer()
{
	if (setitimer(ITIMER_VIRTUAL, &reset, NULL))
	{
		return;
	}
}

// TODO: I think this should have a return value
void setTimer()
{
	if (sigsetjmp(running->_env, 1) == 1)
	{
		return;
	}
	if (setitimer(ITIMER_VIRTUAL, &tv, NULL))
	{
		return;
	}
}

// TODO: what to do if switchThreads fails? also - need to remove the parameter
void switchThreads (int signum)
{
	moveThread(running, READY);
	setTimer();
	goToNextReady();
}

// TODO: I think this should have a return value
void resetTimer()
{
	removeTimer();
	setTimer();
}

int uthread_init(int quantum_usecs)
{
	// set reset to the itimer reset value
	reset.it_value = {0, 0};

	// init the block set and check if the quantum_usecs is valid
	if (sigfillset(&blockSignalSet) || quantum_usecs <= 0)
	{
		return FAILED;
	}

	tv.it_value.tv_sec = quantum_usecs / MICROS_PER_SECOND;  /* first time interval, seconds part */
	tv.it_value.tv_usec = quantum_usecs % MICROS_PER_SECOND; /* first time interval, microseconds part */
	tv.it_interval.tv_sec = 0;  /* following time intervals, seconds part */
	tv.it_interval.tv_usec = 0; /* following time intervals, microseconds part */
	
	// register the switchThreads as the signal handler, meaning it would be called every quantum_usecs macro-seconds
	struct sigaction newAction;
	newAction.sa_handler = switchThreads;
	if (sigemptyset(&newAction.sa_mask))
	{
		return FAILED;
	}

	// TODO: should set SA_RESTART as a flag?
	// this says not to:
	// http://www.gnu.org/software/libc/manual/html_node/Setting-an-Alarm.html
	// this recommends using it:
	// http://www.gnu.org/software/libc/manual/html_node/Flags-for-Sigaction.html#Flags-for-Sigaction
	newAction.sa_flags = 0;

	resetTimer();
	if (sigaction(SIGVTALRM, &newAction, NULL))
	{
		return FAILED;
	}

	// according to the sigaction man page:
	// sa_mask specifies a mask of signals  which  should  be  blocked  (i.e.,
    // added  to  the signal mask of the thread in which the signal handler is
    // invoked) during execution of the signal handler.  In addition, the sig‐
    // nal  which triggered the handler will be blocked, unless the SA_NODEFER
    // flag is used.
	// TODO: is the signal ever unblocked? if not, a call to unblock the signal should be placed at the end of switchThreads

	running = new Thread();

	//start the timer which fires the signals which causes the switchThreads
	if (setitimer(ITIMER_VIRTUAL, &tv, NULL))
	{
		return FAILED;
	}

	return SUCCESS;
}


unsigned int _get_free_tid()
{
	unsigned int curFreeId = freeId;

	for (; (freeId < MAX_THREAD_NUM) && threadMap.count(freeId); freeId++) {}

	return curFreeId;
}

int uthread_spawn(void (*func)(void), Priority pr)
{
	if ((func == NULL) ||
		(readyRed.size() + readyOrange.size() + readyGreen.size() + suspended.size() + 1 >= MAX_THREAD_NUM))
	{
		return FAILED;
	}

	unsigned int tid = _get_free_tid();
	Thread * thread = new Thread(tid, func, pr);
	threadMap[tid] = thread;
	getList(pr)->push_back(thread);
	return tid;
}

int uthread_terminate(int tid)
{
	if (tid == MAIN_THREAD_ID)
	{
		// TODO: after the creation of the class, add a call to its destructor here
		exit(0);
	}
	if (tid < MIN_FREE_ID)
	{
		return FAILED;
	}

	unordered_map<unsigned int, Thread*>::const_iterator iter = threadMap.find(tid);
	if (iter == threadMap.end())
	{
		return FAILED;
	}
	delete iter->second;

	// update the freeId if its not the minimal anymore
	if (tid < (int) freeId)
	{
		freeId = tid;
	}

	if (tid == uthread_get_tid())
	{
		removeTimer();
		setTimer();
		goToNextReady();
	}

	return SUCCESS;
}

int moveThread(Thread * thread, Status status)
{
	switch (thread->_status)
	{
		case READY:
		{
			getList(thread->_priority)->remove(thread);
			break;
		}
		case SUSPENDED:
		{
			suspended.remove(thread);
			break;
		}
		case RUNNING:
		{
			break;
		}
		case TERMINATED:
		{
			return FAILED;
			break;
		}
	}
	thread->_status = status;
	switch (status)
	{
		case RUNNING:
		{
			running->_quantumNum++;
			total++;
			running = thread;
			break;
		}
		case READY:
		{
			getList(thread->_priority)->push_back(thread);
			break;
		}
		case SUSPENDED:
		{
			suspended.push_back(thread);
			break;
		}
		case TERMINATED:
		{
			threadMap.erase(thread->_id);
			delete thread;
		}
	}
	return SUCCESS;
}

Thread * moveThread(int tid, Status status)
{
	Thread * thread = threadMap[tid];
	if (moveThread(thread, status))
	{
		return NULL;
	}
	return thread;
}

/* Suspend a thread */
int uthread_suspend(int tid)
{
	resetTimer();
	Thread * thread = threadMap[tid];
	Status oldStatus = thread->_status;
	if (moveThread(thread, SUSPENDED))
	{
		return FAILED;
	}
	if (oldStatus == RUNNING)
	{
		goToNextReady();
	}
	return SUCCESS;
}

/* Resume a thread */
int uthread_resume(int tid)
{
	if (NULL == moveThread(tid, READY))
	{
		return FAILED;
	}
	return SUCCESS;
}

/* Get the id of the calling thread */
int uthread_get_tid()
{
	return running->_id;
}

/* Get the total number of library quantums */
int uthread_get_total_quantums()
{
	return total;
}

/* Get the number of thread quantums */
int uthread_get_quantums(int tid)
{
	return threadMap[tid]->_quantumNum;
}
