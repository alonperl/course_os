#include "Thread.hpp"

#ifdef __x86_64__
/* code for 64 bit Intel arch */

typedef unsigned long address_t;
#define JB_SP 6
#define JB_PC 7

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
	address_t ret;
	asm volatile("xor	%%fs:0x30,%0\n"
		"rol	$0x11,%0\n"
				 : "=g" (ret)
				 : "0" (addr));
	return ret;
}

#else
/* code for 32 bit Intel arch */

typedef unsigned int address_t;
#define JB_SP 4
#define JB_PC 5 

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
	address_t ret;
	asm volatile("xor	%%gs:0x18,%0\n"
		"rol	$0x9,%0\n"
				 : "=g" (ret)
				 : "0" (addr));
	return ret;
}

#endif

Thread::Thread(void (*f)(void), Priority pr, unsigned int newTid)
{
	_tid = newTid;

	if (!isValidPriority(pr))
	{
		throw FAIL;
	}

	address_t stackPointer = (address_t)_stack + STACK_SIZE - sizeof(address_t);
	_programCounter = (address_t)f;
	sigsetjmp(_threadEnv, 1);
	(_threadEnv->__jmpbuf)[JB_SP] = translate_address(stackPointer);
	(_threadEnv->__jmpbuf)[JB_PC] = translate_address(_programCounter);
	sigemptyset(&_threadEnv->__saved_mask);

	_priority = pr;
	_quantums = 0;
	_state = NEW;
}

Priority Thread::getPriority()
{
	return _priority;
}

address_t Thread::getProgramCounter()
{
	return _programCounter;
}

struct timeval Thread::getReadyFrom()
{
	return _readyFrom;
}

State Thread::getState()
{
	return _state;
}

void Thread::setState(State st)
{
	_state = st;
}

unsigned int Thread::getQuantums()
{
	return _quantums;
}

unsigned int Thread::getTid()
{
	return _tid;
}

sigjmp_buf *Thread::getEnv()
{
	return &_threadEnv;
}

bool Thread::isValidPriority(Priority pr)
{
	switch (pr)
	{
		case RED:
		case ORANGE:
		case GREEN:
			return true;

		default:
			return false;
	}
}

void Thread::incrementQuantums()
{
	_quantums++;
}

void Thread::setReadyFrom()
{
	struct timeval ts;
	int result = gettimeofday(&ts, NULL);
	
	if (result != 0)
	{
		throw FAIL;
	}
	
	_readyFrom = ts;
}

void Thread::resetReadyFrom()
{
	_readyFrom.tv_sec = 0;
	_readyFrom.tv_usec = 0;
}

Thread::~Thread()
{

}