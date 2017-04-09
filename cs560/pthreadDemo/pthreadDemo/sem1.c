#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>

void * thread_func (void *arg);
sem_t bin_sem;

#define RUNNING_APP 10

int main()
{
  int res, i, avail_page;
  pthread_t threads[RUNNING_APP];
  int appid[RUNNING_APP];

  void *thread_result;
  
  // Available pages is only half the running app
  avail_page = RUNNING_APP / 2;

  res=sem_init(&bin_sem,0, avail_page);
  if (res != 0)
  {
    perror("Semaphore init fail!");
    exit(EXIT_FAILURE);
  }

  for (i = 0; i < RUNNING_APP; i++)
  {
	appid[i] = 10 + i;
	res=pthread_create(threads + i,NULL,thread_func,(void*) (appid+i));
	if (res != 0)
	{
     perror("Thread create fail!");
	 exit(EXIT_FAILURE);
	}
  }

  printf("Main: Wait for thread to end...\n");

  // join threads

  for (i = 0; i < RUNNING_APP; i++)
  {
      pthread_join( *(threads + i), NULL);
  }

  printf("Main End");

  sem_destroy(&bin_sem);
  exit(0);
}

void * thread_func (void *arg)
{
  int i, n, pages;
  int* name;
  name = arg;
  n = *name - 5;

  for (i = 0; i < n; i++)
  {
	printf("Thread (%d) waiting for memory page...\n", *name);
	sem_wait(&bin_sem); 
    sem_getvalue(&bin_sem, &pages);
	printf("Thread (%d) get a memory page and start running (%d/%d)...Avail pages = %d\n", *name, i+1, n, pages);
	sleep(i + n);
	printf("Thread (%d) finished running (%d/%d)!\n", *name, i + 1, n);
	sem_post(&bin_sem);
	sleep(1);
  }

  pthread_exit(NULL);
}


