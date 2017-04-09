#include "Matrix.h"
#include "Pr.h"
#include <iostream>
#include <cstdlib>
#include <cmath>

using namespace std;
/** 
  * @param 
  * @param 
**/

// works for three classifiers only
Matrix bks(Matrix &T, Matrix &S1,Matrix &S2,Matrix &S3, int c)
{
	int nr=T.getRow();
	Matrix F(pow(c,3),3+c+1); // 3 cols for classifier result combination; c cols for no. of samples of one of c classes as truth; 1 col for fused label
	Matrix d(1,c);
	Matrix s(1,c);
	Matrix pos(1,c);
	int r=0;
	for(int i=0;i<c;i++)
	{
		for(int j=0;j<c;j++)
		{
			for(int k=0;k<c;k++)
			{
				F(r,0)=i;F(r,1)=j;F(r,2)=k;
				for(int l=0;l<nr;l++)
				{
					if(S1(l,0)==i && S2(l,0)==j && S3(l,0)==k)
					{
						F(r,T(l,0)+3)+=1;
					}
					d=subMatrix(F,r,3,r,3+c-1);
					insertsort(d,s,pos);
					F(r,3+c)=pos(0,c-1);					
				}
				r++;				
			}
		}
	}
	return F;
}


