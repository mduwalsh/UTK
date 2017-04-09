
#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdlib>
#include "Matrix.h"           
#include "Pr.h"
 
using namespace std;
 
#define Usage "Usage: ./testUnsup image_file pixel1 pixel2 color_represent clustering_method \n\t image_file: the file name for image in ppm format \n\t pixel1: number of pixel in one dimenion \nt pixel2: number of pixel in other dimension \n\t color_represent: no. of colors to be represented by \n\t clustering_method: 1-> k-means; 2-> winner takes all; 3-> kohonen map \n"

int main(int argc, char **argv) 
{
	// check to see if the number of argument is correct 
	if (argc ^ 6) {
	cout << Usage;     
	exit(1);   
	}
	// define no. of color in a pixel
	//int r=3; // (R,G,B)   

	// read values from argument
	int row=atoi(argv[2]); 
	int col=atoi(argv[3]);     
	int clr=atoi(argv[4]);   
	int c_method=atoi(argv[5]);
	int *rImg,*cImg;
	rImg=&row; 
	cImg=&col; 
	 
	// read in data from the data file	  
	Matrix mTr = readImage(argv[1], rImg, cImg);
	int nrTr = mTr.getRow();   // get the number of rows
	int ncTr=mTr.getCol(); 

	// define arbitrary clr cluster means 
	Matrix C(clr,ncTr);    
	
	// call kmeans to get lookup table that refers each pixel in original pic to updated cluster mean pixel
	Matrix lookup;
	switch(c_method)  
	{
		case 1:
			lookup=kmeans(mTr,C, 255);   
			break; 
		case 2:
			lookup=winnerTakesAll(mTr,C,255);   
			break;  
		case 3:
			lookup=kohonenMap(mTr,C,255, 200,4,4,0.00001);  
	}

	Matrix kTr(nrTr,ncTr), dTr(nrTr,ncTr);
	for(int i=0;i<nrTr;i++)
	{
		for(int j=0;j<ncTr;j++)       
		{  
			kTr(i,j)=C(lookup(i,0),j);  
		} 
	} 
	writeImage("flower2.ppm", kTr, row, col);
	for(int i=0;i<nrTr;i++) 
	{ 
		for(int j=0;j<ncTr;j++)
		{
			dTr(i,j)=mTr(i,j)-kTr(i,j);
		}
	} 
	writeImage("diff_flower_kmeans.ppm", dTr, row, col); 
	
	
  return 0;
}
