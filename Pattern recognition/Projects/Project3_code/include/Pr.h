/*
 * pr.h - header file of the pattern recognition library
 *
 * Author: Hairong Qi, ECE, University of Tennessee
 *
 * Date: 01/25/04
 *
 * Please send all your comments to hqi@utk.edu 
 * 
 * Modified:
 *   - 09/24/13: add "const" to the filename parameters to remove warning 
 *               msg in new compilers (Steven Clukey)
 *   - 04/26/05: reorganized for the Spring 2005 classs
 */

#ifndef _PR_H_
#define _PR_H_

#include "Matrix.h"
#include "Estimate.h"

/////////////////////////  
// file I/O
Matrix readData(const char *,            // the file name
                int);                    // the number of columns of the matrix
Matrix readData(const char *,            // the file name
                int,                     // the number of columns
                int);                    // the number of rows (or samples)
Matrix readData(const char *);           // read data file to a matrix with 1 row
void writeData(Matrix &, const char *);  // write data to a file
Matrix readImage(const char *,           // read the image from a file
                 int *,                  // the number of rows (or samples)
                 int *);                 // the number of columns
void writeImage(const char *,            // write the image to a file
                Matrix &,                // the matrix to write
                int,                     // the number of rows
                int);                    // the number of columns


////////////////////////
// distance calculation
double euc(const Matrix &,         // Euclidean distance between two vectors
	   const Matrix &);
double mah(const Matrix &,         // the Mahalanobis distance, input col vec
	   const Matrix &C,        // the covariance matrix
	   const Matrix &mu);      // the mean (a col vector)

double minkowski(const Matrix &x, const Matrix &y, int p); // calculate minkowski distance of order p

// Estimates calculation
Estimate estimateCalculation(const Matrix &train, int c);

////////////////////////
// classifiers

// maximum a-posteriori probability (mpp)
int mpp(const Matrix &train,        // the training set of dimension mx(n+1)
                                    // where the last col is the class label
                                    // that starts at 0
        const Matrix &test,         // one test sample (a col vec), nx1
        const int,                  // number of different classes
	const int,                  // caseI,II,III of the discriminant func
	const Matrix &Pw);          // the prior prob, a col vec


// likelyhood ratio (lr)
int lr(Matrix *mu, Matrix *cov, Matrix &teData, int classes, Matrix &Pw);

// kNN using partial distance
int knnPartDist(Matrix &tr, Matrix &te, int k, int nf, int c, int p);
// kNN without using partial distance
int knn(Matrix &tr, Matrix &te, int k, int nf, int c, int p);
void largesttoback(Matrix &inmtx, Matrix &pos);

// preprocessing 
void normalize(Matrix &tr, Matrix &te, const int nf, const int flag);

// dimension reduction
int pca(Matrix &tr, Matrix &te, const int nf, const float err, const int flag); // dimension reduction using PCA
int fld(Matrix &tr, Matrix &te, int classes, int nf, int flag); // dimension reduction by using FLD

#endif

