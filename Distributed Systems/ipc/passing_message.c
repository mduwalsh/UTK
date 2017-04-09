/*
client:
*/
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#define KEY 1200    // base for message queues

struct 
{
  long   type;      // message type ( > 0 )
  int    id;        // id
  int    temp;      // temperature
  int    stable;    // stability
} Send, Receive;

size_t Length = sizeof(Send) - sizeof(long);

int main(int argc, char *argv[]) 
{
  int qidL,qidS,status, id = atoi(argv[2]);         // client id

  if(argc != 3) {
    printf("USAGE: ./pm Temp ID");
    return 1;
  }
  if ((qidS = msgget(KEY, 0600|IPC_CREAT)) < 0){    // server message queue
    status = errno;
    perror("client: msgget failed");
    printf("qidS = %d, errno = %d\n", qidS, status);
    return 1;
  }
  if ((qidL = msgget(KEY+id, 0600|IPC_CREAT)) < 0){ // local message queue
    status = errno;
    perror("client: msgget failed");
    printf("qidL = %d, errno = %d\n", qidS, status);
    return 1;
  }
  Send.type   = id;
  Send.id     = id;
  Send.temp   = atoi(argv[1]);

  while (1){
    if (msgsnd(qidS, &Send, Length, 0) < 0){
      status = errno;
      perror("client: msgsnd failed");
      printf("errno = %d\n", status);
      return 1;
    }
    if (msgrcv(qidL, &Receive, Length, 1, 0) < 0){
      status = errno;
      perror("client: msgrcv failed");
      printf("errno = %d\n", status);
      return 1;
    }
    if (Receive.stable)
      break;
    else
      Send.temp = (3*Send.temp + 2*Receive.temp)/5; // update temperature
  }
  printf("\nclient %d Temp: %u\n", Send.id, Send.temp);
  return 0;
}

/*
gcc -ggdb -g3 -Wall -o pm passing_message.c

./pm 100 1 &  ./pm  22 2 &  ./pm  50 3 &  ./pm  40 4 & ./mp  60 

kill %1 %2 %3 %4 %5
ipcrm -Q 1200
ipcrm -Q 1201
ipcrm -Q 1202
ipcrm -Q 1203
ipcrm -Q 1204
*/
