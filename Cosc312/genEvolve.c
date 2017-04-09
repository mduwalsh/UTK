#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>
#define pBit (int)(8*sizeof(unsigned long)) // no. of bits in unsigned long

typedef unsigned long *chromosome; // define chromosome as pointer of unsigned long data type
typedef unsigned long **population; // define population as pointer of pointer for unsigned long	

void generateChildren(chromosome parent1, chromosome parent2, chromosome child1, chromosome child2, chromosome mask, int bSize);
void swap(int *, int *);
int getRandom(int);
void displayChromosomePopulation(unsigned long**, int, int);
void writeToFileChromosomeGeneration(FILE *, population, int, int);
char* getBinaryFormat(unsigned long);

main(int argc, char **argv)
{
	if(argc^5)
	{
		printf("Usage: ./genEvolve output_file no_of_bits_in_chromosome no_of_generations no_of_population \n");
		exit(1);
	}
	char *filename=argv[1]; // output file where generations of population are stored
	int B=atoi(argv[2]); // no. of bits to represent a chromosome
	int G=atoi(argv[3]); // no. of generations
	int N=atoi(argv[4]); // no. of population in a generation
	int bSize=(1+(B-1)/pBit); // no. of unsigned long block needed to hold all bits in chromosome
	int i,j,k,l; // counter variables to be used in loops
	FILE *fp;
	fp=fopen(filename,"w+");
	// generation array allocation
	population *gen;
	gen=malloc(2*sizeof(population));

	int *pIndex=malloc(N*sizeof(int));// virtual index array for population
	// cromosome mask
	chromosome mask;
	mask=malloc(bSize*sizeof(unsigned long));

	// population size within generation allocation in memory	
	for(i=0;i<2;i++) 
	{
		gen[i]=malloc(N*sizeof(chromosome));			
	}

	// population chromosome size allocation
	for(i=0;i<N;i++)
	{
		gen[0][i]=malloc(bSize*sizeof(unsigned long));	
		gen[1][i]=malloc(bSize*sizeof(unsigned long));	
	}
	
	// set population values
	for(i=0;i<N;i++)
	{
		for(j=0;j<bSize;j++)
		gen[0][i][j]=1234556+i+j;;	
	}
	
	// assign returned value from function to calculate mask 
	//mask= callSomeFunction();
	for(j=0;j<bSize;j++)
		mask[j]=123405+j;
	
	// display 1st generation of population
	printf("Generation %d# \n",1);
	displayChromosomePopulation(gen[0], N, bSize);
	writeToFileChromosomeGeneration(fp,gen[1],N,bSize);
	k=0;
	do{
		// random sorting of integers within 0 and N-1
		// to pick random parents in pair with those integers as position	
		for(i=0;i<N;i++)
			pIndex[i]=i;
		for(i=0;i<N;i++)
		{
			swap(&pIndex[getRandom(N-i-1)],&pIndex[N-i-1]);
		}

		//generation of child from parents 
		for(j=0;j<N;j=j+2)
		{			
			generateChildren(gen[0][pIndex[j]], gen[0][pIndex[j+1]],gen[1][j],gen[1][j+1], mask, bSize);			
		}

		// set newly generated population as parents for next generation
		gen[0]=gen[1];
		printf("Generation %d#\n",k+2);
		displayChromosomePopulation(gen[1], N, bSize);
		writeToFileChromosomeGeneration(fp,gen[1],N,bSize);
		// increase counter
		k++;	
						
	}while(k<G-1); 
	// free memory allocated
	free(gen);
	free(pIndex);
	free(mask);
	// close file
	fclose(fp);
}

void generateChildren(chromosome parent1, chromosome parent2, chromosome child1, chromosome child2, chromosome mask, int bSize)
{
	int i;
	for(i=0;i<bSize;i++)
	{
		child1[i]= (parent1[i]&mask[i])|(parent2[i]&(~mask[i]));
		child2[i]= (parent1[i]&~mask[i])|(parent2[i]&(mask[i]));
	}
}

// returns random integer between 0 and max value passed
int getRandom(int max)
{
	return (rand()%(max+1));	

}
// interchange values of variables passed
void swap(int *n1, int *n2)
{
	int temp;
	temp=*n1;
	*n1=*n2;
	*n2=temp;  
}

// displays chromosome in binary format
void displayChromosomePopulation(unsigned long **pop, int popSize, int bSize)
{
	char *A;
	A=malloc(pBit*bSize*sizeof(char));	
	int k,j;
	for(j=0;j<popSize;j++)
	{
		strcpy(A,"");
		for(k=0;k<bSize;k++)
		{
			strcat(A,getBinaryFormat(pop[j][k]));
		}
		printf("%s\n",A);
	}
	free(A);
}

// returns number in binary string format
char* getBinaryFormat(unsigned long n)
{
	int i;
	unsigned long m=1;
	char *A=malloc(pBit*sizeof(char));
	m=m<<(pBit-1);
	strcpy(A,"");
	for(i=0;i<pBit;i++)
	{
		if((n&m)==0)
		{
			strcat(A,"0");
		}
		else 
		{
			strcat(A,"1");
		}
		m=m>>1;		
	}
	return A;
}

// writes chromosomes of a generation population to file 
void writeToFileChromosomeGeneration(FILE *fp, population pop, int popSize, int bSize)
{
	char *A;
	A=malloc(pBit*bSize*sizeof(char));	
	int k,j;
	for(j=0;j<popSize;j++)
	{
		strcpy(A,"");
		for(k=0;k<bSize;k++)
		{
			strcat(A,getBinaryFormat(pop[j][k]));
		}
		fprintf(fp,"%s\n",A);
	}
	fprintf(fp,"\n");
	free(A);
}

