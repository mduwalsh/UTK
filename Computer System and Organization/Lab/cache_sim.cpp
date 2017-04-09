#include<cstdlib>
#include<cmath>
#include<iostream>
#include<fstream>

using namespace std;

// functions declarations
long convertHexToDecimal(char *); // converts hex string to decimal value
int getCacheSetNumber(long , int ); // returns set no. / line no. the address block belongs to for given cache lines
long getWordAddress(long , int ); // returns word address for given byte address and word size
long getBlockAddress( long , int ); // returns block address for given word address and block size
int getcacheTag(long , int ); // returns cache tag for given block address and no. of cache lines
void setLRUUnitHighest(int **,int,int,int); // sets highest priority for the recently used block and rearrange priority order


#define Usage "Usage: ./cache_sim trace_file no_of_cache_levels write_method mem_access_time word_size block_size cache1_lines_no cache1_associativity cache1_hit_time cache2_lines_no cache2_associativity cache2_hit_time cache3_lines_no cache3_associativity cache3_hit_time \n\t trace_file: name of file containing byte addresses trace \n\t no_of_cache_levels: how many levels of cache \n\t write_method: it can be either '0' for write through or '1' for write back \n\t mem_access_time: access time for main memory \n\t word_size: size of word in bytes \n\t block_size: size of block in number of words \n\t cache1_lines_no: number of sets or lines in 1st level cache \n\t cache1_associativity: associatvity for 1st level cache \n\t cache1_hit_time: time to access 1st level cache \n\t other parameters are not needed if no_of_cache_levels is entered 1; if no_of_cache_levels entered 2, then provide cache2_lines_no, cache2_associativity, cache2_hit_time parameters similar to 1st level cache; if no_of_cache_levels entered 2, then provide cache3_lines_no, cache3_associativity, cache3_hit_time for 3rd level cache similar to 1st level cache."

int main(int argc, char** argv)
{
	// check to see if the number of argument is correct
	if (argc < 10) {
	cout << Usage;
	exit(1);
	}
	
	char *fname; // to read file name
	int wordSize,blockSize; // common word size in bytes for all levels of cache; 
	// assumed common block size for easy tracking of dirty blocks replacement accross multi cache levels

	// individual parameters of cache
	int blockSize1, cacheLines1, assoc1,blockSize2, cacheLines2, assoc2,blockSize3, cacheLines3, assoc3, hitTime1, hitTime2, hitTime3; 
	// variable setting of cache levels and write method (write back-1, write through-0); // write allocate is assumed for both
	int noOfCache, writeMethod, memAccessTime; 
	
	// declaration of two dimensional pointer arrays to store tags, lru values and dirty bit status for all 3 levels of cache
	int **cache1,**cache2,**cache3,**LRU1,**LRU2,**LRU3,**dirty1,**dirty2,**dirty3; 
	
	long byteAdd, wordAdd, blockAdd; // variables to store diff. version of address corresponding 	
	int setNo; // no. of line in cache address belong to 
	int tag; // store tag part of an address read for different level of cache
	
	// calculation variables declaration
	int totalAccess=0, totalRead=0, totalWrite=0, cache1Access=0, cache2Access=0, cache3Access=0;
	int cache1Read=0, cache2Read=0, cache3Read=0, cache1Write=0, cache2Write=0, cache3Write=0;
	int cache1ReadHit=0, cache2ReadHit=0, cache3ReadHit=0, cache1ReadMiss=0, cache2ReadMiss=0, cache3ReadMiss=0;
	int cache1WriteHit=0, cache2WriteHit=0, cache3WriteHit=0, cache1WriteMiss=0, cache2WriteMiss=0, cache3WriteMiss=0;
	int cache1ReadDirty=0,cache2ReadDirty=0,cache3ReadDirty=0,cache1WriteDirty=0,cache2WriteDirty=0,cache3WriteDirty=0;	
	
	int i, j, k; // variables to be used during loops and other ops
	
	// configuration from command line
	fname=argv[1];	
	noOfCache=atoi(argv[2]);
	writeMethod=atoi(argv[3]);
	memAccessTime=atoi(argv[4]);	
	wordSize=atoi(argv[5]);
	blockSize=atoi(argv[6]);
	// read settings for cache 1
	blockSize1=blockSize; // block size made same for convienience of tracking dirty blocks through levels of cache
	cacheLines1=atoi(argv[7]);
	assoc1=atoi(argv[8]);
	hitTime1=atoi(argv[9]);
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

	// initialize cache, lru and dirty bits for cache 1 so no garbage values will be present
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
		blockSize2=blockSize; 
		cacheLines2=atoi(argv[10]);
		assoc2=atoi(argv[11]);	
		hitTime2=atoi(argv[12]);
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

	if(noOfCache>2)
	{
		blockSize3=blockSize; 
		cacheLines3=atoi(argv[13]); 
		assoc3=atoi(argv[14]);
		hitTime3=atoi(argv[15]);
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
		hit=0; pos=0;
		// increment 1st level cache
		cache1Access++;	

		if(op=='R') // if it is read access
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
						// if next level of cache is there, then the dirty block to be replaced is written to next level and marked dirty
						if(noOfCache>1)
						{
							int dirtyBlockAdd=cache1[setNo][pos]*cacheLines1+setNo;
							// for the dirty block address in cache1, get set no and tag in cache 2									
							int dirtyCache2SetNo=getCacheSetNumber(dirtyBlockAdd,cacheLines2);
							int dirtyCache2tag=getcacheTag(dirtyBlockAdd,cacheLines2);
							for(i=0;i<assoc2;i++)
							{
								if(cache2[dirtyCache2SetNo][i]==dirtyCache2tag)
								{
									dirty2[dirtyCache2SetNo][i]=1;
									break;
								}
							}
							
						}
					}
				}	
	
				cache1[setNo][pos]=tag; // replace tag of the least recently used block

				setLRUUnitHighest(LRU1,setNo,assoc1,pos); // set highest value in LRU
					
				if(noOfCache>1) // if there is more than one level cache
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
								// if next level of cache is there, then the dirty block to be replaced is written to next level and marked dirty
								if(noOfCache>2)
								{
									int dirtyBlockAdd=cache2[setNo][pos]*cacheLines2+setNo;
									// for the dirty block address in cache2, get set no and tag in cache 3
									int dirtyCache3SetNo=getCacheSetNumber(dirtyBlockAdd,cacheLines3);
									int dirtyCache3tag=getcacheTag(dirtyBlockAdd,cacheLines3);
									for(i=0;i<assoc3;i++)
									{
										if(cache3[dirtyCache3SetNo][i]==dirtyCache3tag)
										{
											dirty3[dirtyCache3SetNo][i]=1;
											break;
										}
									}
							
								}
							}
						}	
						cache2[setNo][pos]=tag; // replace tag of the least recently used block
						setLRUUnitHighest(LRU2,setNo,assoc2,pos); // set highest value in LRU
						
						if(noOfCache>2) // if there is more than 2 levels of cache
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
		else // if it is write
		{
			totalWrite++;
			cache1Write++;			
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
				cache1WriteHit++;			
				setLRUUnitHighest(LRU1,setNo,assoc1,pos); // set highest value in LRU	
				// set dirty bit for the block to signify change been made to the data in the block
				dirty1[setNo][pos]=1;				 
			}
			else
			{

				cache1WriteMiss++;
				pos=LRU1[setNo][0]; // least recently used block position
				if(writeMethod==1) // '1' corresponds to write back
				{
					// check if the block to be replaced is dirty or not
					if(dirty1[setNo][pos]==1)
					{
						cache1WriteDirty++;
						dirty1[setNo][pos]==0;
						// if next level of cache is there, then the dirty block to be replaced is written to next level and marked dirty
						if(noOfCache>1)
						{
							int dirtyBlockAdd=cache1[setNo][pos]*cacheLines1+setNo;
							// for the dirty block address in cache1, get set no and tag in cache 2									
							int dirtyCache2SetNo=getCacheSetNumber(dirtyBlockAdd,cacheLines2);
							int dirtyCache2tag=getcacheTag(dirtyBlockAdd,cacheLines2);
							for(i=0;i<assoc2;i++)
							{
								if(cache2[dirtyCache2SetNo][i]==dirtyCache2tag)
								{
									dirty2[dirtyCache2SetNo][i]=1;
									break;
								}
							}
							
						}
					}
				}
				cache1[setNo][pos]=tag; // replace tag of the least recently used block
				setLRUUnitHighest(LRU1,setNo,assoc1,pos); // set highest value in LRU
		
				if(noOfCache>1)
				{
					// check in 2nd level cache
					cache2Access++;
					cache2Write++;
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
						cache2WriteHit++;				
						setLRUUnitHighest(LRU2,setNo,assoc2,pos); // set highest value in LRU					 
						dirty2[setNo][pos]=1; // set dirty bit to signify changes been made to the block
					}
					else
					{
						cache2WriteMiss++;
						pos=LRU2[setNo][0]; // least recently used block position
						if(writeMethod==1) // '1' corresponds to write back
						{
							// check if the block to be replaced is dirty or not
							if(dirty2[setNo][pos]==1)
							{
								cache2WriteDirty++;
								dirty2[setNo][pos]==0;
								// if next level of cache is there, then the dirty block to be replaced is written to next level and marked dirty
								if(noOfCache>2)
								{
									int dirtyBlockAdd=cache2[setNo][pos]*cacheLines2+setNo;
									// for the dirty block address in cache2, get set no and tag in cache 3
									int dirtyCache3SetNo=getCacheSetNumber(dirtyBlockAdd,cacheLines3);
									int dirtyCache3tag=getcacheTag(dirtyBlockAdd,cacheLines3);
									for(i=0;i<assoc3;i++)
									{
										if(cache3[dirtyCache3SetNo][i]==dirtyCache3tag)
										{
											dirty3[dirtyCache3SetNo][i]=1;
											break;
										}
									}
							
								}
							}
						}	
						cache2[setNo][pos]=tag; // replace tag of the least recently used block
						setLRUUnitHighest(LRU2,setNo,assoc2,pos); // set highest value in LRU

						if(noOfCache>2)
						{
							// check in 3rd level cache
							cache3Access++;
							cache3Write++;
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
								cache3WriteHit++;				
								setLRUUnitHighest(LRU3,setNo,assoc3,pos); // set highest value in LRU					 
								dirty3[setNo][pos]=1; // set dirty bit to signify changes been made to data in the block
							}
							else
							{

								cache3WriteMiss++;
								pos=LRU3[setNo][0]; // least recently used block position
								if(writeMethod==1) // '1' corresponds to write back
								{
									// check if the block to be replaced is dirty or not
									if(dirty3[setNo][pos]==1)
									{
										cache3WriteDirty++;
										dirty3[setNo][pos]==0;
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
	}
	
	// calculate AMAT
	double AMATRead, AMATWrite, rHitRate1, rMissRate1, rHitRate2, rMissRate2, rHitRate3, rMissRate3, wHitRate1, wMissRate1, wHitRate2, wMissRate2, wHitRate3, wMissRate3;
	// calculate read and write miss rate for each levels of cache available
	rMissRate1=cache1ReadMiss/(double)cache1Read;
	wMissRate1=cache1WriteMiss/(double)cache1Write;
	if(noOfCache>1)
	{
		rMissRate2=cache2ReadMiss/(double)cache2Read;
		wMissRate2=cache2WriteMiss/(double)cache2Write;
	}
	if(noOfCache>2)
	{
		rMissRate3=cache3ReadMiss/(double)cache3Read;
		wMissRate3=cache3WriteMiss/(double)cache3Write;
	}
	
	// calculate AMAT for write back method
	double rDBRRate1, rDBRRate2, rDBRRate3, wDBRRate1, wDBRRate2, wDBRRate3; // read and write dirty blocks removed rate for each levels of cache
	if(writeMethod==1) // write back
	{
		rDBRRate1=cache1ReadDirty/(double)cache1ReadMiss;
		wDBRRate1=cache1WriteDirty/(double)cache1WriteMiss;
		if(noOfCache==1)
		{
			AMATRead=hitTime1+rMissRate1*(memAccessTime + rDBRRate1*memAccessTime);
			AMATWrite=hitTime1+wMissRate1*(memAccessTime + wDBRRate1*memAccessTime);
		}
		else if(noOfCache==2)
		{
			rDBRRate2=cache2ReadDirty/(double)cache2ReadMiss;
			wDBRRate2=cache2WriteDirty/(double)cache2WriteMiss;
			AMATRead=hitTime1+rMissRate1*(hitTime2+rDBRRate1*hitTime2+rMissRate2*(memAccessTime+rDBRRate2*memAccessTime));
			AMATWrite=hitTime1+wMissRate1*(hitTime2+wDBRRate1*hitTime2+wMissRate2*(memAccessTime+wDBRRate2*memAccessTime));
		}
		else
		{
			rDBRRate2=cache2ReadDirty/(double)cache2ReadMiss;
			rDBRRate3=cache3ReadDirty/(double)cache3ReadMiss;
			wDBRRate2=cache2WriteDirty/(double)cache2WriteMiss;
			wDBRRate3=cache3WriteDirty/(double)cache3WriteMiss;
			AMATRead=hitTime1+rMissRate1*(hitTime2+rDBRRate1*hitTime2+rMissRate2*(hitTime3+rDBRRate2*hitTime3+rMissRate3*(memAccessTime+rDBRRate3*memAccessTime)));
			AMATWrite=hitTime1+wMissRate1*(hitTime2+wDBRRate1*hitTime2+wMissRate2*(hitTime3+wDBRRate2*hitTime3+wMissRate3*(memAccessTime+wDBRRate3*memAccessTime)));
		}
	}
	else // write through // caculate AMAT
	{
		if(noOfCache==1)
		{
			AMATRead=hitTime1+rMissRate1*(memAccessTime);
			AMATWrite=hitTime1+memAccessTime+wMissRate1*(memAccessTime);
		}
		else if(noOfCache==2)
		{
			AMATRead=hitTime1+rMissRate1*(hitTime2+rMissRate2*(memAccessTime));
			AMATWrite=hitTime1+hitTime2+memAccessTime+wMissRate1*(hitTime2+wMissRate2*(memAccessTime));
		}
		else
		{
			AMATRead=hitTime1+rMissRate1*(hitTime2+rMissRate2*(hitTime3+rMissRate3*(memAccessTime)));
			AMATWrite=hitTime1+hitTime2+hitTime3+memAccessTime+wMissRate1*(hitTime2+wMissRate2*(hitTime3+wMissRate3*(memAccessTime)));
		}
	}
	
	// display of statistics
	
	cout<<"total accesses: "<<totalAccess<<endl;
	cout<<"total read access: "<<totalRead<<endl;
	cout<<"total write access: "<<totalWrite<<endl;
	cout<<"Write method : ";
	if(writeMethod==1)
	{
		cout<<"Write Back";
	}
	else
	{
		cout<<"Write Through";
	}
	cout<<endl;
	
	// cache 1 statistics
	cout<<endl<<"##  cache 1 statistics  ##  "<<endl<<endl;
	cout<<"total cache1 access: "<<cache1Access<<endl;
	cout<<"cache1 read access: "<<cache1Read<<endl;
	cout<<"cache1 read hit: "<<cache1ReadHit<<endl;
	cout<<"cache1 read miss: "<<cache1ReadMiss<<endl;
	cout<<"cache1 read hit ratio: "<< cache1ReadHit/(double)cache1Read<<endl;
	cout<<"cache1 read miss ratio: "<<cache1ReadMiss/(double)cache1Read<<endl;
	if(writeMethod==1)
	{
		cout<<"cache1 dirty blocks removed on read misses : "<<cache1ReadDirty<<endl;
		cout<<"cache1 dirty blocks removed percent on read misses: "<< cache1ReadDirty/(double)cache1ReadMiss<<endl;
	}
	cout<<"cache1 write access: "<<cache1Write<<endl;	
	cout<<"cache1 write hit: "<<cache1WriteHit<<endl;
	cout<<"cache1 write miss: "<<cache1WriteMiss<<endl;
	cout<<"cache1 write hit ratio: "<<cache1WriteHit/(double)cache1Write<<endl;
	cout<<"cache1 write miss ratio: "<<cache1WriteMiss/(double)cache1Write<<endl;
	if(writeMethod==1)
	{
		cout<<"cache1 dirty blocks removed on write misses : "<<cache1WriteDirty<<endl;
		cout<<"cache1 dirty blocks removed percent on write misses: "<< cache1WriteDirty/(double)cache1ReadMiss<<endl;
	}

	// cache 2 statistics
	if(noOfCache>1)
	{
		cout<<endl<<endl<<"##  cache 2 statistics  ##  "<<endl<<endl;	
		cout<<"total cache2 access: "<<cache2Access<<endl;
		cout<<"cache2 read access: "<<cache2Read<<endl;
		cout<<"cache2 read hit: "<<cache2ReadHit<<endl;
		cout<<"cache2 read miss: "<<cache2ReadMiss<<endl;
		cout<<"cache2 read hit ratio: "<< cache2ReadHit/(double)cache2Read<<endl;
		cout<<"cache2 read miss ratio: "<<cache2ReadMiss/(double)cache2Read<<endl;
		if(writeMethod==1)
		{
			cout<<"cache2 dirty blocks removed on read misses : "<<cache2ReadDirty<<endl;
			cout<<"cache2 dirty blocks removed percent on read misses: "<< cache2ReadDirty/(double)cache2ReadMiss<<endl;
		}
		cout<<"cache2 write access: "<<cache2Write<<endl;		
		cout<<"cache2 write hit: "<<cache2WriteHit<<endl;
		cout<<"cache2 write miss: "<<cache2WriteMiss<<endl;
		cout<<"cache2 write hit ratio: "<<cache2WriteHit/(double)cache2Write<<endl;
		cout<<"cache2 write miss ratio: "<<cache2WriteMiss/(double)cache2Write<<endl;
		if(writeMethod==1)
		{
			cout<<"cache2 dirty blocks removed on write misses : "<<cache2WriteDirty<<endl;
			cout<<"cache2 dirty blocks removed percent on write misses: "<< cache2WriteDirty/(double)cache2ReadMiss<<endl;
		}

	}
	if(noOfCache>2)
	{
		cout<<endl<<endl<<"##  cache 3 statistics  ##  "<<endl<<endl;	
		
		cout<<"total cache3 access: "<<cache3Access<<endl;
		cout<<"cache3 read access: "<<cache3Read<<endl;
		cout<<"cache3 read hit: "<<cache3ReadHit<<endl;
		cout<<"cache3 read miss: "<<cache3ReadMiss<<endl;
		cout<<"cache3 read hit ratio: "<< cache3ReadHit/(double)cache3Read<<endl;
		cout<<"cache3 read miss ratio: "<<cache3ReadMiss/(double)cache3Read<<endl;
		if(writeMethod==1)
		{
			cout<<"cache3 dirty blocks removed on read misses : "<<cache3ReadDirty<<endl;
			cout<<"cache3 dirty blocks removed percent on read misses: "<< cache3ReadDirty/(double)cache3ReadMiss<<endl;
		}
		cout<<"cache3 write access: "<<cache3Write<<endl;		
		cout<<"cache3 write hit: "<<cache3WriteHit<<endl;
		cout<<"cache3 write miss: "<<cache3WriteMiss<<endl;
		cout<<"cache3 write hit ratio: "<<cache3WriteHit/(double)cache3Write<<endl;
		cout<<"cache3 write miss ratio: "<<cache3WriteMiss/(double)cache3Write<<endl;
		if(writeMethod==1)
		{
			cout<<"cache3 dirty blocks removed on write misses : "<<cache3WriteDirty<<endl;
			cout<<"cache3 dirty blocks removed percent on write misses: "<< cache3WriteDirty/(double)cache3ReadMiss<<endl;
		}
	}

	// display AMAT read and write and as a whole
	cout<<endl<<"** AMAT **"<<endl;
	cout<<"Read AMAT : "<<AMATRead<<endl;
	cout<<"Write AMAT : "<<AMATWrite<<endl;
	cout<<"AMAT : "<<((double)AMATRead*(double)totalRead + (double)AMATWrite*(double)totalWrite)/(double)totalAccess<<endl;

	
  return 0;
}

long convertHexToDecimal(char *hexString)
{
	return strtol(hexString,0,16);
}

// returns cache line/set number for block address and number of lines
int getCacheSetNumber(long blockAdd, int cacheLines)
{
	return (int)(blockAdd % cacheLines);
}
//returns word address for byte address and word size
long getWordAddress(long byteAdd, int wordSize)
{
	return byteAdd/(long)wordSize;
}
// returns block address for word address and block size
long getBlockAddress(long wordAdd, int blockSize)
{
	return wordAdd/(long)blockSize;
}

// returns cache tag
int getcacheTag(long blockAdd, int cacheLines)
{
	return (int)blockAdd/cacheLines;
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
