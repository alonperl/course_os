#include "SignalManager.hpp"
#include "Scheduler.hpp"

sigset_t SignalManager::_blockedSignals;
sigset_t SignalManager::_pendingSignals;

void SignalManager::ignoreSignals()
{
	signal(SIGVTALRM, SIG_IGN);
}

void SignalManager::postponeSignals()
{
	sigfillset(&_blockedSignals);
	sigprocmask(SIG_BLOCK, &_blockedSignals, NULL);
}

void SignalManager::unblockSignals()
{
	sigfillset(&_blockedSignals);
	sigprocmask(SIG_UNBLOCK, &_blockedSignals, NULL);
}

bool SignalManager::hasTimerSignalTriggered()
{
	sigemptyset(&_pendingSignals);
	sigpending(&_pendingSignals);
	return sigismember(&_pendingSignals, SIGVTALRM);
}

void SignalManager::startTimer(itimerval *quantum)
{
	signal(SIGVTALRM, staticSignalHandler);
	setitimer(ITIMER_VIRTUAL, quantum, NULL);
}

void SignalManager::stopTimer()
{
	ignoreSignals();
	struct itimerval reset = {0, 0, 0, 0};
	setitimer(ITIMER_VIRTUAL, &reset, NULL);
}

void SignalManager::staticSignalHandler(int sig)
{
	(void)(sig); // Unused, suppress compiler warning
	(*Scheduler::getInstance()).switchThreads(READY);
}