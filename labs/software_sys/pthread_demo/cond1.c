#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

pthread_mutex_t count_mutex     = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  condition_var   = PTHREAD_COND_INITIALIZER;

void *functionCount1();
int  count = 0;
#define COUNT_DONE  10
#define COUNT_HALT1  3
#define COUNT_HALT2  6
#define STUDENT_CNT 10

main()
{
   pthread_t thread1, thread2;
   pthread_t threads[STUDENT_CNT];
   int stu_id[STUDENT_CNT];

   int i;
   
   for (i = 0; i <STUDENT_CNT; i++)
	{
		stu_id[i] = 10 + i;
		pthread_create( threads + i, NULL, &functionCount1, (void*) (stu_id + i));
	}

   sleep(3);
   printf("Signal:     ---------> One Lunch is ready!\n");
   pthread_cond_signal( &condition_var );
//   pthread_cond_signal( &condition_var );
   sleep(5);
   printf("Broadcast:  ---------> All lunch are ready!\n");
   pthread_cond_broadcast( &condition_var );
   sleep(2);

	
	for (i = 0; i <STUDENT_CNT; i++)
	{
		pthread_join( *(threads + i), NULL);
	}

   exit(0);
}

void *functionCount1(void* id)
{
	  int* num;
	  num = id;  

	  printf("Student %d is waiting for lunch\n", *num);
      pthread_cond_wait( &condition_var, &count_mutex );
      
      printf("O(._.)O~~  Student %d is ready to eat lunch\n", *num);

      pthread_mutex_unlock( &count_mutex );
}


