/* Example of using sigaction() to setup a signal handler with 3 arguments
 * including siginfo_t.
 */
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
 
static void hdl (int sig, siginfo_t *siginfo, void *context)
{
	printf ("Sending PID: %ld, UID: %ld\n",
			(long)siginfo->si_pid, (long)siginfo->si_uid);
}
 
int main (int argc, char *argv[])
{

	struct itimerval timer;

	/* Configure the timer to expire after 250 msec... */
	timer.it_value.tv_sec = 0;
	timer.it_value.tv_usec = 250000;
	/* ... and every 250 msec after that. */
	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = 250000;
	/* Start a virtual timer. It counts down whenever this process is
	executing. */

	struct sigaction act;
 
	memset (&act, '\0', sizeof(act));
 
	/* Use the sa_sigaction field because the handles has two additional parameters */
	act.sa_sigaction = &hdl;
 
	/* The SA_SIGINFO flag tells sigaction() to use the sa_sigaction field, not sa_handler. */
	act.sa_flags = SA_SIGINFO;
	setitimer (ITIMER_VIRTUAL, &timer, NULL);
 
	if (sigaction(SIGVTALRM, &act, NULL) < 0) {
		perror ("sigaction");
		return 1;
	}
 
	while (1)
		sleep (10);
 
	return 0;
}

