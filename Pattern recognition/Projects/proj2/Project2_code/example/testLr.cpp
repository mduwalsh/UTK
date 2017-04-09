#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdlib>
#include "Matrix.h"           
#include "Pr.h"

using namespace std;

#define Usage "Usage: ./testLr training_set test_set classes features \n\t training_set: the file name for training set\n\t test_set: the file name for test set\n\t classes: number of classes\n\t features: number of features (dimension)\n\t"

int main(int argc, char **argv)
{
  int nrTr, nrTe,        // number of rows in the training and test set
    nc;                  // number of columns in the data set; both the 
                         // training and test set should have the same
                         // column number                         
  Matrix Tr, Te;

  // check to see if the number of argument is correct
  if (argc < 5) {
    cout << Usage;
    exit(1);
  }

  int classes = atoi(argv[3]);   // number of classes
  int nf = atoi(argv[4]);        // number of features (dimension)
   
  // read in data from the data file
  nc = nf+1; // the data dimension; plus the one label column
  Tr = readData(argv[1], nc);
  nrTr = Tr.getRow();   // get the number of rows
  Te = readData(argv[2], nc);
  nrTe = Te.getRow();   // get the number of columns

  // prepare the labels and error count
  Matrix labelLR(nrTe, 1);  // a col vector to hold result for LR
  int errCountLR = 0;       // calcualte error rate for MPP

  // assign prior probability
  Matrix Pw(classes, 1);       
  for (int i=0; i<classes; i++)
    Pw(i,0) = (float)1/classes;   // if assuming equal prior probability
	//Pw(0,0)=0.8;Pw(1,0)=1-Pw(0,0); // to change probability for class 0 and 1 in two class case

	// calculate Estimates
	Estimate est=estimateCalculation(Tr,classes);
	Matrix *mean=est.getMean();
	Matrix *cov=est.getCovariance();
	//est.calculateEstimates(Tr,classes);

// to test two modal performance
	/*mean[0](0,0)=-0.75;mean[0](1,0)=0.2;
	cov[0](0,0)=0.25;cov[0](0,1)=0;cov[0](1,0)=0;cov[0](1,1)=0.3;
	mean[1](0,0)=0.3;mean[1](1,0)=0.3;
	cov[1](0,0)=0.1;cov[1](0,1)=0;cov[1](1,0)=0;cov[1](1,1)=0.1;
	Pw(0,0)=0.8;Pw(1,0)=0.2;*/

  // perform classification 
  for (int i=0; i<nrTe; i++) {
	// classify one test sample at a time, get one sample from the test data
	Matrix sample = transpose(subMatrix(Te, i, 0, i, nf-1)); 
	// get the size of input raw data
	int nctr = Tr.getCol();
	int nrte = sample.getRow();

	if (nctr != (nrte+1)) {
	cout << "MPP: "
	   << "Training and testing set do not have same number of features\n";
	exit(3);
	}	

	// call lr to perform classification
	labelLR(i,0) = lr(mean, cov, sample, classes, Pw);

	

	// check if the classification result is correct or not
	if (labelLR(i,0) != Te(i,nf))
	errCountLR++;
  }

  // calculate accuracy 
  cout << "The error rate using Likelihood ratio is = " << (float)errCountLR/nrTe << endl;

  return 0;
}
      
