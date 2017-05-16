/*
	process code
*/

#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>

#define KEY      1000 // base for message queues

int *Qid;   // message queue IDs
int *QidC;
int NODES;  // no. of nodes
typedef struct msg
{
	long mtype;        // message type ( > 0 )
	char task;        // local event, send msg or initiate snap shot (l,m,s)	
	int sender;          // process id 
	int ptime;  	// physical time of event
	int sltime;	// logical time of sender
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
  int status=  msgrcv(msqid, msgp, msgsz, msgtyp, msgflg);
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

void display_snapshot_stat(int pid, int ptime, int ltime, int snap_sender, int snap_stime)
{
	printf("Snapshot at node #%d\n",pid);
	printf("Physical time: %d\n",ptime);
	printf("Logical time: %d\n",ltime);
	printf("last message received from node: %d\n",snap_sender);
	printf("last message sent time from sender: %d\n",snap_stime);
}

void go_snap(int node, int ltime, bool *snapped, int *missingSnapNodes)
{
	int i;
	Message sbuff;
	for(i=0;i<NODES;i++)
	{
		snapped[i]=false; // set snapshot token back to false for all other nodes
	}
	snapped[node]=1;
	*missingSnapNodes=NODES-1; // missing snapshots set to number of total nodes less one	

	sbuff.mtype=1;
	sbuff.sender=node;
	sbuff.task='S';
	sbuff.sltime=ltime;
	// broadcast snap request to all other nodes
	for(i=0;i<NODES;i++)
	{
		if(i!=node)
		{			
			sbuff.receiver=i;			
			msgSnd(Qid[i], &sbuff, Length, 0);
		}
	}
	
}

int main(int argc, char *argv[]) 
{	
	if (argc ^ 3) {
	printf("USAGE: ./snap_imp.out filename process_id");
	exit(0);
	}
	
	int pid=atoi(argv[2]); // process id
	char* fname=argv[1];
	char op;
	FILE *fp;
	int pTime,p,n,t;
	int i, endoffile;
	int status;
	bool complete=0;
	Message s_buff,r_buff;
	s_buff.mtype = 1;
	r_buff.mtype = 1;
	
	char mode='R';
	int lst_msg_sender=-1;
	int lst_msg_stime=-1;
	int snap_sender=-1; // last msg sender before snapshot
	int snap_stime=-1; // last msg send time of sender before snapshot
	int rTime=0,lTime=0;
	bool *snapped;
	int missingSnapNodes;
	

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
	snapped=malloc(NODES*sizeof(bool));
	for (i = 0; i<NODES; i++){
		if (msgGet(Qid+i,KEY+i, 0600|IPC_CREAT))           // get queue IDs
			return 1;                                        //   (error)
	}
	if (msgGet(QidC,KEY-1, 0600|IPC_CREAT))           // get queue IDs
			return 1;  
	// for synchronizing processes for 1st time
	msgRcv(Qid[pid], &r_buff, Length, 1, 0);

	i=0;
	endoffile=0;
	while(!complete)
	{
		do{
			if(fscanf(fp,"%d", &pTime)!=EOF)
			{
				fscanf(fp,"%c", &op); // fetch operation
				if(op==' ')
					fscanf(fp,"%c", &op);
				fscanf(fp,"%d", &p); // fetch node of operation root
				if(p==pid) // only if node read is the process node
				{
					while(pTime>rTime) // if time of op is greater than time of node read messages and hold ops
					{
						status=msgRcv(Qid[pid], &r_buff, Length, 1, IPC_NOWAIT);
						if(status!=-1)
						{
							// update logical time
							if(lTime>r_buff.sltime) 
							{
								lTime++;		
							}	
							else
							{
								lTime=r_buff.sltime+1;
							}									
							if(r_buff.task=='G') // if general message
							{
								lst_msg_sender=r_buff.sender;
								lst_msg_stime=r_buff.ptime;
								if(mode=='S' && !snapped[r_buff.sender]) // snapping
								{
									// record it in snapshot
									snap_sender=r_buff.sender;
									snap_stime=r_buff.ptime;
								}
							}
							else if(r_buff.task=='S') // if snapshot token message
							{
								if(mode!='S')
								{
									mode='S'; // set node mode as snapping	
									// record local state of that node
									snap_sender=lst_msg_sender;
									snap_stime=lst_msg_stime;
									//begin snapping
									go_snap(pid,lTime, snapped, &missingSnapNodes);	
									
								}
								if(!snapped[r_buff.sender])
								{
									snapped[r_buff.sender]=true;
									missingSnapNodes--;
									if(missingSnapNodes==0)
									{
										// snapshot complete
										display_snapshot_stat(pid,rTime,lTime,snap_sender,snap_stime);
										mode='R';
										complete=1;
																
									}
								}
							}
						}
						rTime++;						
					}
					if(pTime<=rTime)
					{	
						lTime++;				
						switch (op)
						{
							case 'm': // message send event
								fscanf(fp,"%d %d",&n,&t);
								s_buff.sender=p;
								s_buff.task='G';
								s_buff.receiver=n;
								s_buff.sltime=lTime;
								s_buff.ptime=pTime;
								s_buff.delay=t; // will not be used since delay is consider as channel delay in implementation
								msgSnd(Qid[n], &s_buff, Length, 0);
								rTime++;				
								break;
							case 'l': // local event
								fscanf(fp,"%d",&t);
								// change node state 
								rTime+=t;
								break;
							case 's': // snapshot event
								mode='S';
								// record local state of that node
								snap_sender=lst_msg_sender;
								snap_stime=lst_msg_stime;
								 // go snap								
								go_snap(pid,lTime, snapped, &missingSnapNodes);
								rTime++;
			
						}												
					}
				
				}
			}
			else
			{
				endoffile=1;
			}
			
		}while(p!=pid && endoffile==0);

		// check for messages
		status=msgRcv(Qid[pid], &r_buff, Length, 1, IPC_NOWAIT);
		if(status!=-1)
		{
			if(lTime>r_buff.sltime)
			{
				lTime++;		
			}	
			else
			{
				lTime=r_buff.sltime+1;
			}					
			if(r_buff.task=='G')
			{
				lst_msg_sender=r_buff.sender;
				lst_msg_stime=r_buff.ptime;
				if(mode=='S' && !snapped[r_buff.sender]) // snapping
				{
					// record it in snapshot
					snap_sender=r_buff.sender;
					snap_stime=r_buff.ptime;
				}
			}
			else if(r_buff.task=='S')
			{
				if(mode!='S')
				{
					mode='S';
					// record local state of that node
					snap_sender=lst_msg_sender;
					snap_stime=lst_msg_stime;
					//begin snapping
					go_snap(pid,lTime, snapped, &missingSnapNodes);	
						
				}
				if(!snapped[r_buff.sender])
				{
					snapped[r_buff.sender]=true;
					missingSnapNodes--;
					if(missingSnapNodes==0)
					{
						// snapshot complete
						display_snapshot_stat(pid,rTime,lTime,snap_sender,snap_stime);
						mode='R';
						complete=1;
					}
				}
			}
		}
		rTime++;
		
	}
      
	fclose(fp);
	free(Qid);
	free(snapped);
	msgctl(Qid[pid], IPC_RMID, 0);
	return 0;
}



