/**
 * @file Thread.cpp
 * @author  griffonn ednussi
 * @version 1.0
 * @date 18 April 2015
 * 
 * @brief Thread API implementation.
 *
 * @section LICENSE
 * This program is a free software. You can freely redistribute it.
 *
 * @section DESCRIPTION
 * Implements all the API functions, and also hardware-specific features.
 */
#include <iostream>
#include "Thread.hpp"

#define RESUMING 1
#define SUCCESS 0
#define FAIL -1
#define RETURN_FAIL 1

#ifndef _SYSERR_MESSAGES
#define SYSERR "system error: "
#define SYSERR_SIGEMPTYSET "couldn't empty set\n"
#endif

/* Hardware related */
#ifdef __x86_64__
/* code for 64 bit Intel arch */

typedef unsigned long address_t;
#define JB_SP 6
#define JB_PC 7

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translateAddress(address_t addr)
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
address_t translateAddress(address_t addr)
{
	address_t ret;
	asm volatile("xor	%%gs:0x18,%0\n"
		"rol	$0x9,%0\n"
				 : "=g" (ret)
				 : "0" (addr));
	return ret;
}

#endif


/**
 * @brief Thread constructor
 * 
 * @param f Start routine
 * @param pr Priority - as defined in uthreads library. Cannot be changed.
 * @param int Thread ID. Cannot be changed
 */
Thread::Thread(void (*f)(void), Priority pr, unsigned int newTid)
{
	_tid = newTid;

	if (!_isValidPriority(pr))
	{
		throw FAIL;
	}

	address_t stackPointer = (address_t)_stack + STACK_SIZE - sizeof(address_t);
	_programCounter = (address_t)f;
	sigsetjmp(_threadEnv, RESUMING);
	(_threadEnv->__jmpbuf)[JB_SP] = translateAddress(stackPointer);
	(_threadEnv->__jmpbuf)[JB_PC] = translateAddress(_programCounter);
	if (sigemptyset(&_threadEnv->__saved_mask) == FAIL)
	{
		std::cerr << SYSERR << SYSERR_SIGEMPTYSET << std::endl;
		exit(RETURN_FAIL);
	}

	_priority = pr;
	_quantums = 0;
	_state = NEW;
}

/**
 * @return thread's priority
 */
Priority Thread::getPriority()
{
	return _priority;
}

/**
 * @return current program counter
 */
address_t Thread::getProgramCounter()
{
	return _programCounter;
}

/**
 * @return the time this thread last entered ready PQ
 */
struct timeval Thread::timeInReadyQueue()
{
	return _readyFrom;
}

/**
 * @return current state
 */
State Thread::getState()
{
	return _state;
}

/**
 * @brief Change state to given
 * 
 * @param st new state
 */
void Thread::setState(State st)
{
	_state = st;
}

/**
 * @return quantums this thread runned till now
 */
unsigned int Thread::getQuantums()
{
	return _quantums;
}

/**
 * @return Thread tid
 */
unsigned int Thread::getTid()
{
	return _tid;
}

/**
 * @return thread's environment
 */
sigjmp_buf *Thread::getEnv()
{
	return &_threadEnv;
}

/**
 * @brief Check if given priority is a valid Priority
 * as set in uthreads library
 * 
 * @param pr Priority
 * @return true iff given priority is valid
 */
bool Thread::_isValidPriority(Priority pr)
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

/**
 *  @brief Increment the number of qunatums for the thread
 */
void Thread::incrementQuantums()
{
	_quantums++;
}

/**
 * @brief Set timestamp _readyFrom to now
 */
void Thread::setTimeInReadyQueue()
{
	struct timeval ts;
	int timestampSucceed = gettimeofday(&ts, NULL);
	
	if (timestampSucceed != SUCCESS)
	{
		throw FAIL;
	}
	
	_readyFrom = ts;
}

/**
 * @brief Reset timestamp _readyFrom
 */
void Thread::resetTimeInReadyQueue()
{
	_readyFrom.tv_sec = 0;
	_readyFrom.tv_usec = 0;
}