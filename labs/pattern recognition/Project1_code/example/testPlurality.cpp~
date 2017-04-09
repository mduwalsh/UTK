
#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdlib>
#include "Matrix.h"           
#include "Pr.h"
 
using namespace std;
 
#define Usage "Usage: ./testPlurality cl1_output_file cl2_output_file cl3_output_file c \n"

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
	// prepare data for plurality fusion
	Matrix T=subMatrix(cl1,0,0,nr-1,0);
	Matrix S1=subMatrix(cl1,0,1,nr-1,1);
	Matrix S2=subMatrix(cl2,0,1,nr-1,1); 
	Matrix S3=subMatrix(cl3,0,1,nr-1,1);
	
	// test the lookup table 
	Matrix d(1,c);
	Matrix init(1,c);
	Matrix s(1,c);
	Matrix pos(1,c);
	int error=0;
	for(int i=0;i<nr;i++)
	{
		d=init;
		d(0,S1(i,0))+=1;
		d(0,S2(i,0))+=1;
		d(0,S3(i,0))+=1;
		insertsort(d,s,pos);
		if(pos(0,c-1)!=T(i,0))
		{
			error++;
		}	
		cout<<S1(i,0)<<" "<<S1(i,0)<<" "<<S1(i,0)<<" "<<T(i,0)<<" "<<pos(0,c-1)<<endl;			
	}	
	cout<<"The error rate is: "<<(double)error/nr<<endl;
	return 0;
}
