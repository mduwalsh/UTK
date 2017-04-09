// currently only works for 2 classes

#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include "Matrix.h"           
#include "Pr.h"

using namespace std;

#define Usage "Usage: ./testkNN training_set test_set classes features use_partial_dist minkowski_order dimension_reduction_method k \n\t training_set: the file name for training set\n\t test_set: the file name for test set\n\t classes: number of classes \n\t features: number of features (dimension) \n\t k: no. of neighbours to be considered \n\t use_partial_dist: to use partial distance '1' else '0' \n\t minkowski_order: order for minkowski distance calculation \n\t \n\t dimension_reduction_method: 0,1,2 for do not use, fld, pca \n\t "

int main(int argc, char **argv)
{
  int nrTr, nrTe,        // number of rows in the training and test set
    nc;                  // number of columns in the data set; both the 
                         // training and test set should have the same
                         // column number                         
  Matrix Tr(0,0), Te(0,0);

  // check to see if the number of argument is correct
  if (argc ^ 9) {
    cout << Usage;
    exit(1);
  }

	// start time
	clock_t sTime=clock();

	int classes = atoi(argv[3]);   // number of classes
  	int nf = atoi(argv[4]);        // number of features (dimension)
	int use_part_dist=atoi(argv[5]); // use partial distance or not
	int p= atoi(argv[6]); // order of minkowski distance where 2 equivalents to euclidean distance
	int dr=atoi(argv[7]); // dimension reduction method to be used
	int k=atoi(argv[8]); // k of nearest neighbours
	
   
	// read in data from the data file
	nc = nf+1; // the data dimension; plus the one label column 
	Tr = readData(argv[1], nc);
	nrTr = Tr.getRow();   // get the number of rows
	Te = readData(argv[2], nc);
	nrTe = Te.getRow();   // get the number of rows

	// normalization of data set
	normalize(Tr, Te, nf, 1); // normalize sample data in training and test set both; flag set to 1 for testing set normalization

	int flag=1; // to include testing sample or not in dimension reduction, '1' means include yes
		
	// FLD method
	if(dr==1)
	{
		int fnf=fld(Tr, Te, classes, nf, flag); // apply fld dimension reduction 

		// now crop training and test set to reduced dimensions but keep label column
		// first assign value of label column to column after fnf
		for(int i=0;i<nrTr;i++)
		{
			Tr(i,fnf)=Tr(i,nf);
		}
		for(int i=0;i<nrTe;i++)
		{
			Te(i,fnf)=Te(i,nf);
		}
		Matrix fTr=subMatrix(Tr,0,0,nrTr-1,fnf);
		Matrix fTe=subMatrix(Te,0,0,nrTe-1,fnf);
		Tr=fTr;Te=fTe;nf=fnf; 
	}
	else if(dr==2)
	{
		float err=0.1; // maximum error allowed
		int flag=1; // to include Testing sample or not in normalization
		int pnf=pca(Tr, Te, classes, err, flag); // apply pca dimension reduction 

		// now crop training and test set to reduced dimensions but keep label column
		// first assign value of label column to column after pnf
		for(int i=0;i<nrTr;i++)
		{
			Tr(i,pnf)=Tr(i,nf);
		}
		for(int i=0;i<nrTe;i++)
		{
			Te(i,pnf)=Te(i,nf);
		}
		Matrix pTr=subMatrix(Tr,0,0,nrTr-1,pnf);
		Matrix pTe=subMatrix(Te,0,0,nrTe-1,pnf);
		Tr=pTr;Te=pTe;nf=pnf;
	}

	// prepare the labels and error count
	Matrix labelkNN(nrTe,1);  // a col vector to hold result for MPP
	int errCountkNN = 0;       // calcualte error rate for MPP
	// intialize TP,TN,FP,FN
	int TP=0,TN=0,FP=0,FN=0;		

  // perform classification
  for (int i=0; i<nrTe; i++) 
{ 
    // classify one test sample at a time, get one sample from the test data
    Matrix sample = subMatrix(Te, i, 0, i, nf-1); 
	
    // call kNN to perform classification
	if(use_part_dist==1)
	{
		labelkNN(i,0) = knnPartDist(Tr, sample, k, nf, classes, p);
	}
	else
	{
		labelkNN(i,0) = knn(Tr, sample, k, nf, classes, p);
	}
    
    // check if the classification result is correct or not
    	if (labelkNN(i,0) != Te(i,nf))
	{
      		errCountkNN++;
	}
	
  }

	// end time
	clock_t eTime=clock();

	// calculate accuracy
	cout << "The error rate using kNN =" << (float)errCountkNN/nrTe << endl;
	cout<<"The execution time = "<<(eTime-sTime)/(double)CLOCKS_PER_SEC<<endl;
  
  return 0;
}
