/*
 *  testMpp.cpp - test routine to use MPP to process the synthetic dataset
 *
 */
#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdlib>
#include "Matrix.h"             
#include "Pr.h"

using namespace std;

#define Usage "Usage: ./testMpp training_set test_set classes features cases \n\t training_set: the file name for training set\n\t test_set: the file name for test set\n\t classes: number of classes\n\t features: number of features (dimension)\n\t cases: used for MPP, cases can only be 1, 2, or 3\n"

int main(int argc, char **argv)
{
  int nrTr, nrTe,        // number of rows in the training and test set
    nc;                  // number of columns in the data set; both the 
                         // training and test set should have the same
                         // column number                         
  Matrix Tr, Te;

  // check to see if the number of argument is correct
  if (argc < 6) {
    cout << Usage;
    exit(1);
  }

  int c= atoi(argv[3]);   // number of classes
  int nf = atoi(argv[4]);        // number of features (dimension)
  int cases = atoi(argv[5]);     // used by MPP
 
  // read in data from the data file
  nc = nf+1; // the data dimension; plus the one label column
  Tr = readData(argv[1], nc);
  nrTr = Tr.getRow();   // get the number of rows
  Te = readData(argv[2], nc);
  nrTe = Te.getRow();   // get the number of columns

  // prepare the labels and error count
  Matrix labelMPP(nrTe, 1);  // a col vector to hold result for MPP
  int errCountMPP = 0;       // calcualte error rate for MPP

  // assign prior probability
  Matrix Pw(c, 1);        
  for (int i=0; i<c; i++)
    Pw(i,0) = (float)1/c;   // if assuming equal prior probability

	Matrix output,cm;
	output.createMatrix(nrTe,3);
	cm.createMatrix(c+1,c+1);

  // perform classification
  for (int i=0; i<nrTe; i++) {
    // classify one test sample at a time, get one sample from the test data
    Matrix sample = transpose(subMatrix(Te, i, 0, i, nf-1)); 

    // call MPP to perform classification
    labelMPP(i,0) = mpp(Tr, sample, c, cases, Pw);
    
    // check if the classification result is correct or not
    if (labelMPP(i,0) != Te(i,nf))
      errCountMPP++;
//generating confustion matrix element 
	output(i,0)=i;//sample number
	output(i,1)=Te(i,nf);//actual result
	output(i,2)=labelMPP(i,0);//classification result 
  }
//generate CM
	cm(0,0)=0;
	for (int i=0;i<c;i++)
	{ 	cm(0,i+1)=i;
		cm(i+1,0)=i;
	}


	for(int r=0;r<c;r++){

		for(int i=0;i<c;i++)
		{
		int w=0;
		for(int j=0;j<nrTe;j++)
			{
			if ((output(j,2)==r)&&(output(j,1)==i))
			w++;
			}
		cm(i+1,r+1)=w;
		}
		

	}
	//writeData(output,"output");
	writeData(cm,"cmMPP");

  // calculate accuracy
  cout << "The error rate using MPP is = " << (float)errCountMPP/nrTe << endl;
 
  return 0;
}
      

  
