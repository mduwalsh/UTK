#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>

#define PROMPT "user@tshell:"
#define FSYS "tfs"             // file name
#define FSIZE 100              // file size in mb
#define BLOCKSIZE (4*1024)     // block size in bytes
#define INODEBLOCKS 25         // no. of inode blocks
#define MAXLENGTH 255          // max limit of file or directory name
#define INODESIZE 64           // size of inode structure
#define FTSIZE 150             // file table size
#define INODESINBlOCK (int)(BLOCKSIZE)/INODESIZE         // no. of inodes within an inode block


// defines superblock structure
typedef struct superblock{
  int size;             // total no. of disk blocks
  int iSize;            // total no. of inode blocks
  int first_inode;      // pointer to first inode block
  int first_dblock;     // pointer to first data block
  int next_fdblock;     // pointer to 1st free block in freelist
  int next_finode;     // pointer to 1st free inode  
  int root_inum;        // inode number of root directory
} SuperBlock;

// inode structure
typedef struct inode{
  int flags;              // flag to identify type of file (directory, regular or link); 0: unused, 1: directory, 2: file
  int fSize;              // file size
  int pointer[13];        // array of ints as pointers to data blocks
} Inode;


// free list structure
typedef struct flist {
  int *flink;  
} *Flist; 

// inode block
typedef struct inode_block{
  Inode node[INODESINBlOCK];
} InodeBlock;

// indirect block
typedef struct indirect_block{
  int pointer[(int)(BLOCKSIZE)/4];
} IndirectBlock;


// file descriptor structure
typedef struct file_descriptor{
  Inode *fp;              // pointer to inode struct of opened file
  int   inum;             // inode number
  int   csp;              // current seek pointer in opened file; this relative to start of each individual file
  int   mode;             // file mode 1: read only, 2: write only, 3: read/write
} FileDescriptor;

// file table structure
typedef struct file_table{
  int count;               // no. of opened files
  bool bitmap[FTSIZE];              // bitmap 1: denotes opened file/ 0: open slot
  FileDescriptor fdArray[FTSIZE]; // array of file descriptor; index to this array is fd of file represented by it 
} FileTable;

// directory entry structure
typedef struct d_entry{
  int  inum;    // inode number 
  int  reclen;  // entry length / size // not sure if needed right now
  int  strlen;  // length of entry name string  // not sure if need right now
  char name[MAXLENGTH];   // name of entry / file or directory name  
  //struct d_entry *nentry;
} DEntry;

// directory structure
typedef struct directory{
  DEntry entry;
  struct directory * ndir;
} Directory;

// structure for storing file system contexts
typedef struct file_system_context{
  int home;            // inum of home directory
  int cwd;             // inum of current workding directory
} FileSystemContext;

// global variables 
typedef void (*sighandler_t)(int);
static char *MY_ARGV[100];   // global variable to hold command line arguments
static int ARGC;             // static global variable to hold no. of command arguments 
static char PromptPath[1024] = "";   // path to be added while displaying shell prompt
SuperBlock SBlock;              // in memory copy of super block of file system
FileSystemContext FSContext;    // holds file system contexts till shell is running
FileTable FTable;               // file table to keep hold and track of open files
FILE *FSP;                     // file pointer to disk file system

// initializes file table
void init_file_table()
{
  int i;
  i = FTSIZE;
  FTable.count = 0;
  while(i--) FTable.bitmap[i] = 0;  
}

// reads superblock
int read_superblock(int bno, SuperBlock *block)
{
  int i;
  if(fseek(FSP, bno*BLOCKSIZE, SEEK_SET)) {printf("Error while reading superblock!! Couldn't get specified block on file!!"); return 1;}
  i = fread(block, sizeof(SuperBlock), 1, FSP);
  if(i != 1){ printf("Error!! SuperBlock couldn't be read!! %d\n", i); return 1;}
  return 0;
}

// reads inodeblock
int read_inodeblock(int bno, InodeBlock *block)
{
  int i; 
  if(fseek(FSP, bno*BLOCKSIZE, SEEK_SET)) {printf("Error while reading inodeblock!! Couldn't get %d block on file!!\n", bno); return 1;}
  i = fread(block, sizeof(InodeBlock), 1, FSP); 
  if(i != 1){ printf("Error!! InodeBlock couldn't be read!!\n"); return 1;}	
  return 0;
}

// method to write superblock into the filesystem
void write_superblock(int bno, SuperBlock block)
{
  int i;
  if(fseek(FSP, bno*BLOCKSIZE, SEEK_SET)) {printf("Error while writing superblock!! Couldn't get specified block on file!!\n"); return;}
  i = fwrite(&block, sizeof(SuperBlock), 1, FSP);
  if(i != 1) printf("Error!! SuperBlock couldn't be written!! \n");
}

// method to write inodeblock into the filesystem
void write_inodeblock(int bno, InodeBlock block)
{
  int i; 
  if(fseek(FSP, bno*BLOCKSIZE, SEEK_SET)) {printf("Error while writing inode block!! Couldn't get %d block on file!!", bno); return;}
  i = fwrite(&block, sizeof(InodeBlock), 1, FSP);
  if(i != 1){
    printf("Error!! Inode block couldn't be written on block %d!!\n", bno);
  }
}

// modifies an inode of given index in an inode block 
void modify_inode_in_block(Inode *src, InodeBlock *iblk, int index)
{
  int i;
  iblk->node[index].flags = src->flags;
  iblk->node[index].fSize = src->fSize;
  for(i = 0; i < 13; i++) iblk->node[index].pointer[i] = src->pointer[i];
}

void copy_inode(Inode *src, Inode *dst)
{
  int i;
  dst->flags = src->flags;
  dst->fSize = src->fSize;
  for(i = 0; i < 13; i++) dst->pointer[i] = src->pointer[i];
}

// reads inode for given inum
void read_inode(Inode *inode, int inum){
  // find inode block with inum inode number
  int iblk  = (inum / INODESINBlOCK) + SBlock.first_inode;  // no. of inode block in which inode is; offset of first_inode is added
  int index = (inum-1) % INODESINBlOCK;   // no. of inodes after which the inode starts; minus 1 is because inum starts from 1
  int i;
  InodeBlock block;
  // read the inode block
  if(i = read_inodeblock(iblk, &block)){
    printf("Error!! Couldn't read inodeblock  from disk!!\n");
    return;
  }
  copy_inode(&block.node[index], inode);
}

// writes inode into specified index number
void write_inode(Inode *inode, int inum){
  // find inode block with inum inode number
  int iblk  = (inum / INODESINBlOCK) + SBlock.first_inode;  // no. of inode block in which inode is; offset of first_inode is added
  int index = (inum-1) % INODESINBlOCK;   // no. of inodes after which the inode starts; minus 1 is because inum starts from 1
  int i;
  InodeBlock block;
  // read the inode block
  if(i = read_inodeblock(iblk, &block)){
    printf("Error!! Couldn't read inodeblock  from disk!!\n");
    return;
  }
  // modify the inode with that inode number
  modify_inode_in_block(inode, &block, index);
  // write the inode block back
  write_inodeblock(iblk, block);
}

// writes integer in specified block
void write_integer(int bno, int data)
{
   int i;
   if(fseek(FSP, bno*BLOCKSIZE, SEEK_SET)) {printf("Error while writing pointer to data blocks!! Couldn't get %d block on file!!", bno); return;};
  i = fwrite(&data, sizeof(int), 1, FSP);
  if(i != 1) {
    printf("Error!! Couldn't write integer data in %d block!!\n", bno); 
  }
}

// writes indirect block
void write_indirectblock(int bno, IndirectBlock *inblock)
{
   int i; 
   if(fseek(FSP, (bno + SBlock.first_dblock)*BLOCKSIZE, SEEK_SET)){
     printf("Error while writing indirect block!! Couldn't get %d block on file!!", bno); return;
   }
   i = fwrite(&inblock, sizeof(IndirectBlock), 1, FSP);
   if(i != 1){
     printf("Error!! Indirect block couldn't be written on block %d!!\n", bno);
   }
}

// reads indirect block
void read_indirectblock(int bno, IndirectBlock *inblock)
{
   int i; 
   if(fseek(FSP, (bno + SBlock.first_dblock)*BLOCKSIZE, SEEK_SET)){
     printf("Error while reading indirect block!! Couldn't get %d block on file!!", bno); return;
   }
   i = fread(&inblock, sizeof(IndirectBlock), 1, FSP);
   if(i != 1){
     printf("Error!! Indirect block couldn't be read on block %d!!\n", bno);
   }
}

// returns index of pointer to which inode has been used
int get_pindex(Inode *inode)
{
  int pindex; int size;  
  pindex = inode->fSize / (BLOCKSIZE);    
  if(pindex > 10){
    // recalculate pindex
    size = inode->fSize - 10 * BLOCKSIZE;   // remaining size
    pindex = 11 + (size/((BLOCKSIZE/4)*BLOCKSIZE));  // each indirect block contains BLOCKSIZE/4 pointers to data block
  }      
  return pindex;
}

// initialize pointers in indirect block
void init_indirectblock(IndirectBlock *inblock)
{
  int i;
  for(i = 0; i < (int)(BLOCKSIZE)/4; i++){  // set all pointers to -1
     inblock->pointer[i] = -1;
   }
}

// returns pointer index in indirect block to be used for writing
int get_indb_pointer(IndirectBlock *inblock)
{
  int i;
  if(inblock->pointer[0] == -1){  // if 1st pointer is not used, return that
    return 0;
  }
  for(i = 0; i < (int)(BLOCKSIZE)/4; i++){  // if any of pointer is not allocated, return pointer one lower than that
    if(inblock->pointer[i] == -1) break;
  }
  return --i;
}

// writes directory content/ entry into data block with some offset
void write_fcontent(Inode *inode, int blk, int off, char* data)
{
  int i;
  fseek(FSP, (SBlock.first_dblock + blk)*BLOCKSIZE + off, SEEK_SET );
  i = fwrite(&data, strlen(data), 1, FSP);
  if(i != 1){
    printf("Error!! File content couldn't be written on block %d!!\n", blk);
  }
  inode->fSize = inode->fSize + strlen(data);
}


// writes directory content/ entry into data block with some offset
void write_dcontent(Inode *inode, int blk, int off, DEntry *de)
{
  int i;
  fseek(FSP, (SBlock.first_dblock + blk)*BLOCKSIZE + off, SEEK_SET );
  i = fwrite(de, sizeof(DEntry), 1, FSP);
  if(i != 1){
    printf("Error!! DEntry content couldn't be written on block %d!!\n", blk);
  }
  inode->fSize = inode->fSize + sizeof(DEntry);
}

// writes directory content in indirect block
void write_dcontent_indblock(Inode *inode, int pindex, DEntry *de)
{
  int off, ptr;
  int blkno;
  IndirectBlock inblock;
  if(inode->pointer[pindex] == -1){
    // allocate block to that pointer
    init_indirectblock(&inblock);
  }
  else{
    blkno = inode->pointer[pindex];
    // read indirect block
    read_indirectblock(blkno, &inblock); 
  }   
  // find pointer to use
  ptr = get_indb_pointer(&inblock);
  // find size already written to the block
  off = inode->fSize - 10*BLOCKSIZE -(pindex-11)*(BLOCKSIZE/4*BLOCKSIZE) - ptr*BLOCKSIZE;
  // if data block full, allocate new pointer 
  off = (off < 0)? 0:off;
  if(off + sizeof(DEntry) > BLOCKSIZE){
    // increase pointer of indirect block ptr++
    ptr++;
    if(ptr >= (BLOCKSIZE/4)){
      // increase pindex
      pindex++;
      if(pindex >=13){
	printf("File limit exceeded!!\n");
      }
      else{
	write_dcontent_indblock(inode, pindex, de);	  
      }
    }
    else{
      // allocate datablock for that ptr // update indirect block /inblock.pointer[ptr]
      // write indirect block
      write_indirectblock(blkno, &inblock);
      off = 0;
      write_dcontent(inode, inblock.pointer[ptr], off, de);
    }
  }
  else{
    off = off; // write data to data block indexed by ptr in indirect block
    write_dcontent(inode, inblock.pointer[ptr], off, de);
  }
}

// writes directory entry in file
void write_dentry(Inode *inode, DEntry *de)
{
  int dblk, off, pindex, size;
  pindex = get_pindex(inode);
  if(pindex <= 10){
    if(inode->pointer[pindex] == -1){
      //allocate a block to that pointer
      //for now
      if(de->inum == 1) inode->pointer[pindex] = 0; 
      else
	inode->pointer[pindex] == 1;
    }
    off = inode->fSize - (pindex * BLOCKSIZE);
    if(off + sizeof(DEntry) > BLOCKSIZE){
      pindex++; off = 0;
      if(pindex < 11){
	// allocate block to inode->pointer[pindex]
	dblk = inode->pointer[pindex];
	write_dcontent(inode, dblk, off, de);
      }
      else{
	// allocate indirect block; get indirect block no in inode->pointer[pindex]
	  
	write_dcontent_indblock(inode, pindex, de);	
      }
    }
    else{ // write content with offset off to block indexed by pindex data block
      dblk = inode->pointer[pindex];
      write_dcontent(inode, dblk, off, de);
    }
  }
  if(pindex >10){ //indirect block
    write_dcontent_indblock(inode, pindex, de);	
  }
 
}

// creates root directory 
static int create_root()
{
  DEntry d1, d2;
  d1.inum = 1;   // inum of root directory is reserved 1
  strcpy(d1.name, ".");
  d1.strlen = strlen(d1.name);
  d2.inum = 1;   // parent of root directory is root
  strcpy(d2.name, "..");
  d2.strlen = strlen(d2.name); 
    
  Inode inode;
  inode.flags = 1;  // type directory
  inode.fSize = 0;  // initialize size to 0
  write_dentry(&inode, &d1);     // write directory content to data block made available to inode and update inode   
  write_dentry(&inode, &d2);     // write directory content to data block made available to inode and update inode   
  write_inode(&inode, 1);       // write inode to place holder for that 1st inode
  fseek(FSP, (0+SBlock.first_dblock)*BLOCKSIZE ,SEEK_SET);
  return 0;      // success
}

// creates directory with give name inside given parent inum
void create_directory(char *name, int pinum)
{
  int inum;
  //todo: find free inum
  
  // create inode object for it ; 
  DEntry d1, d2;
  d1.inum = inum;   // inum of newly created directory
  strcpy(d1.name, ".");
  d1.strlen = strlen(d1.name);
  d2.inum = pinum;   // parent directory's inum
  strcpy(d2.name, "..");
  d2.strlen = strlen(d2.name); 
    
  Inode inode;
  inode.flags = 1;  // type directory
  inode.fSize = 0;  // initialize size to 0
  write_dentry(&inode, &d1);     // write directory content to data block made available to inode and update inode   
  write_dentry(&inode, &d2);     // write directory content to data block made available to inode and update inode   
  write_inode(&inode, inum);       // write inode to place holder for that 1st inode
  Inode pinode;
  read_inode(&pinode, pinum);    // read parent inode
  strcpy(d1.name, name);
  write_dentry(&pinode, &d1);    // add entry to parent directory
  write_inode(&pinode, pinum);
  // directory is created

}

void copy_dentry(Dentry *src, Dentry *dst)
{
  src->inum = dst->inum;
  strcpy(src->name, dst->name);
  src->strlen = dst->strlen;
  src->reclen = dst->reclen;
}

void free_directory(Directory *d)
{
  Directory **c, t;
  c = d;
  Directory
  while(c != 0){
    t = 
  }

    temp = node          # save node pointer.
    node = node.next     # advance to next.
    free temp            # free the saved one.
head = null  
}

// returns status of search entry by name of entry and also returns pointer to entry if found
int get_entry_by_name(int pinum, char *name, Dentry * entry)
{
  Inode inode;
  read_inode(&inode, pinum);
  int dblk, off, pindex, size;
  if(inode.fSize == 0) return 0;   // no record found status
  
  pindex = get_pindex(inode);   // index of pointer that has been used up to
  if(pindex == 0 && inode.pointer[pindex] == -1) return 1;
  Directory *d, *c;
  int read;
  bool found = 0;
  d = malloc(sizeof(Directory));
  c = d;
  if(pindex <= 10){
    for(i = 0; i < pindex; i++){      
      // read one by one for all possible blocks and if found break
      fseek(FSP, (inode.pointer[i]+SBlock.first_dblock)*BLOCKSIZE, SEEK_SET);
      read = 0;
      while(read < BLOCKSIZE){  // read max up to BLOCKSIZE
	fread(&c->entry, sizeof(DEntry), 1, FSP);   // read DEntry
	read = read + sizeof(DEntry);
	c->ndir = 0;
	if(c->entry.inum !=0 && strcmp(c->entry.name, name)==0){  // if name matches
	  c->ndir = 0;
	  found = 1;
	  break;
	}
	else{ //if not found allocate memory for ndir 
	  c->ndir = malloc(sizeof(Directory));  //
	  c = c->ndir;
	}
      }
      if(found) break;  // found then break the loop      
    }
    //if not found read offset too
    if(!found){
      off = inode.fSize - (pindex * BLOCKSIZE);
      // read one by one for all possible blocks and if found break
      fseek(FSP, (inode.pointer[pindex]+SBlock.first_dblock)*BLOCKSIZE, SEEK_SET);
      read = 0;
      while(read < off){  // read max up to off
	fread(&c->entry, sizeof(DEntry), 1, FSP);   // read DEntry
	read = read + sizeof(DEntry);
	c->ndir = 0;	  
	if(c->entry.inum !=0 && strcmp(c->entry.name, name)==0){  // if name matches
	  found = 1;
	  break;
	}
	else{ //if not found allocate memory for ndir 
	  c->ndir = malloc(sizeof(Directory));  //
	  c = c->ndir;
	}
      }
      if(!found){  // not found
	return 0;
      }
    }
    else{
      // fill entry
      copy_dentry(&c->entry, entry);
      return 1;
    }    
    
  } 
  /*if(pindex >10){ //indirect block
    for(i = 0; i < 11; i++){
       // read one by one for all possible blocks and if found break
    }
    //if not found
    Indirect inblock[pindex-10];
    for(i = 11; i < pindex; i++){
      read_indirectblock(&inblock[i], inode.pointer[pindex]);
      for(j = 0; j < BLOCKSIZE/4; j++){
	// read one by one for all j pointed blocks and if found break out of all
      }
      // if found true break
      // find pointer to use
      ptr = get_indb_pointer(&inblock[pindex]);
      for(j = 0; j < ptr; j++){
	// read one by one for all j pointed blocks and if found break out of all
      }
      // if found break else
      // find size already written to the block
      off = inode->fSize - 10*BLOCKSIZE -(pindex-11)*(BLOCKSIZE/4*BLOCKSIZE) - ptr*BLOCKSIZE;
      if(off >= sizeof(DEntry)){
	//read offset too
      }
      }
    
  }*/
  // free malloced space
  
}


// format file system
void format_fs()
{
  int i, j, h;
  // initialize superblock
  SBlock.size         = (FSIZE*1024*1024)/BLOCKSIZE;   // total blocks in file system
  SBlock.iSize        = INODEBLOCKS;              // no. of inode blocks 
  SBlock.first_inode  = 1;     // inode starts at block 1; constant throughout
  SBlock.first_dblock = SBlock.iSize + 1;  // data blocks starts after iSize+1 blocks; constant throughout (no. of Superblock + no. of inode blocks)
  SBlock.next_fdblock = SBlock.iSize + 1; // every data block is free at first
  SBlock.next_finode  = 1;              // inode starts at 1 not 0;
  SBlock.root_inum    = 1;    // inode 1 is assigned for root directory  

  // write all inode blocks with each inode flags set to 0
  InodeBlock iblock[INODEBLOCKS];
  for(i = 0; i < INODEBLOCKS; i++){     // go through all inode blocks
    for(j = 0; j < (int)(BLOCKSIZE)/INODESIZE; j++){         // go through all inodes inside each inode block
      iblock[i].node[j].flags = 0;   // set nodes are free
      for(h = 0; h < 13; h++)
	iblock[i].node[j].pointer[h] = -1;
    }
    write_inodeblock(i+1, iblock[i]);   // write inode block
  }

  // link freelist pointers in data blocks
  for(i = 0; i < (FSIZE-(INODEBLOCKS+1) - 1); i++){  // go through all data blocks execept last data block (because no where to point)
    // write freelist pointers in each data block starting from SBlock.first_dblock
    write_integer(SBlock.first_dblock+i, i+1);
  }

  // create root directory ; update superblock
  if(create_root()) { 
    printf("Error creating root directory!! Can not format disk!!\n"); 
    return;
  }
  write_superblock(0, SBlock);    // write superblock to disk  
}

// lauches file system and shell will be ready to use
void launch_fs()
{
  // read superblock into memory
  if(read_superblock(0, &SBlock)){
    printf("Error!! Couldn't read superblock from disk!!\n");
    return;
  }
  // initialize file table
  init_file_table();
  // initialize file context
  FSContext.home = SBlock.root_inum;   // starting directory at beginning of shell
  FSContext.cwd  = FSContext.home;           // current working directory at beginning of shell  
}

// formats disk space
void create_fs()
{
  if(ARGC>1){
    printf("argc = %d\n", ARGC);
    printf("Error!! Higher number of arguments than expected!!\n"); return;
  }
  
  int i=0 ;
  if(!(FSP = fopen(FSYS, "wb+"))){ printf("Error!! Couldn't create file system!!\n"); return;}
  if((i = posix_fallocate(fileno(FSP), 0, FSIZE*1024*1024))){
    printf("Error!! Couldn't allocate 100 mb disk space!!\n"); 
    fclose(FSP); 
    return;
  }
  format_fs();  // format created file 
  launch_fs();  // launch file system
  printf("Disk formatted!! Ready to use!!\n");  
  /*InodeBlock blk;
  read_inodeblock(1, &blk);
  printf("Flags and size are %d %d \n", blk.node[0].flags, blk.node[0].fSize);*/
}

// executes command
void exec_cmd(char *cmd)
{
  if(strcmp(cmd, "mkfs") == 0){
      create_fs();
  }
}



void handle_signal(int signo)
{
  printf("\n%s%s$ ", PROMPT, PromptPath);
  fflush(stdout);
}

// parses command arguments into max 10 arguments in indexed array 1st one being command itself
void fill_argv(char *tmp_argv)
{
  char *foo = tmp_argv;
  int index = 0;
  char ret[100];
  bzero(ret, 100);
  while(*foo != '\0') {
    if(index == 10)
      break;
    
    if(*foo == ' ') {  // space signifies end of one argument
      if(*(foo+1) == ' ' || *(foo+1) == '\0') {foo++; continue;}  // if next character is also space or null then just move forward
      if(MY_ARGV[index] == NULL)
	MY_ARGV[index] = (char *)malloc(sizeof(char) * strlen(ret) + 1);
      else {
	bzero(MY_ARGV[index], strlen(MY_ARGV[index]));
      }
      strncpy(MY_ARGV[index], ret, strlen(ret));
      strncat(MY_ARGV[index], "\0", 1);
      bzero(ret, 100);
      index++;
    } else {
      strncat(ret, foo, 1);     // concatenates one character at a time to ret string
    }
    foo++;
    /*printf("foo is %c\n", *foo);*/
  }
  MY_ARGV[index] = (char *)malloc(sizeof(char) * strlen(ret) + 1);
  strncpy(MY_ARGV[index], ret, strlen(ret));
  strncat(MY_ARGV[index], "\0", 1);
  ARGC = index + 1;   // no. of arguments
}

// frees MY_ARGV array
void free_argv()
{
  int index;
  for(index=0;MY_ARGV[index]!=NULL;index++) {
    //bzero(MY_ARGV[index], strlen(MY_ARGV[index])+1);
    MY_ARGV[index] = NULL;
    free(MY_ARGV[index]);
  }
}




int main()
{
  char c;
  int i, fd;
  char *tmp = (char *)malloc(sizeof(char) * 100);   // collects command line text; max 100 char assumed
  char *cmd = (char *)malloc(sizeof(char) * 100);   // holds command or value in 0 index of MY_ARGV
  void *t;
  
  signal(SIGINT, SIG_IGN);
  signal(SIGINT, handle_signal);
  
  printf("%s%s$ ", PROMPT, PromptPath);
  fflush(stdout);
  while(c != EOF) {
    c = getchar();
    switch(c) {
    case '\n': if(tmp[0] == '\0') {
        printf("%s%s$ ", PROMPT, PromptPath);
      } else {
	fill_argv(tmp);     // parse command line text into indexed array based on space
	t = strncpy(cmd, MY_ARGV[0], strlen(MY_ARGV[0]));   // string at 0 of MY_ARGV is command to be executed
	t = strncat(cmd, "\0", 1);
	// printf("%s\n", cmd);
	// check if it is our commandlist and evaluate arguments and execute
	exec_cmd(cmd);
	free_argv();
        printf("%s%s$ ", PROMPT, PromptPath);
	bzero(cmd, 100);
      }
      bzero(tmp, 100);
      break;
    default: strncat(tmp, &c, 1);
      break;
    }
  }
  free(tmp);	
  free(cmd);
  printf("\n");
  return 0;
}
