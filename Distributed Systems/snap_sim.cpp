#include<cstdlib>
#include<cmath>
#include<iostream>
#include<fstream>

using namespace std;

#define CH_LENGTH 100

#define Usage "Usage: ./snap command_file \n command_file: the name of file controlling messages\n"

class Message{
	public:
	int snode; // sender node
	int sTime; // physical time of message
	char *type; // type general "G" or snap message "S"
	int rnode; // receiver node
	int delay; // time to reach end of other node
	int value; // information in message // not in use now
};

class Snap{
	public:
	int node; // node at which snapshot is taken
	int pTime; // physical time at which snapshot is taken
	int lTime; // logical time at which snapshot is taken
	int sender; // node from which latest message received before snapshot is taken
	int rTime; // time at which latest message was received
	int sTime; // time at which latest message was sent from sender
	int value;
};

class NodeState{
	public:
	char mode; // 'S' snapping mode, 'R' running mode;
	int value; // latest value
	int lTime; // logical time at node
	int rTime; // message receive time
	int sender; // last message sender
	int sTime; // last message sent time from sender
	char event; // event last happened
	int realTime; // real physical time of the node
};

// global variables
NodeState *nodeState;
Message ***channel;
Snap *snapState;
bool **snapped;
int *missingSnapNodes;
int nodes=0;
// place message from sender to receiver in the channel
void place_message_in_channel(int pTime,int sender, int receiver, int delay, char *type)
{
	int i;
	for(i=0;i<CH_LENGTH;i++)
	{
		// if channel's message queue has any empty space, then place message
		if(channel[sender][receiver][i].type=="NULL")
		{
			channel[sender][receiver][i].sTime=pTime;
			channel[sender][receiver][i].type=type;
			channel[sender][receiver][i].delay=delay;
			break;
		}
	}
}

// move message forward by one step in queue in channel
void move_up_message_in_channel(int sender, int receiver)
{
	int i;
	for(i=0;i<CH_LENGTH-1;i++)
	{
		channel[sender][receiver][i]=channel[sender][receiver][i+1];
	}
	channel[sender][receiver][CH_LENGTH-1].type="NULL";
}

// set a node into snapping mode
void go_snap(int node)
{
	int i;
	nodeState[node].mode='S'; // set node mode as snapping
	for(i=0;i<nodes;i++)
	{
		snapped[node][i]=0; // set snapshot token back to false for all other nodes
	}
	snapped[node][node]=1;
	missingSnapNodes[node]=nodes-1; // missing snapshots set to number of total nodes less one

	// record local state of that node
	snapState[node].node=node;
	snapState[node].pTime=nodeState[node].realTime;
	snapState[node].lTime=nodeState[node].lTime;
	snapState[node].sender=nodeState[node].sender;
	snapState[node].rTime=nodeState[node].rTime;
	snapState[node].sTime=nodeState[node].sTime;

	// broadcast snap request to all other nodes
	for(i=0;i<nodes;i++)
	{
		if(i!=node)
		{
			place_message_in_channel(snapState[node].pTime,node, i, 0, "S");
		}
	}
	
}
// displays snapshot statistics at a node 
void display_snapshot_stat(int node)
{
	cout<<endl<<"Snapshot at node #"<<node<<endl;
	cout<<"physical time: "<<snapState[node].pTime<<endl;
	cout<<"logical time: "<<snapState[node].lTime<<endl;
	cout<<"last message received from node: "<<snapState[node].sender<<endl;
	cout<<"last message sent time from sender: "<<snapState[node].sTime<<endl;
}

int main(int argc, char** argv)
{
	if (argc^2) {
		cout << Usage;
		exit(1);
	}

	int globalTime=0; // global real time	
	char *fname=argv[1];
	int pTime=0, n=0, p=0, t=0;
	char op;
	ifstream infile;

	//open file for reading
	infile.open(fname, ios::in);
	if (!infile) {
		cout << "READDATA: Can't read data file: " << fname << endl;
		exit(1);
	}
	// read first line from file
	infile>>nodes; // read no. of nodes
	nodeState=new NodeState[nodes]; // define no. of nodestates
	channel=new Message**[nodes]; // allocate memory for channels of message
	snapState=new Snap[nodes];
	snapped=new bool*[nodes];
	missingSnapNodes=new int[nodes];
	for(int i=0;i<nodes;i++)
	{
		channel[i]=new Message*[nodes];
		snapped[i]=new bool[nodes];
	}
	for(int i=0;i<nodes;i++)
	{
		for(int j=0;j<nodes;j++)
		{
			channel[i][j]=new Message[CH_LENGTH];
			for(int k=0;k<CH_LENGTH;k++)
			{channel[i][j][k].type="NULL";}
		}
		nodeState[i].mode='R';
		nodeState[i].lTime=0;
		nodeState[i].realTime=0;
		nodeState[i].sender=-1;
		nodeState[i].sTime=-1;
	}
	cout<<endl<<"no. of nodes = "<<nodes<<endl;

	bool msgLine=0, endOfActions=0;
	while(infile>>pTime>>op || msgLine==1)
	{		
		while(pTime>=globalTime || (endOfActions==1 && msgLine==1)) // physical time greater than real time then keep checking in message channel
		{	
			
			{globalTime++;}	

			// check in each node for queue of message
			msgLine=0;
			for(int i=0;i<nodes;i++)
			{	
				if(nodeState[i].realTime<globalTime)
					nodeState[i].realTime=globalTime; // sync with global real time if node's realtime is lagging
				for(int j=0;j<nodes;j++)
				{
					if(i!=j) // only for channels with non selfnode
					{
						if(channel[j][i][0].type!="NULL")
							msgLine=1;
						if(channel[j][i][0].type=="G") // for general message
						{
							if((channel[j][i][0].sTime+channel[j][i][0].delay)<=nodeState[i].realTime) // check if message send time plus delay is less than or equal to real time and real time for node j which may be increased due to local event
							{	//message received from node j to node i
								nodeState[i].lTime++;
								nodeState[i].rTime=nodeState[i].realTime;
								nodeState[i].event='R';
								nodeState[i].sender=j;
								nodeState[i].sTime=channel[j][i][0].sTime;
								if(nodeState[i].mode=='S' && !snapped[i][j]) // snapping mode
								{
									// record incoming messages too for snapshot of j node
									snapState[i].node=i;
									snapState[i].pTime=nodeState[i].realTime;
									snapState[i].lTime=nodeState[i].lTime;
									snapState[i].sender=j;
									snapState[i].sTime=channel[j][i][0].sTime;
									snapState[i].rTime=nodeState[i].realTime;
								}
								// move up front message in that message channel
								move_up_message_in_channel(j,i);		
							}
						}
						else if(channel[j][i][0].type=="S") // if message is snap message
						{
							nodeState[i].lTime++;
							nodeState[i].rTime=nodeState[i].realTime;
							nodeState[i].event='R';
							if(nodeState[i].mode=='R') // node is not snapping begin snapping mode
							{
								// begin snapping
								go_snap(i);
							}
							if(!snapped[i][j]) // if node i was recording channel j  
							{	// stop recording for channel j; snapshot token is now back to node i
								snapped[i][j]=true;
								missingSnapNodes[i]--;
								if(missingSnapNodes[i]==0) // snapshot token is back from all channels, recording for snapshot is completed
								{
									// then it is snapshot is taken/ completed at this time
									snapState[i].pTime=nodeState[i].realTime;
									snapState[i].lTime=nodeState[i].lTime;
									// display snapshot taken at this node
									display_snapshot_stat(i);
									// go back to normal running mode
									nodeState[i].mode='R';
								}
							}
							// move up front message in that message channel
							move_up_message_in_channel(j, i);
						}
						
						
					}
				}
			}
			
		}
		if(pTime<=globalTime)
		{ 
			if(infile>>p){
				switch (op)
				{
					case 'm': // message send event
						infile>>n>>t;
						nodeState[p].lTime++;
						nodeState[p].realTime++;
						nodeState[p].event='M';
						place_message_in_channel(pTime,p,n,t,"G");
						break;
					case 'l': // local event
						infile>>t;
						nodeState[p].lTime++;
						nodeState[p].event='L';
						nodeState[p].realTime+=t;//
						break;
					case 's': // snapshot event
						//infile>>p;
						nodeState[p].lTime++;
						nodeState[p].event='S';
						nodeState[p].realTime++;
						// begin snapping; record its local state and then send snapshot request to all channels
						go_snap(p);
					
				}
			}
			else
			{
				endOfActions=1;
			}		
		}

	}

	return 0;

}
