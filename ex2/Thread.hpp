/**
 * @file Thread.hpp
 * @author  griffonn ednussi
 * @version 1.0
 * @date 18 April 2015
 * 
 * @brief Thread class and API definition.
 *
 * @section LICENSE
 * This program is a free software. You can freely redistribute it.
 *
 * @section DESCRIPTION
 * Defines a unique enum for possible thread states as far as the pripority 
 * enum. Includes all of the thread unqiue imprinted information and functions
 * to work with.
 */
#ifndef _THREAD_H
#define _THREAD_H

#include <cstdlib>
#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include <sys/time.h>
#ifndef _UTHREADS_H
#include "uthreads.h"
#endif

/* Possible Thread states */
typedef enum State
{ 
	READY, 
	RUNNING, 
	BLOCKED, 
	NEW 
} State;

typedef unsigned long address_t;

/**
 * @brief This class represents single thread, its state and methods
 * 
 * @param f Start routine
 * @param pr Priority - as defined in uthreads library. Cannot be changed.
 * @param int Thread ID. Cannot be changed
 */
class Thread
{
public:
	Thread(void (*f)(void), Priority pr, unsigned int newTid);
	
	/**
	 * @return Thread tid
	 */
	unsigned int getTid();

	/**
	 * @return current program counter
	 */
	address_t getProgramCounter();

	/**
	 * @return thread's priority
	 */
	Priority getPriority();
	
	/**
	 * @return current state
	 */
	State getState();

	/**
	 * @return quantums this thread runned till now
	 */
	unsigned int getQuantums();

	/**
	 * @return the time this thread last entered ready PQ
	 */
	struct timeval timeInReadyQueue();

	/**
	 * @return thread's environment
	 */
	sigjmp_buf *getEnv();

	/**
	 * @brief Set timestamp _readyFrom to now
	 */
	void setTimeInReadyQueue();

	/**
	 * @brief Reset timestamp _readyFrom
	 */
	void resetTimeInReadyQueue();

	/**
	 * @brief Change state to given
	 * 
	 * @param st new state
	 */
	void setState(State st);

	/**
	 *  @brief Increment the number of qunatums for the thread
	 */
	void incrementQuantums();

private:
	sigjmp_buf _threadEnv;
	char _stack[STACK_SIZE];
	address_t _programCounter;
	Priority _priority;
	State _state;

	struct timeval _readyFrom;
	unsigned int _quantums;
	unsigned int _tid;

	/**
	 * @brief Check if given priority is a valid Priority
	 * as set in uthreads library
	 * 
	 * @param pr Priority
	 * @return true iff given priority is valid
	 */
	bool _isValidPriority(Priority pr);
};

#endif