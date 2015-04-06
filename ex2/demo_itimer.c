/*
 * Interval-timer demo program.
 * Hebrew University OS course.
 * Questions: os@cs.huji.ac.il
 */

#include <stdio.h>
#include <signal.h>
#include <sys/time.h>

int gotit = 0;

void timer_handler(int sig)
{
  gotit = 1;
  printf("Using timer_handler\n", sig);
}

void th2(int sig)
{
  static int times = 0;
  printf("Using th2 ");
  printf("%d\n", times++);
  if (times % 5 == 0)
  {
    signal(SIGVTALRM, timer_handler);
  }
}

int main(void)
{
  signal(SIGVTALRM, timer_handler);

  struct itimerval tv;
  tv.it_value.tv_sec = 2;  /* first time interval, seconds part */
  tv.it_value.tv_usec = 0; /* first time interval, microseconds part */
  tv.it_interval.tv_sec = 2;  /* following time intervals, seconds part */
  tv.it_interval.tv_usec = 0; /* following time intervals, microseconds part */

  setitimer(ITIMER_VIRTUAL, &tv, NULL);
  for(;;) {
    if (gotit) {
      signal(SIGVTALRM, th2);
      gotit = 0;
    }
  }

  printf("finished\n");
  return 0;
}

