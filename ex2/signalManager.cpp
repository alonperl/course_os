#include "signalManager.hpp"

sigset_t SignalManager::blockedSignals;
sigset_t SignalManager::pendingSignals;

void SignalManager::ignoreSignals()
{
	signal(SIGVTALRM, SIG_IGN);
}

void SignalManager::postponeSignals()
{
	sigfillset(&blockedSignals);
	sigprocmask(SIG_BLOCK, &blockedSignals, NULL);
}

void SignalManager::unblockSignals()
{
	sigfillset(&blockedSignals);
	sigprocmask(SIG_UNBLOCK, &blockedSignals, NULL);
}

bool SignalManager::hasTimerSignalTriggered()
{
	sigemptyset(&pendingSignals);
	sigpending(&pendingSignals);
	return sigismember(&pendingSignals, SIGVTALRM);
}

void SignalManager::startTimer(void (*handler)(int sig), itimerval *quantum)
{
	signal(SIGVTALRM, handler);
	setitimer(ITIMER_VIRTUAL, quantum, NULL);
}

void SignalManager::stopTimer()
{
	ignoreSignals();
	struct itimerval reset = {0, 0};
    setitimer(ITIMER_VIRTUAL, &reset, NULL);
}