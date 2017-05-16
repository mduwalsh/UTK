#include "Matrix.h"
#include "Pr.h"
#include <iostream>
#include <cstdlib>
#include <cmath>

using namespace std;
/** 
  * @param S: data matrix
  * @param C: cluster center matrix
**/
Matrix kohonenMap(Matrix &S, Matrix &C, int max_val, int kmax, int map_width, double sig, double e)
{
	int nrS=S.getRow();
	int ncS=S.getCol();
	int nrC=C.getRow();
	int ncC=C.getCol();
	int d=0;
	Matrix dist(1,nrC);
	Matrix sdist(1,nrC);
	int pos;
	double minD;
	Matrix lookup(nrS,1); // lookup table for sample belonging to which cluster
	Matrix count(nrC,1); // to store count for each cluster center
	Matrix sum(nrC,ncC);
	Matrix g(nrC,ncC);
	int x=0;
	double sigma=1;
	double phi=0;

	// initialize cluster means and map co-ordinates
	for(int i=0;i<nrC;i++)
	{
		for(int j=0;j<ncC;j++)
		{
			C(i,j)=rand()%(max_val+1);
		} 		
	}
	int y=0;
	do{			
		for(int k=0;k<map_width;k++)
		{
			g(y,0)=y;
			g(y,1)=x;
			g(y,2)=k;
			y++;
			if(y>=nrC)
			{
				break;
			}
		}
		if(y<nrC)
			x++;
	}while(y<nrC);
	
	// calculate sigma
	/*double maxD=0;double gdist;
	for(int i=0;i<nrC;i++)
	{
		gdist=pow(g(i,1)-g(0,1),2)+pow(g(i,2)-g(0,2),2);
		if(gdist>maxD)
		{
			maxD=gdist;
		}
	}*/
	sigma=sig;

	//Matrix x1,y1; // for storing points to be use calculate distance
	for(int i=0;i<nrS;i++)
	{
		lookup(i,0)=0;
	}
	bool change;

	// while change of assignment to centers occurs
	do{
		change=false; 
		cout<<d++<<endl;
		// clear values in count and cluster mean
		for(int i=0;i<nrC;i++)
		{
			for(int j=0;j<ncC;j++)
			{
				sum(i,j)=0;
				count(i,0)=0;
			}			
		}

		for(int i=0;i<nrS;i++)
		{
				
			// calculate distance between sample data and each cluster mean 
			minD=0;
			pos=0;
			for(int k=0;k<ncC;k++)
				minD+=(S(i,k)-C(0,k))*(S(i,k)-C(0,k));
			for(int j=0;j<nrC;j++)
			{
				dist(0,j)=0;
				for(int k=0;k<ncC;k++)
					dist(0,j)+=(S(i,k)-C(j,k))*(S(i,k)-C(j,k));	
				// identify positional value of cluster mean that will have minimum distance
				if(dist(0,j)<minD)
				{
					pos=j;
					minD=dist(0,j);
				}			
			}
			
			// if lookup table value for this doesn't matches cluster mean's position then update
			if(((int)lookup(i,0))!=pos)
			{
				change=true;
				lookup(i,0)=pos;				
			}
			for(int j=0;j<nrC;j++)
			{
				phi=exp(-(pow(g(pos,1)-g(j,1),2)+pow(g(pos,2)-g(j,2),2))/(2*pow(sigma,2)));
				// update value all cluster means
				for(int k=0;k<ncC;k++)
				{
					C(j,k)+=e*phi*(S(i,k)-C(j,k)); 
				} 
			}

			// calculate sum of values in each dimension belonging to each cluster
			for(int j=0;j<ncC;j++)
			{
				sum((int)lookup(i,0),j)+=S(i,j);									
			} 
			// increase count for the cluster mean by 1
			count((int)lookup(i,0),0)++;
						
		}
		
		// update mean for each cluster
		/*if(change==true)
		{
	
			// divide sum by count and update mean
			for(int i=0;i<nrC;i++)
			{
				if(count(i,0)!=0)
				{
					for(int j=0;j<ncC;j++)
					{
					
						C(i,j)=(sum(i,j)/count(i,0));
					}					
				}				
			}

		}*/
		
		// update epsilon and sigma
		e=e*0.9;
		sigma=sigma*0.9;
	} while(d<kmax && change==true);
	
	return lookup;

		
}


