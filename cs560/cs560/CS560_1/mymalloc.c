/*	mymalloc.c
 *	Drew Nash -- anash4@eecs.utk.edu
 *	CS360-002, Spring 2013 (Monday lab)
 *  31 March 2013 (Yes, I finished this on Easter Sunday)

This program works just like malloc() and free(). It contains the functions declared in mymalloc.h,
which was provided with the lab instructions. my_malloc() uses sbrk() to allocate memory on the
stack. It returns a pointer to the first usable memory location that has been allocated. my_free()
takes a pointer to the first usable memory location that has been allocated. It then takes the array
and "frees" it by adding the location and size to the free list to possibly be reallocated later.

The instructions included several restrictions:
1. Only one global variable allowed -- SATISFIED
2. sbrk() must only be called on vals >= 8192 -- SATISFIED
3. Only external procedure calls allowed: malloc(), qsort(), free(), (all only once) -- SATISFIED
4. Libraries cannot be utilized -- SATISFIED
5. Should not assume that sbrk() is not called by other procedures the user may use -- OKAY
6. No more than 8 bytes should be allocated for bookkeeping for each chunk -- SATISFIED
7. Free list nodes should have a minimum size of 8 or 12 bytes -- SATISFIED (all are min. 12 bytes)
8. Assume that all pointers are 4 bytes -- OKAY (the gradescript automatically compiles with -m32)

No makefile was created for this lab since the gradescript actually compiles the programs as part
of its shell commands.
*/

#include <stdio.h>
#include <stdlib.h>
#include "mymalloc.h"

/* This typedef struct is from the lecture notes and forms the nodes of the Free list, which is
   essentially nodes linked together that hold chunks of memory that are ready to be allocated
   by my_malloc(). */
typedef struct flist {
	int size;				/* size of the node, including metadata */
	struct flist *flink;	/* forward pointer to the next node */
	struct flist *blink;	/* back pointer to the previous node */
} *Flist;

void *malloc_head = NULL;	/* pointer to head of Free list -- only global variable */

/* My malloc allocates memory of the passed size and returns a pointer to the first usable address
   of that allocated memory. The size of the allocated memory is stored at the front of this memory
   Therefore, the first usable address is 8 bytes past the size */
void *my_malloc(size_t size){
	Flist f;				/* Free list node -- temporary */
	Flist location = NULL;	/* Free list node -- location of allocated memory */
	int *ptr, *ptr2;		/* Temporary pointers */
	int total_size;			/* Total size of the allocated memory (including bookkeeping) */
	int node_found = 0;		/* Boolean used to indicate if node of correct size is found */

	/* First, the size that is passed must be set to a multiple of 8 greater than or equal to
	   the passed size so that it can be properly aligned */
	if (size % 8 != 0) size += 8 - (size % 8);	/* set it to multiple of 8 */
	size += 8;	/* add 8 bytes for additional info */

	/* The Free list needs to be checked first before sbrk() is called. malloc_head is NULL if the
	   list is empty */
	if (malloc_head != NULL){
		f = (Flist) malloc_head;
		if (f->size < size){	/* If the head is not the correct size, check the other nodes */
			while (f->flink != NULL){ /* f->flink == NULL when at end of list */
				f = f->flink;
				if (f->size >= size){ /* If the node is the correct size, save it */
					location = f;
					total_size = location->size;
					node_found = 1;
					break;
				}
			}
		} else { /* If the head is the correct size, save it */
			location = f;
			total_size = location->size;
			node_found = 1;
		}
	}
	
	/* If a node is found, take the appropriate amount of memory off the back of the node */
	if (node_found){
		/* If, once the proper amount of memory has been taken, the node has 12 bytes or greater
		   remaining, keep the node remainder in the list. Otherwise, use the whole thing */
		if (total_size - size >= 12){
			ptr = (void *) location + location->size - size; /* pointer to start of memory */
			*ptr = (int) size;	/* set the size at the start of memory */
			location->size -= size; /* change the size of the node */
		} else {
			ptr = (void *) location; /* pointer to start of memory (whole node) */
			/* If the node is the malloc_head, set it to NULL since it is now in use */
			if (location == malloc_head){
				malloc_head = NULL;
			}
			/* Delete the node from the Free list by setting the appropriate links */
			if (location->flink != NULL){
				if (location->blink != NULL){
					location->flink->blink = location->blink; /* Set next node's blink */
				} else {
					location->flink->blink = NULL;
					malloc_head = location->flink; /* Set malloc_head, if necessary */
				}
			}
			if (location->blink != NULL){
				if (location->flink != NULL){
					location->blink->flink = location->flink; /* Set previous node's flink */
				} else {
					location->blink->flink = NULL;
				}
			}
			/* Delete the node by setting links to NULL */
			location->flink = NULL;
			location->blink = NULL;
		}
	} else { /* If no node is found, call sbrk() */
		if (size - 8 > 8176){ /* Only call sbrk(>8196) iff passed size >= 8176 */
			ptr = (void *) sbrk(size); /* pointer to allocated memory */
			total_size = size; /* total size of allocated memory */
		} else { /* Do not call sbrk() with vals < 8192 */
			ptr = (void *) sbrk(8192);
			total_size = 8192;
		}
		/* If the actual size minus the passed (user-requested) size >= 12, return the passed size
		   plus 8 additional bytes and keep the rest in the free list. Otherwise, just return the
		   entire thing. */
		if (total_size - size >= 12){
			location = (Flist) ptr; /* create a node for the remaining memory */
			ptr = (void *) ptr + total_size - size; /* pointer to allocated memory */
			*ptr = (int) size; /* set the size at the start of memory */
			location->size = (int) total_size - size; /* set size of node */
			location->flink = NULL; /* flink is always NULL since it is the newest node */
			/* If there is no malloc_head, set this new node as the malloc_head */
			if (malloc_head == NULL){
				malloc_head = location;
				location->blink = NULL;
			} else { /* Otherwise, find the most recent node before this one */
				f = malloc_head;
				while (f->flink != NULL){
					f = f->flink;
				}
				location->blink = f; /* This node's previous link is the most recent before this */
				f->flink = location; /* That node's flink is this node */
			}
		} else {
			*ptr = total_size; /* set size at beginning of allocated memory */
		}
	}
	ptr += 2; /* Return a pointer 8 bytes past beginning sizeof(ptr) * 2 */
	return ptr;
}

/* This function takes a previously allocated chunk of memory and "frees" it by putting the
   memory in a node in the Free list. */
void my_free(void *ptr){
	Flist location, f;	/* Free list location and temporary */
	int *size;			/* Pointer to size */

	ptr -= 2 * 4; /* Take the pointer 8 bytes backward to the beginning */
	location = (Flist) ptr; /* Create the node */
	size = (int *) ptr;	/* Get the size of the memory */
	location->size = *size;	 /* Set the node's size */
	location->flink = NULL; /* flink is always NULL since it is the newest node */
	/* If there is no malloc_head, set this new node as the malloc_head */
	if (malloc_head == NULL){
		malloc_head = location;
		location->blink = NULL;
	} else { /* Otherwise, find the most recent node before this one */
		f = (Flist) malloc_head;
		while (f->flink != NULL){
			f = f->flink;
		}
		location->blink = f; /* This node's previous link is the most recent before this */
		f->flink = location; /* That node's flink is this node */
	}
	return;
}

/* This function returns the first node of the Free list, which is the malloc_head, and is
   NULL if the list is empty */
void *free_list_begin(){
	return malloc_head;
}

/* This function returns the next node of passed node of the Free list, which is the passed node's
   flink */
void *free_list_next(void *node){
	Flist f;
	f = (Flist) node;
	return f->flink;
}

/* This function is used by qsort() and is similar to the function given on the qsort() reference
   webpage on www.cplusplus.com. It takes two pointers to two vals (in this case pointers) as
   arguments. If ptr1 is less than ptr2, a negative value is returned. If ptr2 is less than ptr1,
   then a positive value is returned. If they are equal, then 0 is returned. This function
   is used to sort the pointers in the array of pointers in coalesce_free_list() */
int compare(const void *ptr1, const void *ptr2){
	return (*(void **)ptr1 - *(void **)ptr2);
}

/* This function traverses through the Free list and combines the nodes that occupy adjaent blocks
   of memory. It calls malloc(), qsort(), and free() one time each, which is allowed by the
   instructions */
void coalesce_free_list(){
	void *ptr;		/* Temporary pointer */
	void **ptrs;	/* Array of pointers */
	int i, size;	/* Induction variable, Size */
	Flist f, f2;	/* Temporary Free list nodes */

	/* First, the number of nodes in the Free tree needs to be determined */
	size = 0;
	if (malloc_head == NULL){
		return;
	} else {
		size++;
		f = (Flist) malloc_head;
		while (f->flink != NULL){
			f = f->flink;
			size++;
		}
	}
	
	if (size == 1) return; /* If the tree has one node, then nothing can be combined */

	ptrs = malloc(sizeof(ptr) * size); /* Allocate the array to the correct size */

	/* Add the pointers to each node to the array */
	f = (Flist) malloc_head;
	for (i = 0; i < size; i++){
		ptrs[i] =(Flist) f;
		f = f->flink;
	}
	
	/* Sort the array of pointers from lowest memory size to highest */
	qsort(ptrs, size, sizeof(ptr), compare);
	
	/* If two nodes occupy adjacent memory locations, delete one of the nodes and expand the
	   other node */
	for (i = 0; i < size - 1; i++){
		f = (Flist) ptrs[i]; /* set first node to compare */
		f2 = (Flist) ptrs[i + 1]; /* set second node to compare */
		ptr = ptrs[i]; /* set pointer to first node */
		ptr += f->size;	/* move pointer to end of first node */
		if (ptr == ptrs[i + 1]){ /* if the pointer is at the beginning of the second, combine */
			f->size += f2->size; /* increase size of first node */
			ptrs[i + 1] = ptrs[i]; /* remove second node pointer from array since it is deleted */
			/* When deleting the second node, set the links appropriately */
			if (f2->flink != NULL){
				if (f2->blink != NULL){
					f2->flink->blink = f2->blink; /* Next node's blink is previous node */
				} else {
					f2->flink->blink = NULL;
					malloc_head = f2->flink; /* Next node is malloc_head if current is head */
				}
			}
			if (f2->blink != NULL){
				if (f2->flink != NULL){
					f2->blink->flink = f2->flink; /* Previous node's flink is next node */
				} else {
					f2->blink->flink = NULL;
				}
			}
			/* Finally, delete the node */
			f2->flink = NULL;
			f2->blink = NULL;
		}
	}
	
	free(ptrs); /* Free the memory allocated for the array */
	return;
}
