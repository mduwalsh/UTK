#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PROMPT "[VIRTUAL SHELL]:" 
#define FSYS "tfs"             // file name
#define FSIZE 100              // file size in mb
#define BLOCKSIZE (4*1024)     // block size in bytes
#define INODEBLOCKS 25         // no. of inode blocks
#define MAXLENGTH 255          // max limit of file or directory name
#define INODESIZE 100           // size of inode structure
#define FTSIZE 150             // file table size
#define INODESINBlOCK ((int)(BLOCKSIZE)/INODESIZE)         // no. of inodes within an inode block
#define IPOINTS 13             // no. of pointers in an inode


// defines superblock structure
typedef struct superblock{
  int size;             // total no. of disk blocks
  int iSize;            // total no. of inode blocks
  int first_inode;      // pointer to first inode block
  int first_dblock;     // pointer to first data block
  int next_fdblock;     // pointer to 1st free block in freelist
  int next_finode;     // pointer to 1st free inode  // not used now
  int root_inum;        // inode number of root directory
} SuperBlock;

// inode structure
typedef struct inode{
  char c_date[28];        // created date for file or directory  
  int flags;              // flag to identify type of file (directory, regular or link); 0: unused, 1: directory, 2: file, 3: link
  int fSize;              // file size
  int l_count;            // link count
  int pointer[IPOINTS];        // array of ints as pointers to data blocks
  
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
  Inode *inode;              // pointer to inode struct of opened file
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
void *out;

// writes msg on std output console or file
void display_msg(char *msg, FILE *fp)
{
  fprintf(fp, "%s", msg);
  fprintf(fp, "\n");
}


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

// initialise value in inode struct
void init_inode(Inode *inode)
{
  int i;
  inode->flags = 0;
  inode->fSize = 0;
  inode->l_count = 0;
  strcpy(inode->c_date, "");
  for(i = 0; i < IPOINTS; i++) inode->pointer[i] = -1;
}

void copy_inode(Inode *src, Inode *dst)
{
  int i;
  dst->flags = src->flags;
  dst->fSize = src->fSize;
  dst->l_count = src->l_count;
  strcpy(dst->c_date, src->c_date);
  for(i = 0; i < IPOINTS; i++) dst->pointer[i] = src->pointer[i];
}

// reads inode for given inum
void read_inode(Inode *inode, int inum){
  // find inode block with inum inode number
  int iblk  = ((inum-1) /( INODESINBlOCK)) + SBlock.first_inode;  // no. of inode block in which inode is; offset of first_inode is added
  int index = (inum-1) %( INODESINBlOCK);   // no. of inodes after which the inode starts; minus 1 is because inum starts from 1
  int i;
  InodeBlock block;
  // read the inode block
  if(i = read_inodeblock(iblk, &block)){
    printf("Error!! Couldn't read inodeblock  from disk!!\n");
    return;
  }
  copy_inode(&block.node[index], inode);
}

// modifies an inode of given index in an inode block 
void modify_inode_in_block(Inode *src, InodeBlock *iblk, int index)
{
  copy_inode(src, &iblk->node[index]);
}

void copy_inodeblock(InodeBlock *dest, InodeBlock *src)
{
  int i;
  for(i = 0; i < INODESINBlOCK; i++){
    copy_inode(&src->node[i], &dest->node[i]);
  }
}

// method to write inodeblock into the filesystem
void write_inodeblock(int bno, InodeBlock *iblk)
{
  int i,j;
  if(fseek(FSP, bno*BLOCKSIZE, SEEK_SET)) {printf("Error while writing inode block!! Couldn't get %d block on file!!", bno); return;}
  i = fwrite(iblk, sizeof(InodeBlock), 1, FSP);
  if(i != 1){
    printf("Error!! Inode block couldn't be written on block %d!!\n", bno);
  } 
  
}

// writes inode into specified index number
void write_inode(Inode *inode, int inum){
  // find inode block with inum inode number
  int iblk  = ((inum-1) / INODESINBlOCK) + SBlock.first_inode;  // no. of inode block in which inode is; offset of first_inode is added
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
  write_inodeblock(iblk, &block);
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

// returns index of pointer to which inode has been used
int get_pindex(Inode *inode)
{
  int i;
  for(i = IPOINTS; i--;){
    if(inode->pointer[i] != -1){      
      break;
    }
  }
  if(i < 0) i = 0;
  return i;
}

// *** FreeList ***//

// allocate a block to inode pointer; returns 1 if successful
int allocate_block(Inode *inode, int index, char *msg)
{
  // check superblock for next free block
  if(SBlock.next_fdblock == -1){
    strcpy(msg, "No free blocks available. Run out of space!!");
    return 0;
  }
  int blk;
  // point inode pointer to the free block
  inode->pointer[index] = SBlock.next_fdblock;
  // read block pointed by free block
  fseek(FSP, (SBlock.next_fdblock+SBlock.first_dblock)*BLOCKSIZE, SEEK_SET);
  fread(&blk, sizeof(int), 1, FSP);
  SBlock.next_fdblock = blk;         // update next free block in superblock
  // write change in superblock back to file
  write_superblock(0, SBlock);
  return 1;
}

// frees a block from inode pointer; updates next free block in super block
void free_block(Inode *inode, int index)
{
  int blk;
  blk = SBlock.next_fdblock;
  // point to be freed block to free block in superblock 
  fseek(FSP, (inode->pointer[index]+SBlock.first_dblock)*BLOCKSIZE, SEEK_SET);
  fwrite(&blk, sizeof(int), 1, FSP);  
  SBlock.next_fdblock = inode->pointer[index];  // update next free block in superblock to be recently freed block
  inode->pointer[index] = -1;  // deallocate pointer index in inode  
  // write change in superblock back to file
  write_superblock(0, SBlock);
}

// get free inode number; if found returns inumber else returns 0 as status flag
int get_free_inum()
{
  int i, j, found = 0, num = 1, inum;
  Inode inode;
  for(i = 0; i < INODEBLOCKS; i++){ // through all inode blocks
    for(j = 0; j < INODESINBlOCK; j++, num++){  // through all inodes in an inode block
     read_inode(&inode, num);        // read inode inum
      if(inode.flags == 0){  // if free
	found = 1;
	inum = num;
	break;
      }
      
    }
    if(found) break;
  }
  if(found) return num;
  else return 0;
}

// frees whole inode and blocks allocated for pointers in inode
void free_inode(Inode *inode, int inum)
{
  int i;
  // free blocks allocated to inode first
  for(i = 0; i < IPOINTS; i++){
    if(inode->pointer[i] != -1){    // if block allocated
      free_block(inode, i);         // free block
    }
  }
  init_inode(inode);
  write_inode(inode, inum);
}

// *** FreeList ***//



// seeks pointer to the given offset relative to file associated with inode from beginning of the file
void seek_pointer(int fd, int off)
{
  FileDescriptor *f;
  f = &FTable.fdArray[fd];  // get pointer of file descriptor associated with fd
  f->csp = off;
}

// returns block index and offset in the block for file with current seek pointer location
void get_block_index_and_offset_for_csp(int csp, int *bindex, int *boff)
{
  if(csp == 0){ // current seek pointer is at 0, then index and off is 0
    *bindex = 0; *boff = 0;
    return;
  }
  *bindex = (csp-1) / BLOCKSIZE;    // gives index of block in which this offset off lies
  *boff   = csp % BLOCKSIZE;    // gives offset of the block
  if(*boff == 0) *boff = BLOCKSIZE;   // if offset calculated is 0, current seek pointer is at end of the block  
}

// reads file content into buffer
void read_fcontent(int blk, int off, int size, char *buffer)
{
  int i;
  char msg[255];
  fseek(FSP, (SBlock.first_dblock + blk)*BLOCKSIZE + off, SEEK_SET );
  i = fread(buffer, size, 1, FSP);
  if(i != 1){
    sprintf(msg, "Error!! File content couldn't be read from the block %d!!\n", blk);
    //display_msg(msg, stdout);
    printf("%s\n", msg);
    fflush(stdout);
  } 
}

// writes file content/ entry into data block with some offset
void write_fcontent(int blk, int off, char* data)
{
  int i;
  char msg[255];
  fseek(FSP, (SBlock.first_dblock + blk)*BLOCKSIZE + off, SEEK_SET );
  i = fwrite(data, strlen(data), 1, FSP);
  if(i != 1){
    sprintf(msg, "Error!! File content couldn't be written on block %d!!\n", blk);
    //display_msg(msg, stdout);
    printf("%s\n", msg);
    fflush(stdout);
  } 
}

// writes into file with fd as file descriptor and off as offset of the file
void write_to_file(int fd, char *data)
{
  FileDescriptor *f;
  char msg[255];
  if(FTable.bitmap[fd] == 0){ 
    sprintf(msg,"File decriptor %d is not associated with any opened file!!", fd);
    //display_msg(msg, out);
    printf("%s\n", msg);
    fflush(stdout);
    return;
  }
  f = &FTable.fdArray[fd];  // get pointer of file descriptor associated with fd
  if(f->mode != 2 && f->mode !=3){
    sprintf(msg,"File is not open for write!!");
    //display_msg(msg, stdout);
    printf("%s\n", msg);
    fflush(stdout);
    return;
  }
  Inode *inode;
  inode = f->inode;  // inode of file 

  // get current block index and position in the block
  int *bindex, *boff, length, status;
  bindex = malloc(sizeof(int));
  boff = malloc(sizeof(int));
  get_block_index_and_offset_for_csp(f->csp, bindex, boff);  // gets value of block index and block offset from current seek pointer
  
  // start writing data to file
  if(*boff == BLOCKSIZE){
    // increase bindex
    *bindex = *bindex + 1;
    if(*bindex >= IPOINTS){ 
      sprintf(msg,"Size limit exceeded for a file!!");
      //display_msg(msg, out);
      printf("%s\n", msg);
      fflush(stdout);
      free(bindex);
      free(boff);
      return;
    }
    if(inode->pointer[*bindex] == -1){ // block not allocated to pointer
      // allocate block to pointer
      status = allocate_block(inode, *bindex, msg);
      if(status == 0){    // not allocated
	//display_msg(msg, out);
	printf("%s\n", msg);
	fflush(stdout);
	free(bindex);
	free(boff);
	return;
      }  
    }
    *boff = 0;   // set offset to 0 in new block
  }
  // write data in chunks
  char chunk[BLOCKSIZE];
  //check if data size + boff is greater than block size
  if((*boff + strlen(data)) <= BLOCKSIZE){
    // write data to the block
    write_fcontent(inode->pointer[*bindex], *boff, data);
    *boff += strlen(data);
  }
  else{
    // get length of data to fit the block
    length = BLOCKSIZE - *boff;
    strncpy(chunk, data, length);
    data += length;  // move up pointer in the data array
    // write chunk to the block
    write_fcontent(inode->pointer[*bindex], *boff, chunk);
    *boff += length;
    while(strlen(data) > 0){  // if length > 0 write
      *boff = 0;      
      *bindex++;  // increase to next pointer inode
      if(*bindex >= IPOINTS){ 
	sprintf(msg, "Size limit exceeded for a file!!");
	//display_msg(msg, out);
	printf("%s\n", msg);
	fflush(stdout);
	break;
      }
      if(inode->pointer[*bindex] == -1){
      // allocate block to that pointer
	status = allocate_block(inode, *bindex, msg);
	if(status == 0){    // not allocated
	  //display_msg(msg, out);
	  printf("%s\n", msg);
	  fflush(stdout);
	  free(bindex);
	  free(boff);
	  return;
	}  
      }
      if(strlen(data) < BLOCKSIZE) 
	length = strlen(data);
      else
	length = BLOCKSIZE;
      strncpy(chunk, data, length);
      // write chunk to the block
      write_fcontent(inode->pointer[*bindex], *boff, chunk);
      *boff += length;    
      data += length;      
    }
  }
  // update current seek pointer in fd 
  f->csp = (*bindex) * BLOCKSIZE + *boff;
  // update inode for fSize
  // if fSize is less then not need to update fSize because it means it only modified content within a file
  if(inode->fSize < ((*bindex) * BLOCKSIZE + *boff) ){  
    inode->fSize = (*bindex) * BLOCKSIZE + *boff;
  }
  write_inode(inode, f->inum);  // write update of inode back to inode block
  free(bindex);
  free(boff);
}

// read from file of fd file descriptor of certain size
void read_from_file(int fd, int size, char *data)
{
  FileDescriptor *f;
  char msg[255];
  if(FTable.bitmap[fd] == 0){ 
    sprintf(msg, "File decriptor %d is not associated with any opened file!!", fd);
    //display_msg(msg, out);
    printf("%s\n", msg);
    fflush(stdout);
    return;
  }
  f = &FTable.fdArray[fd];  // get pointer of file descriptor associated with fd
  if(f->mode != 1 && f->mode !=3){
    sprintf(msg, "File is not open for read!!");
    //display_msg(msg, out);
    printf("%s\n", msg);
    fflush(stdout);
    return;
  }
  Inode *inode;
  inode = f->inode;  // inode of file 
  // get current block index and position in the block
  int *bindex, *boff, length, tsize;
  bindex = malloc(sizeof(int));
  boff = malloc(sizeof(int));
  get_block_index_and_offset_for_csp(f->csp, bindex, boff);  // gets value of block index and block offset from current seek pointer
  if(inode->fSize < ((*bindex) * BLOCKSIZE + *boff + size)){  // if size exceeds beyond end of file
    sprintf(msg, "Size to be read exceeds file limit!!");
    //display_msg(msg, stdout);
    printf("%s\n", msg);
    fflush(stdout);
    free(bindex);
    free(boff);
    return;
  }
    
  // start reading data from file
  if(*boff == BLOCKSIZE){
    // increase bindex
    *bindex = *bindex + 1;
    *boff = 0;   // set offset to 0 in next block
  }
  // read data in chunks
  char chunk[BLOCKSIZE];
  //check if data size + boff is less than block size
  if(*boff + size <= BLOCKSIZE){
    // read data from the block
    read_fcontent(inode->pointer[*bindex], *boff, size, chunk);
    strncat(data, chunk, size);
    *boff += size;
  }
  else{
    // get length of data fitting in the block indexed by *bindex
    length = BLOCKSIZE - *boff;   
    tsize = size;       // temp variable manipulating size at data gets read
    // read chunk of length from the block
    read_fcontent(inode->pointer[*bindex], *boff, length, chunk);
    *boff += length;  // offset in block moved up by length
    tsize  -= length;  // size to be read now decreased by length
    while(tsize > 0){  // if length > 0 read
      *boff = 0;      
      *bindex++;  // increase to next pointer inode
      if(tsize < BLOCKSIZE) 
	length = tsize;
      else
	length = BLOCKSIZE;
      // read chunk from the block
      read_fcontent(inode->pointer[*bindex], *boff, length, chunk);
      strncat(data, chunk, length); // add read chunk of data to data buffer
      *boff += length;    // position in the block moved up by length
      tsize -= length;    // remaining size of data to be read decreased by length  
    }
  }
  // update current seek pointer in fd 
  f->csp = (*bindex) * BLOCKSIZE + *boff; 
  free(bindex);
  free(boff);
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


// writes directory entry in file
void write_dentry(Inode *inode, DEntry *de)
{
  int dblk, off, pindex, size, status;
  char msg[255];
  pindex = get_pindex(inode);
  if(pindex < IPOINTS){  // direct blocks
    if(inode->pointer[pindex] == -1){
      //allocate a block to that pointer
      status = allocate_block(inode, pindex, msg);
      if(status == 0){    // not allocated
	//display_msg(msg, stdout);
	printf("%s\n", msg);
	fflush(stdout);
	return;
      }      
    }
    off = inode->fSize - (pindex * BLOCKSIZE);    // bytes of data already written to block pointed
    if(off + sizeof(DEntry) > BLOCKSIZE){
      pindex++; off = 0;
      if(pindex < IPOINTS){
	// allocate block to inode->pointer[pindex]
	status = allocate_block(inode, pindex, msg);
	if(status == 0){    // not allocated
	  //display_msg(msg, stdout);
	  printf("%s\n", msg);
	  fflush(stdout);
	  return;
	}  
	dblk = inode->pointer[pindex];
	write_dcontent(inode, dblk, off, de);
      }
      else{ // limit exceeds our implementation size for file
	printf("Out of limit for a file!!\n");	
      }
    }
    else{ // write content with offset off to block indexed by pindex data block
      dblk = inode->pointer[pindex];
      write_dcontent(inode, dblk, off, de);
    }
  }
  
}

void copy_dentry(DEntry *src, DEntry *dst)
{
  dst->inum = src->inum;
  strcpy(dst->name, src->name);
  dst->strlen = src->strlen;
  dst->reclen = src->reclen;
}

void free_directory(Directory *d)
{
  Directory *c, *t;
  c = d; int i = 0;
    
  while(c != 0){
    t = c;
    c = c->ndir;
    free(t);
  }   
}


// returns status of search entry by name of entry and also returns pointer to entry if found
int get_entry_by_name(int pinum, char *name, DEntry * entry)
{
  int i;
  Inode inode;
  read_inode(&inode, pinum);
  int dblk, off, pindex, size;
  if(inode.fSize == 0) return 0;   // no record found status
  
  pindex = get_pindex(&inode);   // index of pointer that has been used up to
  if(pindex == 0 && inode.pointer[pindex] == -1) return 0;
  
  Directory *d, *c;
  int read;
  bool found = 0;
  d = malloc(sizeof(Directory));
  c = d;
  if(pindex < IPOINTS){
    for(i = 0; i < pindex; i++){      
      // read one by one for all possible blocks and if found break
      fseek(FSP, (inode.pointer[i]+SBlock.first_dblock)*BLOCKSIZE, SEEK_SET);
      read = 0;
      if( i == 0){      // if for first block and for first read, no need to malloc; already malloced
	fread(&c->entry, sizeof(DEntry), 1, FSP);   // read DEntry
	read = read + sizeof(DEntry);
	if(c->entry.inum !=0 && strcmp(c->entry.name, name)==0){  // if name matches
	  found = 1;
	  break;
	}
      }
      while((read+sizeof(DEntry)) <= BLOCKSIZE){  // read max up to BLOCKSIZE
	c->ndir = malloc(sizeof(Directory));  //
	c = c->ndir;
	c->ndir = 0;
	fread(&c->entry, sizeof(DEntry), 1, FSP);   // read DEntry
	read = read + sizeof(DEntry);
	if(c->entry.inum !=0 && strcmp(c->entry.name, name)==0){  // if name matches
	  found = 1;
	  break;
	}
      }
      if(found) break;  // found then break the loop      
    }
    //if not found read offset too
    if(!found){
      off = inode.fSize - (pindex * (BLOCKSIZE/(int)sizeof(DEntry))*sizeof(DEntry));   // size of data on block indexed by pindex
      // read one by one for all possible blocks and if found break
      fseek(FSP, (inode.pointer[pindex]+SBlock.first_dblock)*BLOCKSIZE, SEEK_SET);
      read = 0;
      if( pindex == 0){      // if for first block and for first read, no need to malloc; already malloced
	fread(&c->entry, sizeof(DEntry), 1, FSP);   // read DEntry
	read = read + sizeof(DEntry);
	if(c->entry.inum !=0 && strcmp(c->entry.name, name)==0){  // if name matches
	  found = 1;	  
	}
      }
      while(read < off && !found){  // read max up to off
	c->ndir = malloc(sizeof(Directory));  // malloc next node of list; 
	c = c->ndir;                          // move to next node
	c->ndir = 0;
	fread(&c->entry, sizeof(DEntry), 1, FSP);   // read DEntry
	read = read + sizeof(DEntry);
	if(c->entry.inum !=0 && strcmp(c->entry.name, name)==0){  // if name matches
	  found = 1;
	  break;
	}
      }
      if(!found){  // not found
	free_directory(d);
	return 0;
      }
    }    
    // fill entry
    copy_dentry(&c->entry, entry);
    free_directory(d);
    return 1;
           
  }
  else{
    // free malloced space
    free_directory(d);
    return 0;
  }
}


// removes directory entry with inum inode number in parent directory of pinum inode number
void remove_directory(int pinum, int inum)
{  
  // free inode of the directory and block allocated for it
  Inode inode;
  read_inode(&inode, inum);
  free_inode(&inode, inum);

  // find last entry in parent directory
  int pindex, boff, entry_per_block;
  DEntry dl;
  Inode pinode;
  read_inode(&pinode, pinum);   //
  pindex = get_pindex(&pinode);  // index upto which inode pointers have been used
  entry_per_block = BLOCKSIZE/sizeof(DEntry);
  boff   = (pinode.fSize - pindex * entry_per_block * sizeof(DEntry)) - sizeof(DEntry);  // offset to read last block
  fseek(FSP, (pinode.pointer[pindex]+SBlock.first_dblock)*BLOCKSIZE + boff, SEEK_SET);  // move pointer to just infront of last entry
  fread(&dl, sizeof(DEntry), 1, FSP);  // read last entry
  // if that's only one entry in that data block free that data block
  if(boff == 0){
  // free data block
    free_block(&pinode, pindex);
  }
  
  // replace to be removed entry by last entry if it is not last entry; else it is done
  Directory *d, *c;
  int read, idx, i, off;
  bool found = 0;
    
  if(dl.inum != inum){ // if last entry inum not equals to removing entry inum, find entry to be removed
   d = malloc(sizeof(Directory));
   c = d;
   if(pindex < IPOINTS){
     for(i = 0; i < pindex; i++){      
       // read one by one for all possible blocks and if found break
       fseek(FSP, (pinode.pointer[i]+SBlock.first_dblock)*BLOCKSIZE, SEEK_SET);
       read = 0;
       while(read < BLOCKSIZE){  // read max up to BLOCKSIZE
	 fread(&c->entry, sizeof(DEntry), 1, FSP);   // read DEntry
	 read = read + sizeof(DEntry);
	 c->ndir = 0;
	 if(c->entry.inum !=0 && c->entry.inum == inum){  // if inum matches
	   idx = i;
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
       off = pinode.fSize - (pindex * BLOCKSIZE);  // size of data on block indexed by pindex
	// read one by one for all possible blocks and if found break
       fseek(FSP, (pinode.pointer[pindex]+SBlock.first_dblock)*BLOCKSIZE, SEEK_SET);
       read = 0;
       while(read < off){  // read max up to off
	 fread(&c->entry, sizeof(DEntry), 1, FSP);   // read DEntry
	 read = read + sizeof(DEntry);
	 c->ndir = 0;	  
	 if(c->entry.inum !=0 && c->entry.inum == inum){  // if inum matches
	   found = 1;
	   idx = pindex;
	   break;
	  }
	 else{ //if not found allocate memory for ndir 
	   c->ndir = malloc(sizeof(Directory));  //
	   c = c->ndir;
	 }
       }      
     }
     if(found){
       // replace that entry
       fseek(FSP, (pinode.pointer[idx]+SBlock.first_dblock)*BLOCKSIZE + read-sizeof(DEntry), SEEK_SET);
       fwrite(&dl, sizeof(DEntry), 1, FSP);       
     }        
   }   
   free_directory(d);
  }
  pinode.fSize -= sizeof(DEntry);      // size of parent directory inode decreases by sizeof DEntry
  write_inode(&pinode, pinum);
  
}

// creates file in parent directory with pinum inumber and returns inumber for the file
int create_file(char *name, int pinum)
{
  int inum;
  inum =  get_free_inum();
  if(inum == 0){    // no. inode is free
    //display_msg("Couldn't create file. No inode is free!!", stdout);
    printf("Couldn't create file. No inode is free!!\n");
    return;
  }
  // create entry object for it ; 
  DEntry d1;
  d1.inum = inum;   // inum of newly created file
  strcpy(d1.name, name);
  d1.strlen = strlen(d1.name);  
    
  Inode inode;
  time_t t;
  int status;
  char msg[255];
  init_inode(&inode);  // sets initial values for inode attributes
  inode.flags = 2;  // type file
  inode.fSize = 0;  // initialize size to 0  
  inode.l_count = 1;
  strncpy(inode.c_date, ctime(&t), 28);
  status = allocate_block(&inode, 0, msg);
  if(status == 0){    // not allocated
    //display_msg(msg, stdout);
    printf("%s\n", msg);
    return;
  }  
  write_inode(&inode, inum);       // write inode to place holder for that inode
  Inode pinode;
  read_inode(&pinode, pinum);    // read parent inode
  write_dentry(&pinode, &d1);    // add entry to parent directory
  write_inode(&pinode, pinum);   // write parent inode to file system
  return inum;
}

// closes the file associated with fd
void close_file(int fd)
{
  FileDescriptor *f;
  f = &FTable.fdArray[fd];  // get pointer of file descriptor associated with fd
  FTable.bitmap[fd] = 0;    // bitmap for fd index is set to 0; indicates the slot is open
  free(f->inode);            // free memory allocated for inode in file descriptor
  f->mode = 0;
  f->inum = 0;
  f->csp  = 0;
  FTable.count--;           // decrease no. of opened files in the file table
}

// opens a file with name inside parent of inum pinum; sets value of fd if successful
void open_file(char *name, int pinum, int mode, int *fd, char *msg)
{
  DEntry entry;
  bool found = 1;
  int i, j, inum;
  *fd = -1;   // initialize fd to -1  ; can be used as status check in calling function
  i = get_entry_by_name(pinum, name, &entry); // i=1 if file is found and gets file info in parent directory as entry;
  if(i == 0){
    found = 0;   
  }

  Inode inode;
  if(found){
    read_inode(&inode, entry.inum);  // read inode of file
    if(inode.flags != 2){           // if flags not 2, then it is not a file, may be a directory
      sprintf(msg, "%s is not a file!!", name);
      return;
    }
    inum = entry.inum;
  }
  else{             // not found
    if(mode == 2){ // opened in write mode, then create file with that name with size 0
      // create a file
      inum = create_file(name, pinum);
      read_inode(&inode, inum);  // read inode of file          
    }
    else{
      sprintf(msg, "File %s not found!!\n", name);
      return;
    }
  }  
   
  // if found then enter it into file table
  for(j = 0, i = FTSIZE; j < FTSIZE; j++){  // find open slot in file table
    if(FTable.bitmap[j] == 0){
      i = j; 
      break;
    }
  }
  if( i == FTSIZE){
    sprintf(msg, "Too many files opened!! Slot not available!!\n");
    return;
  }
  FTable.bitmap[i] = 1;  // slot is occupied
  FTable.fdArray[i].inode = malloc(sizeof(Inode));
  copy_inode(&inode, FTable.fdArray[i].inode);
  FTable.fdArray[i].inum = inum;
  FTable.fdArray[i].csp = 0;
  FTable.fdArray[i].mode = mode;  
  FTable.count++;
  *fd = i;  // sets f
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
  time_t t;
  init_inode(&inode);
  inode.flags = 1;  // type directory
  inode.fSize = 0;  // initialize size to 0
  inode.l_count = 1;
  strncpy(inode.c_date, ctime(&t), 28);
  write_dentry(&inode, &d1);     // write directory content to data block made available to inode and update inode   
  write_dentry(&inode, &d2);     // write directory content to data block made available to inode and update inode 
  
  write_inode(&inode, 1);       // write inode to place holder for that 1st inode
  //fseek(FSP, (0+SBlock.first_dblock)*BLOCKSIZE ,SEEK_SET);
  return 0;      // success
}

// creates directory with give name inside given parent inum
void create_directory(char *name, int pinum)
{
  int inum;
  inum =  get_free_inum();
  if(inum == 0){  // no inode is free
    //display_msg("Couldn't create directory. No inode is free.", stdout);
    printf("Couldn't create directory. No inode is free.\n");
    fflush(stdout);
    return;
  }
  // create inode object for it ; 
  DEntry d1, d2;
  d1.inum = inum;   // inum of newly created directory
  strcpy(d1.name, ".");
  d1.strlen = strlen(d1.name);
  d2.inum = pinum;   // parent directory's inum
  strcpy(d2.name, "..");
  d2.strlen = strlen(d2.name); 

  Inode inode;
  time_t t;
  init_inode(&inode);
  inode.flags = 1;  // type directory
  inode.fSize = 0;  // initialize size to 0
  inode.l_count = 1;
  strncpy(inode.c_date, ctime(&t), 28);

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


// format file system
void format_fs()
{
  int i, j, h;
  // initialize superblock
  SBlock.size         = (FSIZE*1024*1024)/BLOCKSIZE;   // total blocks in file system
  SBlock.iSize        = INODEBLOCKS;              // no. of inode blocks 
  SBlock.first_inode  = 1;     // inode starts at block 1; constant throughout
  SBlock.first_dblock = SBlock.iSize + 1;  // data blocks starts after iSize+1 blocks; constant throughout (no. of Superblock + no. of inode blocks)
  SBlock.next_fdblock = 0; // every data block is free at first; using relative index
  SBlock.next_finode  = 1;              // inode starts at 1 not 0;
  SBlock.root_inum    = 1;    // inode 1 is assigned for root directory  

  // write all inode blocks with each inode flags set to 0
  InodeBlock iblock[INODEBLOCKS];
  for(i = 0; i < INODEBLOCKS; i++){     // go through all inode blocks
    for(j = 0; j < (int)(BLOCKSIZE)/INODESIZE; j++){         // go through all inodes inside each inode block
      init_inode(&iblock[i].node[j]);                 // initialize inode
      // for(h = 0; h < 13; h++)
      //	iblock[i].node[j].pointer[h] = -1;
    }
    write_inodeblock(i+1, &iblock[i]);   // write inode block
  }
 // link freelist pointers in data blocks
  for(i = 0; i < (SBlock.size-(INODEBLOCKS+1) - 1); i++){  // go through all data blocks execept last data block (because no where to point)
    // write freelist pointers in each data block starting from SBlock.first_dblock
    write_integer(SBlock.first_dblock+i, i+1);
  }
  write_integer(SBlock.first_dblock+i, -1);  // points to -1 for the last block
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




/***************************************** execute command methods *************************************/


// formats disk space
void create_fs()
{
  if(ARGC>1){
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


// executes mkdir command
void make_dir(){
  char msg[255];
  if(ARGC ^ 2){
    sprintf(msg,"Error!! Number of arguments mismatch for mkdir command!!"); 
    //display_msg(msg, stdout);
    printf("%s\n", msg);
    return;
  }
  
  int pinum, i;
  char name[MAXLENGTH];
  DEntry d1;
  pinum = FSContext.cwd;     // inum of current working directory
  strcpy(name, MY_ARGV[1]);
  
  i = get_entry_by_name(pinum, name, &d1);   // check if entry with same name exists in parent directory
  if(i == 1){  // if exists
    sprintf(msg,"Error!! Entry with same name exists in the directory!!");
    //display_msg(msg, stdout);
    printf("%s\n", msg);
    return;
  }
  create_directory(name, pinum); // create directory if already not exists  
}

void get_directory_entries(Inode *inode, Directory *d)
{
  Directory *c;
  int i, read, pindex, off;
  pindex = get_pindex(inode);   // index of pointer that has been used up to
  c = d;
  c->ndir = 0;
      
  if(pindex < IPOINTS){          // index used should be less than no. of pointers in an inode
    for(i = 0; i < pindex; i++){      
      // read one by one for all possible blocks
      fseek(FSP, (inode->pointer[i]+SBlock.first_dblock)*BLOCKSIZE, SEEK_SET);
      read = 0;
      if( i == 0){      // if for first block and for first read, no need to malloc; already malloced
	fread(&c->entry, sizeof(DEntry), 1, FSP);   // read DEntry
	read = read + sizeof(DEntry);
      }
      while((read+sizeof(DEntry)) <= BLOCKSIZE){  // read max up to BLOCKSIZE; don't read if size of DEntry read would cross BLOCKSIZE
	c->ndir = malloc(sizeof(Directory));  //
	c = c->ndir;
	c->ndir = 0;
	fread(&c->entry, sizeof(DEntry), 1, FSP);   // read DEntry
	read = read + sizeof(DEntry);	
      }       
    }
    
    off = inode->fSize - (pindex * (BLOCKSIZE/(int)sizeof(DEntry))*sizeof(DEntry));  // size of data on block indexed by pindex
    // read one by one for all possible blocks
    
    fseek(FSP, (inode->pointer[pindex]+SBlock.first_dblock)*BLOCKSIZE, SEEK_SET);
    read = 0;
    
    if(pindex == 0 && (read+sizeof(DEntry)) <= off){   // if first block, no need to malloc; already malloced
      fread(&c->entry, sizeof(DEntry), 1, FSP);   // read DEntry
      read = read + sizeof(DEntry);
    }
    while((read+sizeof(DEntry)) <= off){  // read max up to off
      c->ndir = malloc(sizeof(Directory));
      c = c->ndir;
      c->ndir = 0; 
      fread(&c->entry, sizeof(DEntry), 1, FSP);   // read DEntry
      read = read + sizeof(DEntry);          	
    }     
  }

}

// lists all entries within a current working directory
void ls()
{
  char msg[255];
  if(ARGC > 1){
    strcpy(msg, "Error!! Higher number of arguments than expected!!\n");
    //display_msg(msg, out);
    printf("%s\n", msg);
    return;
  }
  
  // display list of items for current working directory
  int i, inum;
  Inode inode;
  inum = FSContext.cwd;      // get inum of current working directory
  read_inode(&inode, inum);
  int dblk, off, pindex, size;
  if(inode.fSize <= 2*sizeof(DEntry)) return;   // no entries are there except itself and parent  
 
  //get all linked list of directory entries
  Directory *d, *c;
  d = malloc(sizeof(Directory));
  get_directory_entries(&inode, d);
  // display as list all entries except first two which are self and parent directory
  c = d;
  c = c->ndir;        // skip first two entries
  c = c->ndir;
  while(c != 0){
    //fprintf(out, "%s\t", c->entry.name);
    printf("%s\t", c->entry.name);
    c = c->ndir;   
  }  
  //fprintf(out, "\n");
  printf("\n");
  free_directory(d);
}

// lists all entries within a current working directory and size if it is a file
void tree()
{
  char msg[255];
  if(ARGC > 1){
    strcpy(msg, "Error!! Higher number of arguments than expected!!\n");
    //display_msg(msg, out);
    printf("%s\n", msg);
    fflush(stdout);
    return;
  }
  
  // display list of items for current working directory
  int i, inum;
  Inode inode;
  inum = FSContext.cwd;      // get inum of current working directory
  read_inode(&inode, inum);
  int dblk, off, pindex, size;
  if(inode.fSize <= 2*sizeof(DEntry)) return;   // no entries are there except itself and parent  
 
  //get all linked list of directory entries
  Directory *d, *c;
  d = malloc(sizeof(Directory));
  get_directory_entries(&inode, d);
  // display as list all entries except first two which are self and parent directory
  c = d;
  c = c->ndir;        // skip first two entries
  c = c->ndir;
  while(c != 0){
    //fprintf(out, "%s\t", c->entry.name);
    printf("%s", c->entry.name);
    read_inode(&inode, c->entry.inum);
    if(inode.flags == 2 || inode.flags == 3){
      printf("\t\t%d", inode.fSize);
    }
    printf("\n");
    c = c->ndir;   
  }  
  //fprintf(out, "\n");
  printf("\n");
  free_directory(d);
}

// executes stat command
void stat()
{
  char msg[255];
  if(ARGC ^ 2){
    strcpy(msg, "Error!! Number of arguments expected is 2!!\n");
    //display_msg(msg, out);
    printf("%s\n", msg);
    fflush(stdout);
    return;
  }
  
  // display list of items for current working directory
  int i, pinum, inum, blocks;
  char name[MAXLENGTH];
  Inode inode;
  pinum = FSContext.cwd;      // get inum of current working directory
  DEntry d;
  // check if entry is present in current working directory
  i = get_entry_by_name(pinum, MY_ARGV[1], &d);
  if(i == 0){
    printf("No file or directory of name %s found!!\n", MY_ARGV[1]);
    return;
  }
  // ready information to display for statistics
  inum = d.inum;
  strcpy(name, d.name);
  read_inode(&inode, inum);     // get inode for the file
  blocks = get_pindex(&inode) + 1;  // read no. of pointers that been allocated for that inode
  // display stat
  printf("file: %s\n", name);
  printf("size: %d\t", inode.fSize);
  printf("blocks: %d\t", blocks);
  if(inode.flags == 1) // directory
    printf("directory\n");
  else if(inode.flags == 2) // regular file
    printf("regular file\n");
  printf("Inode : %d\t", inum);
  printf("Links: %d\n", inode.l_count);
  printf("Birth: %s\n", inode.c_date);
}


// executes rmdir command
void rm_dir()
{
  char msg[255];
  if(ARGC ^ 2){
    strcpy(msg, "Error!! Number of arguments expected is 2!!\n");
    //display_msg(msg, stdout);
    printf("%s\n", msg);
    fflush(stdout);
    return;
  }
  // get directory entry by name
  int pinum, inum, i;
  char name[MAXLENGTH];
  DEntry d1;
  pinum = FSContext.cwd;     // inum of current working directory
  strcpy(name, MY_ARGV[1]);
  i = get_entry_by_name(pinum, name, &d1);  // check if exists
  if(i !=1){  // if directory not present
    sprintf(msg, "Error!! No directory named %s found!!\n", name);
    //display_msg(msg, stdout);
    printf("%s\n", msg);
    fflush(stdout);
    return;
  }
 
  Inode inode;
  read_inode(&inode, d1.inum);
  // check if entry is file or directory
  if(inode.flags != 1){
    printf("Error!! %s is not a directory!!\n", d1.name);
    return;
  }
  // check directory empty or not  
  if(inode.fSize > (2 * sizeof(DEntry))){
    sprintf(msg, "Error!! Directory is not empty");
    //display_msg(msg, stdout);
    printf("%s\n", msg);
    fflush(stdout);
    return;
  }
  // remove directory
  remove_directory(pinum, d1.inum);
}

// executes command open
int openf()
{
  char msg[255];
  if(ARGC ^ 3){
    sprintf(msg,"Error!! Number of arguments expected mismatch!! Expected open filename mode\n");
    //display_msg(msg, stdout);
    printf("%s\n", msg);
    fflush(stdout);
    return;
  }
  int pinum, i, mode, fd;
  char name[MAXLENGTH];
  DEntry d1;
  pinum = FSContext.cwd;     // inum of current working directory
  strcpy(name, MY_ARGV[1]);

  // check mode of open
  if(strcmp(MY_ARGV[2], "r") == 0){
    mode = 1;
  }
  else if(strcmp(MY_ARGV[2], "w") == 0){
    mode = 2;
  }
  else if(strcmp(MY_ARGV[2], "w") == 0){
    mode = 3;
  }
  else{
    sprintf(msg, "Error!! Unknown mode!!");
    //display_msg(msg, stdout);
    printf("%s\n", msg);
    fflush(stdout);
    return 0;
  }

  i = get_entry_by_name(pinum, name, &d1);   // check if entry with same name exists in parent directory
  open_file(name, pinum, mode, &fd, msg);
  if(fd == -1){
    // display_msg(msg, stdout);
    printf("%s\n", msg);
    fflush(stdout);
    return 0;
  } 
  else{
    sprintf(msg,"SUCCESS, fd = %d\n", fd);
    //display_msg(msg, stdout);
    printf("%s\n", msg);
    fflush(stdout);
  }
  return fd;
}

// sets seek pointer of a file given by fd to specified offset
void seek()
{
  char msg[255];
  if(ARGC ^ 3){
    strcpy(msg, "Number of arguments expected for seek is 3!!");
    //display_msg(msg, stdout);
    printf("%s\n", msg);
    fflush(stdout);
  }
  int fd = atoi(MY_ARGV[1]);  // read fd and check if it is valid integer
  if(fd == 0 && strcmp(MY_ARGV[1], "0") != 0){
    strcpy(msg, "Invalid number input for file descriptor!!");
    // display_msg(msg, stdout); 
    printf("%s\n", msg);
    fflush(stdout);
    return;
  }
  int off = atoi(MY_ARGV[2]);  // read fd and check if it is valid integer
  if(off == 0 && strcmp(MY_ARGV[2], "0") != 0){
    strcpy(msg, "Invalid input for offset!!");
    //display_msg(msg, stdout); 
    printf("%s\n", msg);
    fflush(stdout);
    return;
  }
  // check if fd input is of opened file
  if(FTable.bitmap[fd] != 1){
    strcpy(msg, "File specified is not opened!!");
    //display_msg(msg, stdout);
    printf("%s\n", msg);
    fflush(stdout);
  }
  seek_pointer(fd, off);
}

// method executing close command
void close_f()
{
  char msg[255];
  if(ARGC ^ 2){
    sprintf(msg,"Error!! Number of arguments expected 2!!");
    //display_msg(msg, out);
    printf("%s\n", msg);
    fflush(stdout);
    return;
  }
  int fd = atoi(MY_ARGV[1]);  // read fd and check if it is valid integer
  if(fd == 0 && strcmp(MY_ARGV[1], "0") != 0){
    strcpy(msg, "Invalid number input for file descriptor!!");
    //display_msg(msg, out); 
    printf("%s\n", msg);
    fflush(stdout);
    return;
  }  
  // check if fd input is of opened file
  if(FTable.bitmap[fd] != 1){
    strcpy(msg, "File specified for close is not opened!!");
    //display_msg(msg, out);
    printf("%s\n", msg);
    fflush(stdout);
    return;
  }
  
  close_file(fd);
}

// frees string array of size n
void free_path(char **path, int n)
{
  int index;
  for(index = 0; index <n; index++) {
    free(path[index]);
  }
}

// parses path
void parse_path(char *tmp_argv, char **path, int *npath)
{
  char *foo = tmp_argv;
  int index = 0;
  char ret[150];
  bzero(ret, 150);
   if(*foo == '/'){ // if '/' it appears first then it is absolute path from root    
    path[index] = (char *)malloc(sizeof(char)* 1);  
    //strncat(path[index], foo, 1);
    //strncat(path[index], "\0", 1);
    strcpy(path[index], "/");
    index++;
    foo++;   
  }
  while(*foo != '\0') {
    if(index == 10)
      break;
    
    if(*foo == '/') {  // '/' signifies end of one directory name   
     if(*(foo+1) == '/' || *(foo+1) == '\0') {foo++; continue;}  // if next character is also space or null then just move forward
      path[index] = (char *)malloc(sizeof(char) * strlen(ret));
      //strncpy(path[index], ret, strlen(ret));
      //strncat(path[index], "\0", 1);
      strcpy(path[index], ret);
      bzero(ret, 150);
      index++;
    } else {
      strncat(ret, foo, 1);     // concatenates one character at a time to ret string
    }
    foo++;
    /*printf("foo is %c\n", *foo);*/
  }
  path[index] = (char *)malloc(sizeof(char) * strlen(ret));
  strcpy(path[index], ret);
  // strncpy(path[index], ret, strlen(ret));
  // strncat(path[index], "\0", 1);
  *npath = index + 1;   // no. of arguments
}

// executes cd command
void cd_path()
{
  char msg[255];
  if(ARGC ^ 2){
    sprintf(msg,"Error!! Number of arguments expected mismatch!!");
    //display_msg(msg, out);
    printf("%s\n", msg);
    fflush(stdout);
    return;
  }
  char *path[100];
  int i, npath, s, start, inum;
  DEntry *d;
  parse_path(MY_ARGV[1], path, &npath);  // path is parsed directory string array; npath is no. of directory in path
  if(npath == 1 && strcmp(path[0], ".") == 0) return; // itself
  if(npath == 1 && strcmp(path[0], "..") == 0){ // parent directory
    if(FSContext.cwd == SBlock.root_inum){
      free_path(path, npath);
      return; // if current directory is root
    }
    d = malloc(sizeof(DEntry));
    s = get_entry_by_name(FSContext.cwd, path[0], d);
    FSContext.cwd = d->inum;
    free(d);
    free_path(path, npath);
    return;
  }
  if(strcmp(path[0], "/") == 0){ // if starts from '/' absolute path search
    inum = SBlock.root_inum; start = 1;
  }  
  else{
    inum = FSContext.cwd;   // else relative to current directory
    start = 0;
  }
  d = malloc(sizeof(DEntry));    
  for(i = start; i < npath; i++){
    s = get_entry_by_name(inum, path[i], d);
    if(!s){
      sprintf(msg,"Error!! Path not found!!");
      //display_msg(msg, stdout);
      printf("%s\n", msg);
      fflush(stdout);
      free_path(path, npath);
      return;
    }
    inum = d->inum;   // if found then replace inum by entry's inum       
  }
  FSContext.cwd = inum;     // current working directory set to inum
  sprintf(msg, "Directory changed to %s", d->name);
  printf("%s\n", msg);
  fflush(stdout);
  free_path(path, npath);
  free(d);
}


// executes write command
void writef()
{
  char msg[255];
  if(ARGC ^ 3){
    sprintf(msg,"Error!! Number of arguments expected 3!! 3rd argument must be string enclosed in \"\"");
    //display_msg(msg, out);
    printf("%s\n", msg);
    fflush(stdout);
    return;
  }
  int fd = atoi(MY_ARGV[1]);  // read fd and check if it is valid integer
  if(fd == 0 && strcmp(MY_ARGV[1], "0") != 0){
    strcpy(msg, "Invalid number input for file descriptor!!");
    //display_msg(msg, out); 
    printf("%s\n", msg);
    fflush(stdout);
    return;
  }
  // check if fd input is of opened file
  if(FTable.bitmap[fd] != 1){
    strcpy(msg, "File specified is not opened!!");
    //display_msg(msg, out);
    printf("%s\n", msg);
    fflush(stdout);
    return;
  }
  // write data
  char *data = malloc(sizeof(char)*strlen(MY_ARGV[2]));
  strcpy(data, "");
  strncpy(data, MY_ARGV[2], strlen(MY_ARGV[2]));
  write_to_file(fd, data);
  free(data);
}

// executes read command
void readf()
{
  char msg[255];
  if(ARGC ^ 3){
    sprintf(msg,"Error!! Number of arguments expected 3!!");
    //display_msg(msg, out);
    printf("%s\n", msg);
    fflush(stdout);
    return;
  }
  int fd = atoi(MY_ARGV[1]);  // read fd and check if it is valid integer
  if(fd == 0 && strcmp(MY_ARGV[1], "0") != 0){
    strcpy(msg, "Invalid number input for file descriptor!!");
    //display_msg(msg, out); 
    printf("%s\n", msg);
    fflush(stdout);
    return;
  } 
  int size = atoi(MY_ARGV[2]);  // read size and check if it is valid integer
  if(size == 0 && strcmp(MY_ARGV[2], "0") != 0){
    strcpy(msg, "Invalid input for offset!!");
    //display_msg(msg, out); 
    printf("%s\n", msg);
    fflush(stdout);
    return;
  }
  // check if fd input is of opened file
  if(FTable.bitmap[fd] != 1){
    strcpy(msg, "File specified is not opened!!");
    //display_msg(msg, out);
    printf("%s\n", msg);
    fflush(stdout);
    return;
  }
  if(FTable.fdArray[fd].mode !=1 && FTable.fdArray[fd].mode !=3){
    strcpy(msg, "File not opened for read!!");
    //display_msg(msg, out);
    printf("%s\n", msg);
    fflush(stdout);
    return;
  }
  // read data
  char *data = malloc(sizeof(char)*size);
  // initialize data
  strcpy(data, "");
  read_from_file(fd, size, data);
  if(strlen(data) > 0){
    //display_msg(data, out);
    printf("%s\n", data);
    fflush(stdout);
  }
  free(data); 
}


/* This function starts the server to allow for Remote Procedure Calls, as defined in the
   assignment description */
void start_server(){
    int portnum;	/* Port number */
    int sock, fd; 	/* Ints to refer to sock and file descriptor, once initialized */
    int n;		/* Temporary variable */
    socklen_t len;	/* Client address length, as defined in the library */
    struct sockaddr_in server, client; /* Server and client addresses, using the library */

    printf("Starting server mode...\n");
    printf("Enter port number. Ports 5555 and 5500 are recommended.\n");
    
    scanf("%d", &portnum);
	printf("Review the manual for information about how to connect from client.\n");
    printf("To exit server mode, end this process and type 'End Server' in Client shell.\n");
    printf("Now redirecting input/output to Client.\n");
    
    /* Initialize sock, using method definted in Linux Howtos Guide */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0){
        perror("Error Starting Server");
        exit(1);
    }
    
    /* Clear out server address */
    bzero((char *) &server, sizeof(server));
    
    /* Set proper server struct attributes, according to methods outlined in Linux Howtos Guide */
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(portnum);

    /* Bind server to port and address, according to Linux Howtos Guide method */
    if(bind(sock, (struct sockaddr *) &server, sizeof(server)) < 0){
        perror("Error Binding Server");
        exit(1);
    }

    /* Start listening to the socket to wait for a connection */
    listen(sock, 5); /* Set to 5 since it is max size permitted */
    len = sizeof(client);

    /* Accept calling client and set file descriptor as fd */
    fd = accept(sock, (struct sockaddr *) &client, &len);
    if (fd < 0){
        perror("Error Accepting Client");
        exit(1);
    }
    setbuf(stdout, NULL); /* Keeps from having to call fflush() every time printf() is called */
    dup2(fd, 0); /* Read from Client, instead of Standard Input */
    dup2(fd, 1); /* Write to Client, instead of Standard Output */
    dup2(fd, 2); /* Write to Client, instead of Standard Error */	
    
    printf("Connected to Server. Type 'End Server' to exit.\n");    
return;
}

void end_server(){
    exit(1);
}

void cat(){
	char name[MAXLENGTH];
	char msg[255];
	int pinum, i, mode, fd, size, found;
	DEntry entry;
	Inode inode;

	if (ARGC != 2){
		printf("Error. Usage: $ cat filename\n");
		return;
	}
    
	pinum = FSContext.cwd; /* inum of current working directory */
	strcpy(name, MY_ARGV[1]);
	mode = 1;
	i = get_entry_by_name(pinum, name, &entry);

	if (i == 0){
		found = 0;
	}
	if (found){
		read_inode(&inode, entry.inum);
		if (inode.flags != 2){
			printf("Error. %s is not a file\n", name);
			return;
		}

		open_file(name, pinum, mode, &fd, msg);
		size = inode.fSize;
	} else {
		printf("Error. %s file not found\n", name);
	} 

	if (fd == -1){
		printf("Error. %s is not a file\n", name);
		return;
	}             
	
	sprintf(MY_ARGV[1], "%d", fd);
    
	printf("%d\n", size);
	MY_ARGV[2] = (char *)malloc(sizeof(char) * size);
	sprintf(MY_ARGV[2], "%d", size);
	ARGC = 3;
	readf(); 

	ARGC = 2;
	close_f();
	return;
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
	unsigned long readspace = 0;    // signifies if space character should be read as part of argument
	int index = 0;
	char ret[200];
	bzero(ret, 200);
	while(*foo != '\0') {
		if(index == 10)
			break;

		if(*foo == ' ' && !readspace) {  // space signifies end of one argument
			if(strlen(ret) == 0){foo++; continue;}
			if(*(foo+1) == ' ' || *(foo+1) == '\0') {foo++; continue;}  // if next character is also space or null then just move forward
			if(MY_ARGV[index] == NULL)
				MY_ARGV[index] = (char *)malloc(sizeof(char) * strlen(ret));
			else {
				bzero(MY_ARGV[index], strlen(MY_ARGV[index]));
			}
			//MY_ARGV[index] = (char *)malloc(sizeof(char) * strlen(ret) + 1);
			// strncpy(MY_ARGV[index], ret, strlen(ret));
			strcpy(MY_ARGV[index], ret);
			//strncat(MY_ARGV[index], "\0", 1);
			bzero(ret, 200);
			index++;
		} else {
			if(*foo == '"'){
				readspace = readspace^1;    // alternate value of read space on appearance of '"' character
			}
			else{
				strncat(ret, foo, 1);     // concatenates one character at a time to ret string
			}
		}
		foo++;
		/*printf("foo is %c\n", *foo);*/
	}
	MY_ARGV[index] = (char *)malloc(sizeof(char) * strlen(ret));
	//strncpy(MY_ARGV[index], ret, strlen(ret));
	//strncat(MY_ARGV[index], "\0", 1);
	strcpy(MY_ARGV[index], ret);
	ARGC = index + 1;   // no. of arguments
}

// frees MY_ARGV array
void free_argv()
{
	int index;
	for(index=0;MY_ARGV[index]!=NULL;index++) {
		bzero(MY_ARGV[index], strlen(MY_ARGV[index])+1);
		MY_ARGV[index] = NULL;
		free(MY_ARGV[index]);
	}
}

/*****
  START DREW's FUNCTIONS
 *****/

void linkf(){
	return ;
}

void unlinkf(){
	return ;
}

void cp(){
	return ;
}

void import(){
	return ;
}

void export(){
	return;
}
/*****
  END DREW's FUNCTIONS
 *****/

// executes command
void exec_cmd(char *cmd)
{
	//out = stdout;
	int chk;
	if(strcmp(cmd, "mkfs") == 0){
		create_fs();  
	}
	else if(strcmp(cmd, "mkdir") == 0){
		make_dir();
	}
	else if(strcmp(cmd, "rmdir") == 0){
		rm_dir();
	}
	else if(strcmp(cmd, "open") == 0){
		chk = openf();
	}  
	else if(strcmp(cmd, "cd") == 0){
		cd_path();    
	} 
	else if(strcmp(cmd, "ls") == 0){
		ls();
	}
	else if(strcmp(cmd, "seek") == 0){
		seek();
	}
	else if(strcmp(cmd, "write") == 0){
		writef();
	}
	else if(strcmp(cmd, "read") == 0){
		readf();
	}
	else if(strcmp(cmd, "close") == 0){
		close_f();
	}
	else if(strcmp(cmd,"tree") == 0){
		tree();
	}
	else if(strcmp(cmd, "stat") == 0){
		stat();
	}
	else if(strcmp(cmd, "startserv") == 0){
		start_server();
	}
	else if(strcmp(cmd, "exitserv") == 0){
		end_server();
	}
	else if(strcmp(cmd, "cat") == 0){
		cat();
	}
	else{
		printf("%s command not found!!\n", cmd);
	}
}


int main()
{
	char c;
	int i;
	char *tmp = (char *)malloc(sizeof(char) * 200);   // collects command line text; max 100 char assumed
	char *cmd = (char *)malloc(sizeof(char) * 200);   // holds command or value in 0 index of MY_ARGV
	void *t;

	signal(SIGINT, SIG_IGN);
	signal(SIGINT, handle_signal);
	// check if file system is already present; if present load the state of file system
	if(!(FSP = fopen(FSYS, "wb+"))){  // if present load file system state
		//  launch_fs();
	}

	printf("%s%s$ ", PROMPT, PromptPath);  // print virtual shell prompt
	fflush(stdout);
	// start reading of characters entered
	while(c != EOF) {
		c = getchar();
		switch(c) {
			case '\n': if(tmp[0] == '\0') {
						   printf("%s%s$ ", PROMPT, PromptPath);
					   } else {
						   fill_argv(tmp);     // parse command line text into indexed array based on space
						   t = strncpy(cmd, MY_ARGV[0], strlen(MY_ARGV[0]));   // string at 0 of MY_ARGV is command to be executed
						   t = strncat(cmd, "\0", 1);
						   // check if it is our commandlist and evaluate arguments and execute
						   exec_cmd(cmd);
						   free_argv();
						   printf("%s%s$ ", PROMPT, PromptPath);
						   bzero(cmd, 200);
					   }
					   bzero(tmp, 200);
					   break;
			default: strncat(tmp, &c, 1);
					 break;
		}
	}
	free(cmd);
	free(tmp);	

	printf("\n");
	fclose(FSP);
	return 0;
}