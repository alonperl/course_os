/*
 * sigsetjmp/siglongjmp demo program.
 * Hebrew University OS course.
 * Questions: os@cs.huji.ac.il
 */

#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>

#define SECOND 1000000
#define STACK_SIZE 4096

char stack1[STACK_SIZE];
char stack2[STACK_SIZE];
char stack3[STACK_SIZE];

sigjmp_buf env[3];

#ifdef __x86_64__
/* code for 64 bit Intel arch */

typedef unsigned long address_t;
#define JB_SP 6
#define JB_PC 7
int gotit = 0;
/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%fs:0x30,%0\n"
		"rol    $0x11,%0\n"
                 : "=g" (ret)
                 : "0" (addr));
    return ret;
}

#else
/* code for 32 bit Intel arch */

typedef unsigned int address_t;
#define JB_SP 4
#define JB_PC 5 

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%gs:0x18,%0\n"
		"rol    $0x9,%0\n"
                 : "=g" (ret)
                 : "0" (addr));
    return ret;
}

#endif


void th(int sig)
{
  printf("enter th\n");
  gotit = 1;
}


void switchThreads()
{
  static int currentThread = 0;

  int ret_val = sigsetjmp(env[currentThread],0);
  printf("SWITCH: ret_val=%d\n", ret_val); 
  if (ret_val == 1) {
      return;
  }
  currentThread = (currentThread + 1) % 3;
  printf("Thread %d\n", currentThread);
  siglongjmp(env[currentThread],1);
}

void f(void)
{
  struct itimerval tv0;
  tv0.it_value.tv_sec = 0;  /* first time interval, seconds part */
  tv0.it_value.tv_usec = 0; /* first time interval, microseconds part */
  tv0.it_interval.tv_sec = 0;  /* following time intervals, seconds part */
  tv0.it_interval.tv_usec = 0; /* following time intervals, microseconds part */
  
  struct itimerval tv;
  tv.it_value.tv_sec = 2;  /* first time interval, seconds part */
  tv.it_value.tv_usec = 0; /* first time interval, microseconds part */
  tv.it_interval.tv_sec = 2;  /* following time intervals, seconds part */
  tv.it_interval.tv_usec = 0; /* following time intervals, microseconds part */
  
  signal(SIGVTALRM, th);

  setitimer(ITIMER_VIRTUAL, &tv0, NULL);
  setitimer(ITIMER_VIRTUAL, &tv, NULL);

  int i = 0;
  while(1){
    ++i;
    printf("in f (%d)\n",i);
    if (gotit)
    {
      gotit = 0;
      switchThreads();
    }
    usleep(SECOND);
  }
}

/**
void g(void)
{
  int i = 0;
  while(1){
    ++i;
    printf("in g (%d)\n",i);
    if (i % 5 == 0) {
      printf("g: switching\n");
      switchThreads();
    }
    usleep(SECOND);
  }
}*/

void setup(void)
{
  address_t sp, pc;

  sp = (address_t)stack1 + STACK_SIZE - sizeof(address_t);
  pc = (address_t)f;
  sigsetjmp(env[0], 1);
  (env[0]->__jmpbuf)[JB_SP] = translate_address(sp);
  (env[0]->__jmpbuf)[JB_PC] = translate_address(pc);
  sigemptyset(&env[0]->__saved_mask);     

  sp = (address_t)stack2 + STACK_SIZE - sizeof(address_t);
  pc = (address_t)f;
  sigsetjmp(env[1], 1);
  (env[1]->__jmpbuf)[JB_SP] = translate_address(sp);
  (env[1]->__jmpbuf)[JB_PC] = translate_address(pc);
  sigemptyset(&env[1]->__saved_mask);         

  sp = (address_t)stack3 + STACK_SIZE - sizeof(address_t);
  pc = (address_t)f;
  sigsetjmp(env[2], 1);
  (env[2]->__jmpbuf)[JB_SP] = translate_address(sp);
  (env[2]->__jmpbuf)[JB_PC] = translate_address(pc);
  sigemptyset(&env[2]->__saved_mask);         

}

int main(void)
{
  setup();
  siglongjmp(env[0], 1);

  for(;;) {
    if (gotit) {
      printf("switching?\n");
      switchThreads();
      gotit = 0;
    }
  }
  return 0;
}

