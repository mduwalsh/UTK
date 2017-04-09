/*
Four external process communicate with a server process to synchronize
temperature (external processes are only aware of the server, the
server is aware of all).

server:
*/

#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#define KEY      1200 // base for message queues
#define CLIENTS  4    // # clients (external process)

int Qid[1+CLIENTS];   // message queue IDs

struct 
{
  long   type;        // message type ( > 0 )
  int    id;          // id
  int    temp;        // temperature
  int    stable;      // stability
} Send, Receive;

size_t Length = sizeof(Send) - sizeof(long);

int msgSnd(int msqid, const void *msgp, size_t msgsz, int msgflg)
{
  int status;

  if (msgsnd(msqid, msgp, msgsz, msgflg) < 0){
    status = errno;
    perror("server: msgsnd failed");
    printf("errno = %d\n", status);
    return 1;
  }
  return 0;
}

int msgRcv(int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg)
{
  int status;

  if (msgrcv(msqid, msgp, msgsz, msgtyp, msgflg) < 0){
    status = errno;
    perror("server: msgrcv failed");
    printf("errno = %d\n", status);
    return 1;
  }
  return 0;
}

int msgGet(int *qid, key_t key, int msgflg)
{
  int status;

  if ((*qid = msgget(key, msgflg)) < 0){
    status = errno;
    perror("server: msgget failed");
    printf("qid = %d, errno = %d\n", *qid, status);
    return 1;
  }
  return 0;
}

int main(int argc, char *argv[]) 
{
  int i,sum,stable, temp[1+CLIENTS];                   // temperatures

  if (argc != 2) {
    printf("USAGE: ./central.out Temp");
    exit(0);
  }
  for (i = CLIENTS; i > -1; i--){
    if (msgGet(Qid+i,KEY+i, 0600|IPC_CREAT))           // get queue IDs
      return 1;                                        //   (error)
    temp[i] = -1;                                      // initialize temperatures
  }
  Send.type   = 1;
  Send.temp   = atoi(argv[1]);                         // starting temperature
  Send.stable = 0;

  for (stable = 0; !stable; ){
    for (sum = 0, stable = 1, i = 0; i++ < CLIENTS;){  // get client temperatures
      if (msgRcv(Qid[0], &Receive, Length, i, 0))      //   receive type i
	return 1;                                      //     (error)
      if (temp[Receive.id]^Receive.temp){              //   temperature change
	temp[Receive.id] = Receive.temp;               //     save temperature
	stable = 0;                                    //     not stable
      }
      sum += Receive.temp;                             //   sum temperatures
    }
    if (stable){
      printf("\nTemperature Stable: %u\n", Send.temp);
      Send.stable = 1;                                 // notify clients
    } else {
      Send.temp = (2*Send.temp + sum)/6;               // update temperature
    }
    for (i = 0; i++ < CLIENTS;){
      if (msgSnd(Qid[i], &Send, Length, IPC_NOWAIT))   // send (to Qid[i])
	return 1;                                      //   (error)
    }
  }
  for (i = 0; i < 1+CLIENTS; i++)                      // remove queues
    if (msgctl(Qid[i], IPC_RMID, 0)){
      perror("server: msgsnd failed");
      return 1;
    }
  return 0;
}
/*
gcc -ggdb -g3 -Wall -o mp message_passing.c 

./pm 100 1 &  ./pm  22 2 &  ./pm  50 3 &  ./pm  40 4 & ./mp  60 

kill %1 %2 %3 %4 %5
ipcrm -Q 1200
ipcrm -Q 1201
ipcrm -Q 1202
ipcrm -Q 1203
ipcrm -Q 1204
*/
