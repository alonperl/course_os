/* Includes */
#include <unistd.h>     /* Symbolic Constants */
#include <sys/types.h>  /* Primitive System Data Types */ 
#include <errno.h>      /* Errors */
#include <stdio.h>      /* Input/Output */
#include <stdlib.h>     /* General Utilities */
#include <pthread.h>    /* POSIX Threads */
#include <string.h>     /* String handling */

/* prototype for thread routine */
void *print_message_function ( void *ptr );

/* struct to hold data to be passed to a thread
   this shows how multiple data items can be passed to a thread */
typedef struct str_thdata
{
    int thread_no;
    char message[100];
} thdata;


pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t c = PTHREAD_COND_INITIALIZER;
int done = 0;


    void thr_exit() {
	pthread_mutex_lock(&m);
	done = 1;
	pthread_cond_signal(&c);
	pthread_mutex_unlock(&m);
	 }
	
	 void thr_join() {
	 pthread_mutex_lock(&m);
	 while (done == 0)
	 pthread_cond_wait(&c, &m);
	 pthread_mutex_unlock(&m);
	 }

int main()
{
    pthread_t thread1, thread2;  /* thread variables */
    thdata data1, data2;         /* structs to be passed to threads */
    
    /* initialize data to pass to thread 1 */
    data1.thread_no = 1;
    strcpy(data1.message, "Hello!");

    /* initialize data to pass to thread 2 */
    data2.thread_no = 2;
    strcpy(data2.message, "Hi!");
    
    /* create threads 1 and 2 */    
    pthread_create (&thread1, NULL, &print_message_function, (void *) &data1);
    pthread_create (&thread2, NULL, &print_message_function, (void *) &data2);




    /* Main block now waits for both threads to terminate, before it exits
       If main block exits, both threads exit, even if the threads have not
       finished their work */ 
       thr_join();
       printf("Someting finished\n");



              
    /* exit */  
    exit(0);
} /* main() */

/**
 * print_message_function is used as the start routine for the threads used
 * it accepts a void pointer 
**/
void *print_message_function ( void *ptr )
{
    thdata *data;            
    data = (thdata *) ptr;  /* type cast to a pointer to thdata */
	int i;
    for (i = 0; i < 10000; ++i)
    {
    	printf("Thread %d says %s the %d time \n", data->thread_no, data->message, i);
    	// printf("test %d\n", i);
    };
    printf("Thread: %d Finished printing\n", data->thread_no);
thr_exit() ;   /* do the work */
    
    pthread_exit(0); /* exit */
} /* print_message_function ( void *ptr ) */

