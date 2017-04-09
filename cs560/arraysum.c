#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define NTHREADS 4
#define ARRAYSIZE 1000000
int main()
{
	int i; double sum = 0; double a[ARRAYSIZE];
	omp_set_num_threads(NTHREADS);  // not sure if thread had to 4 or just leave it default
	#pragma omp parallel for reduction(+:sum)
	for(i = 0; i < ARRAYSIZE; i++){
		a[i] = i * 1.0;
		sum = sum + a[i];		
	}
	printf("sum = %.2f\n", sum);
	
	sum = 0;
	for(i=0; i < ARRAYSIZE; i++){
		a[i] = i * 1.0;
		sum = sum + a[i];
	}
	printf("check sum = %.2f\n", sum);
	return 0;
}

