#ifndef _SIGNAL_MANAGER_H
#define _SIGNAL_MANAGER_H

#include <signal.h>
#include <cstdlib>
#include <sys/time.h>

class SignalManager
{
public:
	static void postponeSignals();
	static void ignoreSignals();
	static void unblockSignals();

	static void staticSignalHandler((void)(sig));

	static bool hasTimerSignalTriggered();
	static void startTimer(itimerval *quantum);
	static void stopTimer();

	static sigset_t blockedSignals;
	static sigset_t pendingSignals;
};

#endif