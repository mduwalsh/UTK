
#include <iostream>
#include <cstdlib>
#include <cmath>
#include "time.h"
#include "Pr.h"
#define usage "Usage: ./fld traning_set testing_set k c nf\n\n"

using namespace std;
int main (int argc, char **Set)
 {	
	clock_t t0=clock(),t1;
	

  if (argc < 6) {
    cout << usage;
    exit(1);
  }
	int i, j,w,f;
	int nc,nr,nf,nrte;
	int c,k,r,Result,error=0;
	char *h=Set[3],*h1=Set[4],*h2=Set[5];
	k=atoi(h);//for k
	c=atoi(h1);//for number of class
	nf=atoi(h2);// for number of feature 

	Matrix data,subt,sub,te,tmp,Mx,output,cm;
	double a,b,n,m=0;

	data = readData(Set[1],nf+1); 
	te = readData(Set[2],nf+1);

	nc = data.getCol();
    nr = data.getRow();
	nrte= te.getRow();

	tmp.createMatrix(k,2);
	Mx.createMatrix(c,2);
	output.createMatrix(nrte,3);
	cm.createMatrix(c+1,c+1);

	for (f=0;f<nrte;f++){
	subt=subMatrix(te,f,0,f,nf-1);  //testing sample
           
		for (i=0;i<k;i++)//finding euc dist bewteen first k sample in training set and testing sample
		{
		sub=subMatrix(data,i,0,i,nf-1);
		
		tmp(i,0)=euc(subt,sub);
		tmp(i,1)=data(i,nf);
		
		}

		for(j=k;j<nr;j++)
		{
			for (w=0;w<k;w++){   //bubble sorting for partial set
				for (i=0;i<k-1;i++){
					if (tmp(i,0)<tmp(i+1,0)){
					a= tmp(i+1,0);
					b= tmp(i+1,1);
					tmp(i+1,0)=tmp(i,0);
					tmp(i+1,1)=tmp(i,1);
					tmp(i,0)=a;
					tmp(i,1)=b;
					}
				}
			}

			sub=subMatrix(data,j,0,j,nf-1);
			n=euc(subt,sub);
			if (tmp(0,0)>n){
				tmp(0,0)=n;
				tmp(0,1)=data(j,nf);}

		}

	



		for (i=0;i<c;i++)//generate labels 
		Mx(i,0)=i;

		for (j=0;j<c;j++)//counting how many sample for each lable 
		{
		r=0;
		for (i=0; i<k; i++)
		{	
		if ((tmp(i,1)==j))
		r++;
		}
		Mx(j,1)=r;
		}

		for (i=0;i<c;i++){
			if (Mx(0,1)<Mx(i+1,1)){
			a= Mx(i+1,0);
			b= Mx(i+1,1);
			Mx(i+1,0)=Mx(0,0);
			Mx(i+1,1)=Mx(0,1);
			Mx(0,0)=a;
			Mx(0,1)=b;
				}
			}

//Mx(0,0) is the result of classification 
Result=Mx(0,0);
//generating confustion matrix element 
	output(f,0)=f;//sample number
	output(f,1)=te(f,nf);//actual result
	output(f,2)=Result;//classification result 
if (Result != te(f,nf))
      error++;


}

//generate CM
	cm(0,0)=0;
	for (i=0;i<c;i++)
	{ 	cm(0,i+1)=i;
		cm(i+1,0)=i;
	}


	for(r=0;r<c;r++){
	cout<<output;
		for(i=0;i<c;i++)
		{
		w=0;
		for(j=0;j<nrte;j++)
			{
			if ((output(j,2)==r)&&(output(j,1)==i))
			//if(output(j,1)=i)
			w++;
			}
		cm(i+1,r+1)=w;
		}
		

	}
	//writeData(output,"output");
	writeData(cm,"cmKNN");
cout<<"error Rate"<<(double)error/nrte<<endl;
t1=clock()-t0;
cout<<"time"<<t1<<endl;
return 0;

}

