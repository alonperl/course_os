#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
void h(int sig)
{
	printf("got\n");
}
main()
{
   sigset_t omvsManaged, sascManaged, pendingSignals, newMask, oldMask;


   sigfillset(&omvsManaged);        /* Request OpenEdition management */
   sigemptyset(&sascManaged);       /* of all signals.                */
   sigemptyset(&pendingSignals);
   sigemptyset(&newMask);
   sigemptyset(&oldMask);

      /* Tell the system which signals should be managed by SAS/C     */
      /*  and which by OpenEdition.                                   */

      /* Block the SIGHUP signal.                                     */
   sigaddset(&newMask, SIGVTALRM);

	sigprocmask(SIG_BLOCK, &newMask, &oldMask);

      kill(0, SIGVTALRM);
      // kill(0, SIGVTALRM);

      /* Check to see if SIGHUP is pending.                           */
   sigpending(&pendingSignals);
   if (sigismember(&pendingSignals, SIGVTALRM)){

      printf("pending sig\n");

   }
   else
   {
      printf("nothing\n");
   }

      /* Restore the old mask.                                        */
   sigprocmask(SIG_SETMASK, &oldMask, NULL);
   
}