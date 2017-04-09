#include<cstdlib>
#include<cmath>
#include<iostream>
#include<fstream>

using namespace std;

int convertHexToDecimal(char *);
int getCacheSetNumber(int , int );
int getWordAddress(int , int );
int getBlockAddress( int , int );
int getcacheTag(int , int );
void setLRUUnitHighest(int **,int,int,int);

int main()
{
	char *fname;
	int wordSize; // common word size in bytes for all levels of cache
	int blockSize1, cacheLines1, assoc1,blockSize2, cacheLines2, assoc2,blockSize3, cacheLines3, assoc3; // individual parameters of cache
	int noOfCache, writeMethod, memReadTime, memWriteTime; // setting of cache levels and write method (write back-1, write through-2); // write allocate is assumed for both
	
	// declaration of two dimensional pointer arrays to store tags, lru values and dirty bit status for all 3 levels of cache
	int **cache1,**cache2,**cache3,**LRU1,**LRU2,**LRU3,**dirty1,**dirty2,**dirty3; 
	
	int byteAdd, wordAdd, blockAdd; // variables to store diff. version of address corresponding 	
	int setNo; // no. of line in cache address belong to 
	int tag; // store tag part of an address read for different level of cache
	
	// calculation variables declaration
	int totalAccess=0, totalRead=0, totalWrite=0, cache1Access=0, cache2Access=0, cache3Access=0, cache1Read=0, cache2Read=0, cache3Read=0, cache1Write=0, cache2Write=0, cache3Write=0,
	cache1ReadHit=0, cache2ReadHit=0, cache3ReadHit=0, cache1ReadMiss=0, cache2ReadMiss=0, cache3ReadMiss=0,
	cache1WriteHit=0, cache2WriteHit=0, cache3WriteHit=0, cache1WriteMiss=0, cache2WriteMiss=0, cache3WriteMiss=0;
	int cache1ReadDirty=0,cache2ReadDirty=0,cache3ReadDirty=0,cache1WriteDirty=0,cache2WriteDirty=0,cache3WriteDirty=0;	
	
	int i, j, k; // variables to be used during loops and other ops
	
	// configuration from command line
	noOfCache=3;
	writeMethod=1;
	wordSize=4;
	// read settings for cache 1
	blockSize1=4;
	cacheLines1=10;
	assoc1=4;
	// ready structure for cache 1 for no. of cache lines
	cache1= new int*[cacheLines1];
	LRU1=new int*[cacheLines1];
	dirty1=new int*[cacheLines1];
	for(i=0;i<cacheLines1;i++)
	{
		cache1[i]=new int[assoc1];
		LRU1[i]=new int[assoc1];
		dirty1[i]=new int[assoc1];
	}

	// intialize cache, lru and dirty bits for cache 1 so no garbage values will be present
	for(i=0;i<cacheLines1;i++)
	{
		for(j=0;j<assoc1;j++)
		{
			cache1[i][j]=-1;
			LRU1[i][j]=j;
			dirty1[i][j]=0;
		}
	}
	
	if(noOfCache>1)
	{
		blockSize2=5; cacheLines2=20;
		assoc2=4;	
		// ready structure for cache 2 for no. of cache lines
		cache2= new int*[cacheLines2];
		LRU2= new int*[cacheLines2];
		dirty2=new int*[cacheLines2];
		for(i=0;i<cacheLines2;i++)
		{
			cache2[i]=new int[assoc2];
			LRU2[i]=new int[assoc2];
			dirty2[i]=new int[assoc2];
		}
		// intialize cache, lru and dirty bits for cache 2 so no garbage values will be present
		for(i=0;i<cacheLines2;i++)
		{
			for(j=0;j<assoc2;j++)
			{
				cache2[i][j]=-1;
				LRU2[i][j]=j;
				dirty2[i][j]=0;
			}
		}
	}

	//if(noOfCache>2)
	{
		blockSize3=6; cacheLines3=40; assoc3=4;
		// ready structure for cache 3 for no. of cache lines
		cache3= new int*[cacheLines3];
		LRU3=new int*[cacheLines3];
		dirty3=new int*[cacheLines3];
		for(i=0;i<cacheLines3;i++)
		{
			cache3[i]=new int[assoc3];
			LRU3[i]=new int[assoc3];
			dirty3[i]=new int[assoc3];
		}
		// intialize cache, lru and dirty bits for cache 3 so no garbage values will be present
		for(i=0;i<cacheLines3;i++)
		{
			for(j=0;j<assoc3;j++)
			{
				cache3[i][j]=-1;
				LRU3[i][j]=j;
				dirty3[i][j]=0;
			}
		}
	}

	fname="trace1.txt";

	ifstream infile;                   // file pointer	
	char add[1024];            // temp array to store line 
	char op;                // temp array to store one number
	
	//open file for reading
	infile.open(fname, ios::in);
	if (!infile) {
		cout << "READDATA: Can't read data file: " << fname << endl;
		exit(1);
	}

	// loop through each line in file 
int hit, pos;

	while (infile >> add >> op)
	{

		// increment total memory access by 1
		totalAccess++;
		// calculate byte and word addresses
		byteAdd=convertHexToDecimal(add);
		wordAdd=getWordAddress(byteAdd,wordSize);

		hit=0; pos=-1;
		// increment 1st level cache
		cache1Access++;	

		if(op=='R')
		{
			totalRead++;
			cache1Read++;		
			
			// search first level cache
			blockAdd=getBlockAddress(wordAdd, blockSize1);
			setNo=getCacheSetNumber(blockAdd,cacheLines1);
			tag=getcacheTag(blockAdd,cacheLines1);
			
			// check if tag exists within a set (loop through no. of associativity)
			hit=0;pos=0;
			for(i=0;i<assoc1;i++)
			{
				if(cache1[setNo][i]==tag)
				{
					hit=1;
					pos=i;
					break;
				}
			}
			if(hit==1)
			{
				cache1ReadHit++;				
				setLRUUnitHighest(LRU1,setNo,assoc1,pos); // set highest value in LRU					 
			}

			else
			{
				cache1ReadMiss++;
				pos=LRU1[setNo][0]; // least recently used block position
				if(writeMethod==1) // '1' corresponds to write back
				{
					// check if the block to be replaced is dirty or not
					if(dirty1[setNo][pos]==1)
					{
						cache1ReadDirty++;
						dirty1[setNo][pos]=0;
					}
				}	
	
				cache1[setNo][pos]=tag; // replace tag of the least recently used block

				setLRUUnitHighest(LRU1,setNo,assoc1,pos); // set highest value in LRU
					
				if(noOfCache>1)
				{
					// check in 2nd level cache
					cache2Access++;
					cache2Read++;
					blockAdd=getBlockAddress(wordAdd, blockSize2);
					setNo=getCacheSetNumber(blockAdd,cacheLines2);
					tag=getcacheTag(blockAdd,cacheLines2);
					// check if tag exists within a set (loop through no. of associativity)
					hit=0;pos=0;
					for(i=0;i<assoc2;i++)
					{
						if(cache2[setNo][i]==tag)
						{
							hit=1;
							pos=i;
							break;
						}
					}
					
					if(hit==1)
					{
						cache2ReadHit++;				
						setLRUUnitHighest(LRU2,setNo,assoc2,pos); // set highest value in LRU					 
					}
					else
					{
						cache2ReadMiss++;
						pos=LRU2[setNo][0]; // least recently used block position
						if(writeMethod==1) // '1' corresponds to write back
						{
							// check if the block to be replaced is dirty or not
							if(dirty2[setNo][pos]==1)
							{
								cache2ReadDirty++;
								dirty2[setNo][pos]=0;
							}
						}	
						cache2[setNo][pos]=tag; // replace tag of the least recently used block
						setLRUUnitHighest(LRU2,setNo,assoc2,pos); // set highest value in LRU
						
						if(noOfCache>2)
						{
							// check in 3rd level cache
							cache3Access++;
							cache3Read++;
							blockAdd=getBlockAddress(wordAdd, blockSize3);
							setNo=getCacheSetNumber(blockAdd,cacheLines3);
							tag=getcacheTag(blockAdd,cacheLines3);
							// check if tag exists within a set (loop through no. of associativity)
							hit=0;pos=0;
							for(i=0;i<assoc3;i++)
							{
								if(cache3[setNo][i]==tag)
								{
									hit=1;
									pos=i;
									break;
								}
							}
							if(hit==1)
							{
								cache3ReadHit++;				
								setLRUUnitHighest(LRU3,setNo,assoc3,pos); // set highest value in LRU					 
							}
							else
							{
								cache3ReadMiss++;
								pos=LRU3[setNo][0]; // least recently used block position
								if(writeMethod==1) // '1' corresponds to write back
								{
									// check if the block to be replaced is dirty or not
									if(dirty3[setNo][pos]==1)
									{
										cache3ReadDirty++;
										dirty3[setNo][pos]=0;
									}
								}	
								cache3[setNo][pos]=tag; // replace tag of the least recently used block
								setLRUUnitHighest(LRU3,setNo,assoc3,pos); // set highest value in LRU
							}
						}
					}
				}
								
			}
		}	
		else
		{
			
					
		}
	}
	
	cout<<"total access: "<<totalAccess<<endl;
	cout<<"total read access: "<<totalRead<<endl;
	cout<<"total write access: "<<totalWrite<<endl;
	cout<<"total cache1 access: "<<cache1Access<<endl;
	cout<<"cache1 read access: "<<cache1Read<<endl;
	cout<<"cache1 write access: "<<cache1Write<<endl;
	cout<<"cache1 read hit: "<<cache1ReadHit<<endl;
	cout<<"cache1 read miss: "<<cache1ReadMiss<<endl;
	cout<<"cache1 write hit: "<<cache1WriteHit<<endl;
	cout<<"cache1 write miss: "<<cache1WriteMiss<<endl;
	if(noOfCache>1)
	{
		cout<<"total cache2 access: "<<cache2Access<<endl;
		cout<<"cache2 read access: "<<cache2Read<<endl;
		cout<<"cache2 write access: "<<cache2Write<<endl;
		cout<<"cache2 read hit: "<<cache2ReadHit<<endl;
		cout<<"cache2 read miss: "<<cache2ReadMiss<<endl;
		cout<<"cache2 write hit: "<<cache2WriteHit<<endl;
		cout<<"cache2 write miss: "<<cache2WriteMiss<<endl;
	
	}
	if(noOfCache>2)
	{
		cout<<"total cache3 access: "<<cache3Access<<endl;
		cout<<"cache3 read access: "<<cache3Read<<endl;
		cout<<"cache3 write access: "<<cache3Write<<endl;
		cout<<"cache3 read hit: "<<cache3ReadHit<<endl;
		cout<<"cache3 read miss: "<<cache3ReadMiss<<endl;
		cout<<"cache3 write hit: "<<cache3WriteHit<<endl;
		cout<<"cache3 write miss: "<<cache3WriteMiss<<endl;
	}
  return 0;
}

int convertHexToDecimal(char *hexString)
{
	return (int)strtol(hexString,0,16);
}

// returns cache line/set number for block address and number of lines
int getCacheSetNumber(int blockAdd, int cacheLines)
{
	return (blockAdd % cacheLines);
}
//returns word address for byte address and word size
int getWordAddress(int byteAdd, int wordSize)
{
	return byteAdd/wordSize;
}
// returns block address for word address and block size
int getBlockAddress(int wordAdd, int blockSize)
{
	return wordAdd/blockSize;
}

// returns cache tag
int getcacheTag(int blockAdd, int cacheLines)
{
	return blockAdd/cacheLines;
}

// changes value in LRU algorithm; set highest value for given block position in a set (0 means least priority value)
void setLRUUnitHighest(int **ls,int set,int assoc,int pos)
{
    int i,value;
	int ** lSet;
	lSet=ls;
    for(i=0;i<assoc;i++)
        if(lSet[set][i] == pos)
		{
            value = i;
			break;
		}
    for(i=value;i<assoc-1;i++)
        lSet[set][i] = lSet[set][i+1];
    lSet[set][assoc-1] = pos;
}
