/**
 * @file SignalManager.cpp
 * @author  griffonn ednussi
 * @version 1.0
 * @date 18 April 2015
 * 
 * @brief A class with static methods to control the signal handling.
 *
 * @section LICENSE
 * This program is a free software. You can freely redistribute it.
 *
 * @section DESCRIPTION
 * Implementation of signal manager class to be able to handle the 
 * signals in organized way.
 */
#include <iostream>
#include "SignalManager.hpp"
#include "Scheduler.hpp"

#define FAIL -1
#define RETURN_FAIL 1

#ifndef _SYSERR_MESSAGES
#define _SYSERR_MESSAGES
#define SYSERR "system error: "
#define SYSERR_SIGFILLSET "couldn't  fill set\n"
#define SYSERR_SIGEMPTYSET "couldn't empty set\n"
#define SYSERR_SIGPROCMASK "couldn't change blocked signals\n"
#define SYSERR_SIGPENDING "couldn't fetch pending signals\n"
#define SYSERR_SIGNAL "couldn't set signal handler\n"
#define SYSERR_SIGISMEMBER "couldn't check signal membership in mask\n"
#define SYSERR_SETITIMER "couldn't set timer\n"

#endif

sigset_t SignalManager::_blockedSignals;
sigset_t SignalManager::_pendingSignals;

/**
 * @brief Block all signals
 */
void SignalManager::postponeSignals()
{
	if (sigfillset(&_blockedSignals) == FAIL)
	{
		systemErrorHandler(SYSERR_SIGFILLSET);
	}

	if (sigprocmask(SIG_BLOCK, &_blockedSignals, NULL) == FAIL)
	{
		systemErrorHandler(SYSERR_SIGPROCMASK);
	}
}

/**
 * @brief Unblock all signals
 */
void SignalManager::unblockSignals()
{
	if (sigfillset(&_blockedSignals) == FAIL)
	{
		systemErrorHandler(SYSERR_SIGFILLSET);
	}

	if (sigprocmask(SIG_UNBLOCK, &_blockedSignals, NULL) == FAIL)
	{
		systemErrorHandler(SYSERR_SIGPROCMASK);
	}
}

/**
 *  @return true iff virtual alarm was triggered in the time 
 *			the signals were postponed.
 */
bool SignalManager::hasTimerSignalTriggered()
{
	if (sigemptyset(&_pendingSignals) == FAIL)
	{
		systemErrorHandler(SYSERR_SIGEMPTYSET);
	}

	sigpending(&_pendingSignals);
	if (sigpending(&_pendingSignals) == FAIL)
	{
		systemErrorHandler(SYSERR_SIGPENDING);
	}

	int virtualAlarmTriggered = sigismember(&_pendingSignals, SIGVTALRM);
	if (virtualAlarmTriggered == FAIL)
	{
		systemErrorHandler(SYSERR_SIGISMEMBER);
	}

	return virtualAlarmTriggered;
}

/**
 *  @brief Starts virtual timer
 *  @param itimerval *quantum - pointer to the time qunatom to initate the
 *								the time by.
 */
void SignalManager::startTimer(itimerval *quantum)
{
	if (signal(SIGVTALRM, staticSignalHandler) == SIG_ERR)
	{
		systemErrorHandler(SYSERR_SIGNAL);
	}

	if (setitimer(ITIMER_VIRTUAL, quantum, NULL) == FAIL)
	{
		systemErrorHandler(SYSERR_SETITIMER);
	}
}

/**
 *  @brief Stops virtual timer
 */
void SignalManager::stopTimer()
{
	if (signal(SIGVTALRM, SIG_IGN) == SIG_ERR)
	{
		systemErrorHandler(SYSERR_SIGNAL);
	}

	/**
	 * Empty timer interval struct for alarm reset 
	 */
	struct itimerval reset = {0, 0, 0, 0};
	

	if (setitimer(ITIMER_VIRTUAL, &reset, NULL) == FAIL)
	{
		systemErrorHandler(SYSERR_SETITIMER);
	}
}

/**
 * @brief A handler that calls to Scheduler switchThreads()
 * 
 * @param sig signal number
 */
void SignalManager::staticSignalHandler(int sig)
{
	(void)(sig); // Unused, suppress compiler warning
	(*Scheduler::getInstance()).switchThreads(READY);
}

/**
 * @brief A system calls error handler
 * 
 * @param err error message 
 */
void SignalManager::systemErrorHandler(const char *err)
{
	std::cerr << SYSERR << err;
	exit(RETURN_FAIL);
}
