
#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdlib>
#include "Matrix.h"           
#include "Pr.h"
 
using namespace std;
 
#define Usage "Usage: ./testWTA data_file nf k CM_file cl_file \n"

int main(int argc, char **argv) 
{
	// check to see if the number of argument is correct 
	if (argc ^ 6) {
	cout << Usage;     
	exit(1);   
	}

	int nf=atoi(argv[2]);	
	int nc=nf+1;
	Matrix Tr=readData(argv[1],nc);
	char *cm_file=argv[4];
	char *cl_file=argv[5];
	int k=atoi(argv[3]);	
	 
	int nrTr=Tr.getRow();
	
	Matrix mTr=subMatrix(Tr,0,0,nrTr-1,nf-1);
	Matrix oTrL(nrTr,1);

	// define arbitrary clr cluster means 
	Matrix C(k,nf);      
	
	// call kmeans to get lookup table that refers each pixel in original pic to updated cluster mean pixel
	Matrix lookup;
	lookup=winnerTakesAll(mTr,C,1,0.000005);   

	// find which cluster mean belongs to which class in supervised method
	Matrix x(k,k);  
	for(int i=0;i<nrTr;i++)
	{
		for(int j=0;j<k;j++)
		{
			if(lookup(i,0)==j) 
			{
				x(j,Tr(i,nf))+=1;			
			} 			
		}
	}
	Matrix s(1,k); 
	Matrix pos(1,k);
	Matrix clookup(k,1); 
	Matrix d;
	Matrix output, cm;
	int error=0;
	output.createMatrix(nrTr,2);
	cm.createMatrix(k+1,k+1);
	for(int j=0;j<k;j++)
	{
		d=subMatrix(x,j,0,j,k-1);
		insertsort(d,s,pos);
		clookup(j,0)=pos(0,k-1); 			
	}
	// assign class according to lookup content using clookup mapping
	for(int i=0;i<nrTr;i++)
	{
		oTrL(i,0)=clookup(lookup(i,0),0);
		output(i,0)=Tr(i,nf);//actual result
		output(i,1)=oTrL(i,0);//classification result 
		if(oTrL(i,0)!=Tr(i,nf))
		error++;		
	}
	// create confusion matrix
	
	//generate CM
	cm(0,0)=0;
	for (int i=0;i<k;i++)
	{ 	cm(0,i+1)=i;
		cm(i+1,0)=i;
	}


	for(int r=0;r<k;r++){
		for(int i=0;i<k;i++)
		{
			int w=0;
			for(int j=0;j<nrTr;j++)
			{
			if ((output(j,1)==r)&&(output(j,0)==i))
			w++;
			}
			cm(i+1,r+1)=w;
		}
	}
	cout<<x<<endl;
	cout<<cm<<endl;
	cout<<clookup<<endl;
	cout<<C<<endl;
	//writeData(output,"output");
	//writeData(cm,cm_file);
	//writeData(output,cl_file);
	cout<<"error rate = "<<(double)error/nrTr<<endl;		
	return 0;
}
