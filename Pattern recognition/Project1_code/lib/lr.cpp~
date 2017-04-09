// returns likely class value for the given sample data with possibility of different classess
// it uses derived rule for likelihood ratio after taking log to both sides and then taking all parameters to one side 
// so ratio is now tranformed to substraction such that
// likelihood1-likelihood2>0
// if it's true then sample belongs to 1st class or else it belongs to 2nd class

#include <iostream>
#include <cstdlib>
#include <cmath>
#include "Pr.h"


using namespace std;

int lr(Matrix *mu, Matrix *cov,Matrix &teData, int classes,Matrix &Pw)
{
	// classification supposing zero-one loss
	// the input sample and mean need to be column vectors
	Matrix disc(1,classes);
	
	int likelyClass=0;
	for(int i=0;i<classes;i++)
	disc(0,i)=-0.5*mtod(transpose(teData-mu[i])->*inverse(cov[i])->*(teData-mu[i])) - 0.5*log(det(cov[i])) + log(Pw(i,0));
	
	for(int i=0;i<classes-1;i++)
	{
		if((disc(0,likelyClass)-disc(0,i+1))>0)
		{
			likelyClass=i;
		}
		else
		{
			likelyClass=i+1;
		}
	}
	return likelyClass;
}
