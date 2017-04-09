
#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdlib>
#include "Matrix.h"           
#include "Pr.h"
 
using namespace std;
 
#define Usage "Usage: ./testBKS cl1_output_file cl2_output_file cl3_output_file c \n"

int main(int argc, char **argv) 
{
	// check to see if the number of argument is correct 
	if (argc ^ 5) {
	cout << Usage;     
	exit(1);   
	}
	// read data from from source in command arguments
	int c=atoi(argv[4]);
	Matrix cl1=readData(argv[1],2);
	Matrix cl2=readData(argv[2],2);
	Matrix cl3=readData(argv[3],2);
	
	int nr=cl1.getRow();
	// prepare data for bks
	Matrix T=subMatrix(cl1,0,0,nr-1,0);
	Matrix S1=subMatrix(cl1,0,1,nr-1,1);
	Matrix S2=subMatrix(cl2,0,1,nr-1,1); 
	Matrix S3=subMatrix(cl3,0,1,nr-1,1);
	
	// call bks
	Matrix F=bks(T,S1,S2,S3,c);
	writeData(F,"lookupBKS.txt");
	cout<<F<<endl;	
	
	// classify with the lookup table 
	int error=0;
	for(int i=0;i<nr;i++)
	{
		for(int j=0;j<pow(c,3);j++)
		{
			if(S1(i,0)==F(j,0) && S2(i,0)==F(j,1) && S3(i,0)==F(j,2))
			{
				if(F(j,3+c)!=T(i,0))
				{
					error++;
				}
				break;
			}
		}
	}	
	cout<<"The error rate is: "<<(double)error/nr<<endl;
	return 0;
}
