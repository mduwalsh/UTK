
// definition of member functions of class Estimate


#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <iomanip>
#include "Matrix.h"
#include "Estimate.h"
using namespace std;

	// empty constructor
	Estimate::Estimate()
	{
		means = (Matrix *) new Matrix [0];    
    		covs = (Matrix *) new Matrix [0];
		nf=0;
		ncl=0;
	}
	// parameterized constructor
	Estimate::Estimate(int nft,int c)
	{
		nf=nft;
		ncl=c;
		means=(Matrix *) new Matrix[ncl];
		for (int i=0; i<ncl; i++)
      		means[i].createMatrix(nf, 1);
		covs=(Matrix *) new Matrix[ncl];
		for (int i=0; i<ncl; i++)
      		covs[i].createMatrix(nf, nf);
	}
	// destructor
	Estimate::~Estimate()
	{
		if(means){delete [] means;}
		if(covs){delete [] covs;}
	}
	// getter for means which is array or pointer to mean matrix
	Matrix * Estimate::getMean()
	{
		return means;
	}
	// setter for means
	void Estimate::setMean(Matrix *mean)
	{			
		for(int i=0;i<ncl;i++)		
		means[i]=mean[i];
	}
	// getter for covs
	Matrix * Estimate::getCovariance()
	{
		return covs;
	}
	// setter for covs
	void Estimate::setCovariance(Matrix *cov)
	{
		for(int i=0;i<ncl;i++)		
		covs[i]=cov[i];		
	}

// alternate way to calculate estimates
	void Estimate::calculateEstimates(const Matrix &train, int c)
	{
		double varavg;
		Matrix tmp;
	  	int nctr;
		int nf,i, j;  	

	// get the size of input raw data
	    nctr = train.getCol();    
	    nf = nctr-1;

	
	    // the mean is an array of nfx1 matrix of size c and the cov is an array of nf x nf matrix of size c
	    means = (Matrix *) new Matrix [c];
	    for (i=0; i<c; i++)
	      means[i].createMatrix(nf, 1);
	    covs = (Matrix *) new Matrix [c];
	    for (i=0; i<c; i++)
	      covs[i].createMatrix(nf, nf);
	
		// calculate the mean and covariance

	    for (i=0; i<c; i++) {
	      tmp = getType(train, i);
	      covs[i] = cov(tmp, nf);
	      means[i] = mean(tmp, nf);
	      
	    }
	
	
	}	



