/*                multi_thr.c                */

/*
  Simple program that just illustrates thread creation, thread exiting,
  waiting for threads, and returning status from threads.
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define MULTI 2

/* Function prototypes for thread routines */
void *sub_a(void *);
void *sub_b(void *);
void *sub_c(void *);
void *sub_d(void *);
void *sub_e(void *);

pthread_t               tid_a, tid_b, tid_c, tid_d, tid_e, tid_main;
pthread_attr_t          attr;
int                     zero;
float                   f;

unsigned long thread_name(pthread_t tid)
{
	unsigned long name;
	name = ((unsigned long) tid)%100;
	return name;
}

void SLEEP(int t)
{
	// prolong sleep time by MULTI times
	sleep(t * MULTI);
}

int time1()
{return(time(NULL)-zero);}


void *sub_a(void *arg)
{int            err, i;
 pthread_t      tid = pthread_self();

 printf("[%2d] A: \t In thread A [%lu]\n", time1(), thread_name(tid));
 SLEEP(1);
 pthread_create(&tid_d, &attr, sub_d, NULL);
 printf("[%2d] A: \t Created thread D [%lu]\n", time1(), thread_name(tid_d));

 SLEEP(3);
 printf("[%2d] A: \t Thread exiting...\n", time1());
 return((void *)77);            /* Same as pthread_exit((void *)77) */
}


void *sub_b(void *arg)
{pthread_t      tid = pthread_self();
 printf("[%2d] B: \t In thread B [%lu]\n", time1(), thread_name(tid));

 SLEEP(4);

 printf("[%2d] B: \t Thread exiting...\n", time1());
 pthread_exit(NULL);
}


void *sub_c(void *arg)
{void           *status;
 int            err, i;

 pthread_t tid = pthread_self();
 pthread_t*      ptid;
 ptid = (pthread_t *) arg; /* Passing a POINTER to the TID */
 unsigned long           name = thread_name(tid);

 printf("[%2d] C: \t In thread C [%lu]\n", time1(), name);
 SLEEP(2);

 printf("[%2d] C: \t Joining main thread...\n", time1());
 if (err=pthread_join(*ptid, &status)) 
 {
   //printf("pthread_join Error. %s", strerror(err));
   perror("pthread_join Error.");
   exit(1);
   }  

 printf("[%2d] C: \t Main thread [%lu] returning status: %ld\n",
        time1(), thread_name(*ptid), ((long int) status));

 SLEEP(1);

 pthread_create(&tid_b, &attr, sub_b, NULL);
 printf("[%2d] C: \t Created thread B [%lu]\n", time1(), thread_name(tid_b));

 SLEEP(4);
 printf("[%2d] C: \t Thread exiting...\n", time1());
 f = 88.8;
 pthread_exit((void *) &f);
}

void *cleanup(void *arg)
{pthread_t tid = pthread_self();
 unsigned long name = thread_name(tid);

  printf("[%2d] D: \t %lu cancelled! \n", time1(), name);
}



void * sub_d(void *arg)
{int            err, i;
 pthread_t      tid_e;
 void           *status;
 pthread_t      tid = pthread_self();

 printf("[%2d] D: \t In thread D [%lu]\n", time1(), thread_name(tid));

 pthread_cleanup_push((void*) cleanup, NULL);
 pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
 pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

 SLEEP(1);
 pthread_create(&tid_e, &attr, sub_e, NULL);
 printf("[%2d] D: \t Created thread E [%lu]\n", time1(), thread_name(tid_e));

 SLEEP(5);

        /* D should get cancelled before this runs. */
 printf("[%2d] D: \t Thread exiting...\n", time1());
 pthread_cleanup_pop(0);
 return((void *)55);
}


void * sub_e(void *arg)
{int            err, i;
 void           *status;
 pthread_t      tid = pthread_self();

 printf("[%2d] E: \t In thread E [%lu]\n", time1(), thread_name(tid)); 

 SLEEP(3);
 printf("[%2d] E: \t Joining thread A...\n", time1());
 if (err=pthread_join(tid_a, &status)) 
 {
	 //printf("pthread_join Error. %s", strerror(err)), exit(1);
   perror("pthread_join Error.");
   exit(1);
 }
 printf("[%2d] E: \t Thread A [%lu] returning status: %ld\n",
        time1(), thread_name(tid_a), ((long int) status));

 SLEEP(2);

 printf("[%2d] E: \t Joining thread C...\n", time1());
 if (err=pthread_join(tid_c, &status)) 
 { //printf("pthread_join Error. %s", strerror(err)), exit(1);
   perror("pthread_join Error.");
   exit(1);
 }
 printf("[%2d] E: \t Thread C [%lu] returning status: %f\n",
        time1(), thread_name(tid_c), *((float *) status));

 SLEEP(2);
 printf("[%2d] E: \t Joining thread D...\n", time1());
 if (err=pthread_join(tid_d, &status)) 
 {
   //printf("pthread_join Error. %s", strerror(err)), exit(1);
   perror("pthread_join Error.");
   exit(1);
 }
 if ((void*) status == (void*) PTHREAD_CANCELED)
   printf("[%2d] E: \t Thread D [%lu] returning status: PTHREAD_CANCELED\n",
          time1(), thread_name(tid_d));
 else
   printf("[%2d] E: \t Thread D [%lu] returning status: %ld\n",
          time1(), thread_name(tid_d), ((long int) status));

 SLEEP(1);
 printf("[%2d] E: \t Thread exiting...\n", time1());
 pthread_exit((void *)44);
}



main()
{int            err;

 zero = time(NULL);
 tid_main = pthread_self();
 printf("Time Thread \t Event\n");
 printf("==== ====== \t =====\n");
 printf("[%2d] Main: \t Started [%lu]\n", time1(), thread_name(tid_main));

 pthread_attr_init(&attr);
 pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
 pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE); /* Also default */

 SLEEP(1);
 pthread_create(&tid_a, &attr, sub_a, NULL);
 printf("[%2d] Main: \t Created thread A [%lu]\n", time1(), thread_name(tid_a));

 SLEEP(1);
 pthread_create(&tid_c, &attr, sub_c, (void *) &tid_main);
 printf("[%2d] Main: \t Created thread C [%lu]\n", time1(), thread_name(tid_c));

 SLEEP(2);
 printf("[%2d] Main: \t Cancelling thread D [%lu]\n",
        time1(), thread_name(tid_d));
 pthread_cancel(tid_d);

 SLEEP(1);
 printf("[%2d] Main: \t Thread exiting...\n", time1());
 pthread_exit((void *) NULL);
}





