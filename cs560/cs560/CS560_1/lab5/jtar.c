/* jtar.c
 * Drew Nash -- anash4@eecs.utk.edu
 * CS 360-002, Spring 2013 (Monday lab)
 * 7 March 2013 (submitted one day late -- sorry)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <utime.h>
#include <dirent.h>
#include "dllist.h"
#include "fields.h"
#include "jrb.h"

/* This program takes directory(ies) or file(s) as arguments and either compresses or extracts them.
   The usage is jtar [cxv] [files...]
   'c' compresses the files, and 'x' extracts them.
   The files are compressed and extracted to/from a 'tarfile'
*/

/* This function compresses the files that are passed to it and it recursively calls itself */
void compress_files(char *filename, JRB inode_list, JRB realpath_list){
	struct stat buf;		/* buffer to contain the info in 'stat' */
	DIR *d;					/* directory pointer */
	struct dirent *de;		/* directory entry pointer */
	int exists, i, j, k, node; /* boolean and induction variables */
	char *s, *tmpfile;		/* buffers */
	Dllist files, tmp;		/* dllist to keep track of files */
	FILE *f;				/* file pointer */
	char c[100000];			/* file contents buffer */
	
	files = new_dllist();	/* holds list of files that are compressed to prevent duplicates */
	
	/* Check to be sure that a file actually does exist before doing anything */
	exists = lstat(filename, &buf);
	if (exists < 0){
		fprintf(stderr, "Couldn't stat %s\n", filename);
	}
	
	/* If this is a directory, recursively call the contents */
	if (S_ISDIR(buf.st_mode)){ /* syntax for checking to see if this is a directory */
		tmpfile = (char *) malloc(sizeof(char)*(strlen(filename)+258)); /* allocate file name string */
		realpath(filename, tmpfile); /* get the actual path */
		if (readlink(filename) == -1){ /* should be hard link if -1 (otherwise, ignore soft links) */

			/* If this directory has not been printed in the tarfile, do so, otherwise, skip */
			if (jrb_find_str(realpath_list, (char *) tmpfile) == NULL){
				fwrite(filename, 500, 1, stdout); /* filename is 500 bytes (arbitrary number that works) */
				fwrite(&buf, sizeof(struct stat), 1, stdout); /* store the stat of the directory */

				/* Insert the inode in to a dllist to ensure duplicates aren't stored */
				if (jrb_find_int(inode_list, buf.st_ino) == NULL){
					jrb_insert_int(inode_list, buf.st_ino, JNULL);
				}

				/* Insert the realpath into a dllist to ensure that hardlinks are stored */
				jrb_insert_str(realpath_list, (char *) tmpfile, JNULL);
			}
		}
	
		/* open the file, and if it fails, exit */
		d = opendir(filename);
		if (d == NULL){
			perror(filename);
			exit(2);
		}
		s = (char *) malloc(sizeof(char)*(strlen(filename)+258)); /* allocate file name string */
		
		/* traverse through the entire directory using the following syntax */
		for (de = readdir(d); de != NULL; de = readdir(d)){
			if ((strcmp(de->d_name, ".") != 0) && (strcmp(de->d_name, "..") != 0)){ /* be sure to ignore '.' */
				sprintf(s, "%s/%s", filename, de->d_name); /* add the '/' for each level */
				dll_append(files, new_jval_s(strdup(s))); /* add these names to the dllist */
			}
		}
		closedir(d); /* close all directories */

	} else if (strcmp(filename, ".") == 0){
		dll_append(files, new_jval_s(strdup(filename))); 
	}
	
	/* traverse through the list of directories and print the files and contents */
	dll_traverse(tmp, files){
		strcpy(s, tmp->val.s);

		/* if a files does not exist, ignore */
		exists = lstat(s, &buf);
		if (exists < 0){
			fprintf(stderr, "Couldn't stat %s\n", tmp->val.s);
		}
		
		/* If it is a directory, call itself recursively to get the files inside */
		if (S_ISDIR(buf.st_mode)){
			compress_files(tmp->val.s, inode_list, realpath_list);
		} else { /* Otherwise, go ahead and write the file */
			tmpfile = (char *) malloc(sizeof(char)*(strlen(tmp->val.s)+258)); /* Allocate file name */
			realpath(tmp->val.s, tmpfile);
			if (readlink(tmp->val.s) == -1){ /* should be hard link */

				/* be sure to prevent the writing of duplicate data */
				if (jrb_find_str(realpath_list, (char *) tmpfile) == NULL){
					fwrite(tmp->val.s, 500, 1, stdout); /* allocate file name */
					fwrite(&buf, sizeof(struct stat), 1, stdout); /* write the stat */
					node = buf.st_ino;
				
					/* open the file and write the contents to the tarfile */
					if ((jrb_find_int(inode_list, node) == NULL)){
						f = fopen(tmp->val.s, "r"); /* open it with "read" access */
						if (f == NULL){
							perror(tmp->val.s);
							exit(3);
						}
						
						/* read the contents of the file using the file size and print them to the tarfile */
						i = fread(&c, buf.st_size, 1, f);
						if (i == 0) break;
						fclose(f); /* be sure to close the file! */
						fwrite(c, buf.st_size, sizeof(char), stdout);
						jrb_insert_int(inode_list, node, JNULL);
					}
					jrb_insert_str(realpath_list, (char *) tmpfile, JNULL);
				}
			}
		}
	}
	free_dllist(files); /* free the list to save memory */
}

/* this function takes the tarfile and extracts the contents back to their original state */
void xtract_files(){
	char buf[100000];	/* tarfile buffer */
	char c[100000];		/* contents buffer */
	struct stat statbuf;/* stat buffer */
	int i, j, node;		/* induction variables */
	FILE *f;			/* file pointer */
	JRB file_list, jnode, jnode2; /* jrb trees for list of files */
	JRB diratime_list, dirmtime_list, dirmod_list; /* mod, mtimes, atimes */
	struct utimbuf times; /* time struct */

	/* make the necessary trees */
	file_list = make_jrb();
	dirmod_list = make_jrb();
	diratime_list = make_jrb();
	dirmtime_list = make_jrb();
	i = 1;
	while (i > 0){ /* reset the buffer */
		strcpy(buf, "");
	
		/* read the file name and the stat */
		i = fread(&buf , 500, 1, stdin);
		i = fread(&statbuf, sizeof(struct stat), 1, stdin);
		if (i <= 0) break;
		
		/* if its a directory,create the directory and store the values in the respective trees */
		if (S_ISDIR(statbuf.st_mode)){
			mkdir(strdup(buf), 0777);
			jrb_insert_str(dirmod_list, strdup(buf), new_jval_l(statbuf.st_mode));
			jrb_insert_str(diratime_list, strdup(buf), new_jval_l(statbuf.st_atime));
			jrb_insert_str(dirmtime_list, strdup(buf), new_jval_l(statbuf.st_mtime));
		} else {

			/* otherwise, write the file */
			node = statbuf.st_ino;

			jrb_traverse(jnode, file_list){
				/* required because "jnode" is buggy */
			}
			
			/* make sure to keep track of the files written */
			jnode = jrb_find_int(file_list, node);
			if (jnode == NULL){
				jrb_insert_int(file_list, node, new_jval_s(strdup(buf)));
				f = fopen(buf, "w"); /* open the file with "write" access */
				i = fread(&c, statbuf.st_size, sizeof(char), stdin); /* read contents */
				
				if (i == 0) break;
				fwrite(&c, statbuf.st_size, sizeof(char), f); /* write contents */
				fclose(f); /* be sure to close the file! */
				times.actime = statbuf.st_atime; /* set access time */
				times.modtime = statbuf.st_mtime;/* set mod time */

				chmod(buf,  statbuf.st_mode); /* set protection */
				utime(buf, &times); /* set times */
			} else {
				j = link(jnode->val.s, buf); /* otherwise, link files */
			}
		}
	}

	/* set directory protection */
	jrb_traverse(jnode, dirmod_list){
		chmod(jnode->key.s, jnode->val.l);
	}

	/* set directory times */
	jrb_traverse(jnode, diratime_list){
		times.actime = jnode->val.l;
		jnode2 = jrb_find_str(dirmtime_list, jnode->key.s);
		times.modtime = jnode2->val.l;
		utime(jnode->key.s, &times);
	}
}	


int main(int argc, char **argv){
	int i;
	JRB inode_list, realpath_list; /* list of inodes and paths */

	inode_list = make_jrb();
	realpath_list = make_jrb();

	/* check for errors */
	if (argc < 2){
		fprintf(stderr, "usage: jtar [cxv] [files ...]\n");
		exit(1);
	}

	if ((strcmp(argv[1], "c") != 0) && (strcmp(argv[1], "x") != 0) && (strcmp(argv[1], "cv") != 0) && (strcmp(argv[1], "xv") != 0)){
		fprintf(stderr, "usage: jtar [cxv] [files ...]\n");
		exit(1);
	}
	
	/* compress */
	if (strcmp(argv[1], "c") == 0){
		for (i = 2; i < argc; i++){
			compress_files(argv[i], inode_list, realpath_list);
		}
	}
	else if (strcmp(argv[1], "x") == 0){
		xtract_files();  /* extract */
	}

	return 0;
}
