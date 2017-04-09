/*
	central process node which controls trigger for operation of process nodes by sending msgs to all nodes 
	only for synchronization to start all process at nearly same time
	it is invoked at last after invoking all other process nodes
*/

#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#define KEY      1000 // base for message queues
int *Qid;
int *QidC;
int NODES;
typedef struct msg
{
	long mtype;        // message type ( > 0 )
	char task;        // local event, send msg or initiate snap shot (l,m,s)	
	int sender;          // process id 
	int ptime;  	// physical time of event
	int delay;  	// time taken for local event
	int receiver;  // if event is send msg, then target node is receiver

} Message;

size_t Length = sizeof(Message) - sizeof(long);

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
  int status=  msgrcv(msqid, msgp, msgsz, msgtyp, msgflg) ;
  return status;
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
	if (argc ^ 2) {
	printf("USAGE: ./central.out filename");
	exit(0);
	}
	int i;
	char* fname=argv[1];
	char op;
	FILE *fp;
	Message sbuff;
	sbuff.mtype = 1;

	//open file for reading
	fp=fopen(fname, "r");
	if (!fp) {
		printf("READDATA: Can't read data file: %s \n", fname);
		exit(1);
	}

	// read first line from file
	fscanf(fp,"%d",&NODES); // read no. of nodes
	Qid=malloc(NODES*sizeof(int)); // stores message queue ids for all nodes
	QidC=malloc(sizeof(int));
	for (i = 0; i<NODES; i++){
		if (msgGet(Qid+i,KEY+i, 0600|IPC_CREAT))           // get queue IDs
			return 1;                                        //   (error)
	}
	if (msgGet(QidC,KEY-1, 0600|IPC_CREAT))           // get queue IDs
			return 1;  
	// for synchronizing process for 1st time
	for(i=0;i<NODES;i++)
	{
		msgSnd(Qid[i], &sbuff, Length, IPC_NOWAIT);
	}
	
	fclose(fp);
	free(Qid);
	free(QidC);
	msgctl(QidC[0], IPC_RMID, 0);
  	return 0;
}

