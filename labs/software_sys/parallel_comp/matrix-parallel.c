/******************************************************************************
 * FILE:	matrix-parallel.c
 * AUTHOR:	Drew Nash - anash4@utk.edu
 * DATE:	30 April 2015 
 * COURSE:	CS 560 - PA3
 * This program implements parallel matrix addition and multiplication.
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

/* This program generates random M x M square matrices, with entries being
   floating point numbers in the following range: [0, 100]. The syntax to
   run this program is as follows:
   ./matrix-parallel mode size print
   with `mode' being either `1' for sum or `2' for product, `size' being
   an integer so that the generated matrices are `size' x `size', and
   `print' being `0' or `1' for false or true, respectively.
*/

/* Print the matrix that is passed. */
void print_matrix(int size, long double **M){
	int i, j;

	for (i = 0; i < size; i++){
		for (j = 0; j < size; j++){
			printf(" %10.2Lf", M[i][j]); /* `Lf' is used with long doubles */
		}
		printf("\n");
	}
	return;
}

/* Add two passed matrices `M1' and `M2' and store the sum in `result' */
void parallel_add(int size, long double **M1, long double **M2, long double **result){
	int i, j;

	omp_set_nested(1); /* Set nested to true so two teams of threads are used */
	#pragma omp parallel for /* All iterations of for loop executed simulataneously */
	for (i = 0; i < size; i++){
		#pragma omp parallel for /* One thread handles each iteration of for loop */
		for (j = 0; j < size; j++){
			result[i][j] = M1[i][j] + M2[i][j]; /* No need for lock since value is updated once */
		}
	}
	return;
}

/* Multiply two passed matrices `M1' and `M2' and store the product in `result' */
void parallel_multiply(int size, long double **M1, long double **M2, long double **result){
	int i, j, k;
	
	omp_set_nested(1); /* Set nested to true so two teams of threads are used */

	/* Algorithm is referenced from ``Introduction to Algorithms'' in Cormen et al. */
	#pragma omp parallel for /* All iterations of for loop executed simulataneously */
	for (i = 0; i < size; i++){
		#pragma omp parallel for private(k) /* One thread handles each iteration of for loop */
		for (j = 0; j < size; j++){
			result[i][j] = 0; /* No need for lock since value is updated once */
			/* `k' is set to private so each thread can go through its own set of iterations */
			for (k = 0; k < size; k++){
				result[i][j] += M1[i][k] * M2[k][j];
			}
		}
	}
	return;
}

int main(int argc, char **argv){
	int i, j, size, mode, print;

	/* Error checking to confirm correct syntax to run program */
	if (argc != 4){
		fprintf(stderr, "ERROR - Usage: matrix-parallel mode size print\n");
		exit(1);
	}

//	srand(time(NULL));      /* Seed random number generator with time */
	print = atoi(argv[3]);
	size = atoi(argv[2]);
	mode = atoi(argv[1]);

	if (mode < 1 || mode > 2){
		fprintf(stderr, "ERROR - Modes: Sum = 1 | Product = 2\n");
		exit(1);
	}   
	if (size < 1 || size > 10000){
		fprintf(stderr, "ERROR - Size: int between 1 and 10000\n");
		exit(1);
	}
	if (print < 0 || print > 1){
		fprintf(stderr, "ERROR - Print: False = 0 | True = 1\n");
		exit(1);
	}

	/* Matrices are 2-dimensional arrays of long doubles, so that large values
	   can be stored, which is important when multiplying large matrices */
	long double **M1;
	long double **M2;
	long double **result;

	/* Memory must be allocated for the primary array of pointers to long doubles */
	M1 = (long double **) malloc(size * sizeof(long double *));
	M2 = (long double **) malloc(size * sizeof(long double *));
	result = (long double **) malloc(size * sizeof(long double *));

	/* Memory must be allocated for each secondary array of long doubles */
	/* NOT parallelized to accurately compare parallel and singular matrix operations */
	for (i = 0; i < size; i++){
		M1[i] = (long double *) malloc(size * sizeof(long double));
		M2[i] = (long double *) malloc(size * sizeof(long double));
		result[i] = (long double *) malloc(size * sizeof(long double));
	}

	/* Load `M1' and `M2' with random floating point numbers from 0 to 100, inclusive */
	/* NOT parallelized to accurately compare parallel and singular matrix operations */
	for (i = 0; i < size; i++){
		for (j = 0; j < size; j++){
			M1[i][j] = (long double) rand() / (long double) (RAND_MAX / 100);
			M2[i][j] = (long double) rand() / (long double) (RAND_MAX / 100);
		}
	}

	/* Print matrices `M1' and `M2', if requested */
	if (print){
		printf("Matrix 1:\n");
		print_matrix(size, M1);
		printf("\n");
		printf("Matrix 2:\n");
		print_matrix(size, M2);
		printf("\n");
	}

	if (mode == 1){	/* Add `M1' and `M2', then print `result' */
		parallel_add(size, M1, M2, result);
		if (print){ /* Print only if requested */
			printf("Sum:\n");
			print_matrix(size, result);
			printf("\n");
		}
	}
	else if (mode == 2){	/* Multiply `M1' and `M2', then print `result' */
		parallel_multiply(size, M1, M2, result);
		if (print){ /* Print only if requested */
			printf("Product:\n");
			print_matrix(size, result);
			printf("\n");
		}
	}

	return 0;
}
