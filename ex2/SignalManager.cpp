#include "SignalManager.hpp"
#include "Scheduler.hpp"

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
	/*  Here, getInstance accepts int quantum_usecs for first init
		and doesn't use its argument at any other calls. Let's pass
		sig to it, as it is not used anyway. Yes, this is bad. */
	(*Scheduler::getInstance(sig)).switchThreads(READY);
}