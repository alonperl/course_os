/**
 * @file SignalManager.hpp
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
 * A signal manager class to be able to handle the signals in organized way.
 */
#ifndef _SIGNAL_MANAGER_H
#define _SIGNAL_MANAGER_H

#include <signal.h>
#include <string>
#include <sys/time.h>

/**
 * @brief Signal manager provides simple API to set and remove timer for
 * virtual alarm, set and remove signal handling, and check if virtual 
 * alarm fired.
 */
class SignalManager
{
public:
	/**
	 * @brief Block all signals
	 */
	static void postponeSignals();

	/**
	 * @brief Unblock all signals
	 */
	static void unblockSignals();

	/**
	 * @brief A handler that calls to Scheduler switchThreads()
	 * 
	 * @param sig signal number
	 */
	static void staticSignalHandler(int sig);

	/**
	 *  @return true iff virtual alarm was triggered in the time 
	 			the signals were postponed.
	 */
	static bool hasTimerSignalTriggered();

	/**
	 *  @brief Starts virtual timer
	 *  @param itimerval *quantum - pointer to the time qunatom to initate the
	 *								the time by.
	 */
	static void startTimer(itimerval *quantum);
	
	/**
	 *  @brief Stops virtual timer
	 */
	static void stopTimer();

	/**
	 * @brief A system calls error handler
	 * 
	 * @param err error message 
	 */
	static void systemErrorHandler(const char *err);

	/* Utility masks */
	static sigset_t _blockedSignals;
	static sigset_t _pendingSignals;
};

#endif