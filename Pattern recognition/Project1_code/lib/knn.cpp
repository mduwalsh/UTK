#include <iostream>
#include <cstdlib>
#include <cmath>
#include "Pr.h"


using namespace std;
/*
	tr: training set data
	te: testing sample data
	k: size of neighbours to be considered
	nf: no. of features
	c: no. of classes
	p: minkowski distance order ; 2 equivalents to euclidean distance
*/

int knn(Matrix &tr, Matrix &te, int k, int nf, int c, int p)
{
	// the input sample needs to be row vectors
	int nr= tr.getRow();
	Matrix dist(1,nr);
	Matrix sdist(1,nr);
	Matrix pos(1,nr);
	Matrix voting(1,c);
	// calculate distance between sample data and each trainging sample
	for(int i=0;i<nr;i++)
	{
		dist(0,i)=minkowski(subMatrix(tr,i,0,i,nf-1), te, p);
	}
	// sort the dist in ascending order
	insertsort(dist,sdist,pos);

	//initialize voting
	for(int i=0;i<c;i++)
	{
		voting(0,i)=0;
	}
	// do voting
	for(int i=0;i<k;i++)
	{
		voting(0,tr(pos(0,i),nf))++;
	}
	Matrix sVoting(1,c), vPos(1,c);
	// sort the voting in ascending order
	insertsort(voting,sVoting,vPos);
	// return the label of largest voting
	return (int)vPos(0,c-1);
}

// kNN using partial distance
int knnPartDist(Matrix &tr, Matrix &te, int k, int nf, int c, int p)
{
	// the input sample needs to be row vectors
	int nr= tr.getRow();
	double pDist;
	Matrix dist(1,k);
	Matrix sdist(1,k);
	Matrix pos(1,k);	
	Matrix voting(1,c);
	// calculate distance^p between sample data and 1st k training samples
	for(int i=0;i<k;i++)
	{
		dist(0,i)=pow(minkowski(subMatrix(tr,i,0,i,nf-1), te, p),p);
		pos(0,i)=i;
	}
	// put max distance to the end of array
	largesttoback(dist, pos);
	// now for all other samples from k+1th to end of training samples, calculate partial distance 
	// and check with max of above k distances and if greater no need to progress calculation for that point and move to another
	bool invalid=0;
	for(int i=k;i<nr;i++)
	{	
		pDist=0;invalid=0;
		for(int j=0;j<nf;j++)
		{
			pDist+=pow(fabs(tr(i,j)-te(0,j)), p);
			if(pDist>dist(0,k-1))
			{
				invalid=1;
				break;
			}
		}
		if(invalid==0) // if pDist as total less than max distance in dist array, then replace it 
		{
			dist(0,k-1)=pDist;
			pos(0,k-1)=i;
			largesttoback(dist, pos); // again calculate max and put it to the end
		}
	}
	//initialize voting
	for(int i=0;i<c;i++)
	{
		voting(0,i)=0;
	}
	// do voting
	for(int i=0;i<k;i++)
	{
		voting(0,tr(pos(0,i),nf))++;
	}
	Matrix sVoting(1,c), vPos(1,c);
	// sort the voting in ascending order
	insertsort(voting,sVoting,vPos);

	// return the label of largest voting
	return (int)vPos(0,c-1);
}

// brings largest value to the back of the array and also does same to the corresponding original position
void largesttoback(Matrix &inmtx, Matrix &pos)
{
	int nr, nc,temp2;
	double temp1;
	nr = inmtx.getRow();
	nc = inmtx.getCol();

	if (nr>1) {
	cout << "INSERTSORT: The matrix to be sorted needs to be a row vector\n";
	exit(3);
	}
	for(int i=0;i<nc-1;i++)
	{
		if(inmtx(0,i)>inmtx(0,i+1))
		{
			temp1=inmtx(0,i+1);
			inmtx(0,i+1)=inmtx(0,i);
			inmtx(0,i)=temp1;
			temp2=pos(0,i+1);
			pos(0,i+1)=pos(0,i);
			pos(0,i)=temp2;
		}
	}
}




