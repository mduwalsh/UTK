#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>


#define LEFT(i) ((i<<1) + 1)          /* left leaf of node i */
#define RIGHT(i) ((i<<1) + 2)     /* right leaf of node i */

int ns, nh;  // global variable for counting numbers of comparisons for selection sort and heap sort
int cp = -1;

void MAX_HEAPIFY(int *A, int hs, int i)
{
  int l, r, largest, temp;
  l = LEFT(i);
  r = RIGHT(i);
  nh++;                            // increase counter for heapsort comparison
  if(l < hs && A[l] > A[i])
    largest = l;
  else
    largest = i;
   nh++;                            // increase counter for heapsort comparison
   if(r < hs && A[r] > A[largest])
    largest = r;
  if(largest != i){              // if largest is not ith element, swap with largest element 
    temp       = A[i];              
    A[i]       = A[largest];
    A[largest] = temp;
    MAX_HEAPIFY(A, hs, largest); // and call self recursive MAX_HEAPIFY() on previous index of largest element 
  }
}

void BUILD_MAX_HEAP(int *A, int al)
{
  int i, hs;
  hs = al;
  for(i = ((al>>1) - 1); i >= 0; i--){         // from half of array length al substract 1 to 0;
    MAX_HEAPIFY(A, hs, i);
  }
} 

void HEAPSORT(int *A, int al)
{
  int i, hs, temp;
  hs = al;
  BUILD_MAX_HEAP(A, al);
  for(i = al-1; i > 0; i--){    // from last index to second index in the array
    temp = A[i];
    A[i] = A[0];
    A[0] = temp;     // done swapping element in root A[1] to A[i] its final position
    hs   = hs - 1;     // decrease heap size by 1
    MAX_HEAPIFY(A, hs, 0);   // call max heapify for root 1
  }
}

void EXCHANGE(int *A, int i, int j)
{
	int t;
	t = A[j];
	A[j] = A[i];
	A[i] = t;
}

void COMAPRE_EXCHANGE(int *A, int i, int j)
{
  int t;
  if(A[i] > A[j]){
    t = A[j];
    A[j] = A[i];
    A[i] = t;
  }
}

void INSERTSORT(int *A, int s, int e)
{
  int i, j;
  for(i = s+1; i < e+1; i++)
    for(j = i-1; j > s-1; j--)
      COMAPRE_EXCHANGE(A, j, j+1);	
}

int MODIFIED_PARTITION(int *A, int p, int r, int x)
{
  int i, j;
  for(j = p; j < r; j++)
    if(A[j] == x){
      EXCHANGE(A, j, r);
      break;
    }
  i = p-1;
  for(j = p; j < r+1; j++){
    if(A[j] < x){
      i = i+1;
      EXCHANGE(A, i, j);
    }		
  }	
  EXCHANGE(A, i+1, r);
  return i+1;	
}

int SELECT(int *A, int s, int e, int i)
{
  int g, *medians;
  int j, k, h, pivot;
  int n = e-s+1; 
  medians = malloc((n+4)/5 * sizeof(int));  // no. of medians will be integer division of (n+4)/5
  
  for (j = 0; j < n/5; j++){
    INSERTSORT(A, s+j*5, s+j*5+4);
    medians[j] = A[s+j*5+2];
  }            
  if(j*5 < n){
    INSERTSORT(A, s+j*5, (n%5)-1);
    medians[j] = A[s+j*5 + ((n%5)-1)/2]; 
    j++;
  } 
  if(j == 1){
    pivot = medians[j-1];
  }
  else{
    pivot = SELECT(medians, 0, j-1, j/2);
  }
  
  k = MODIFIED_PARTITION(A, s, e, pivot); 	
  
  if (k-s == i-1){
    free(medians);
    return A[k];
  }
  if (k-s > i-1)  // If position is more, recur for left
    return SELECT(A, s, k-1, i);
  
  // Else recur for right subarray
  return SELECT(A, k+1, e, i-k+s-1);  
}


// plot data using lines for columns as y axis data
void plot(FILE *gnuplotPipe, int wid, char *datafile, int datacolumn, char *title, char *xlabel, char *ylabel, char *outputfile )
{
  fprintf(gnuplotPipe, "set key outside \n");        
  fprintf(gnuplotPipe, "set term x11 %d \n", wid);
  fprintf(gnuplotPipe, "set title '%s' \n", title);
  fprintf(gnuplotPipe, "set xlabel '%s' \n", xlabel);
  fprintf(gnuplotPipe, "set ylabel '%s' \n", ylabel);
  // fprintf(gnuplotPipe, "set object rectangle at first %d,0 size char 1, char 1 fillcolor rgb 'blue' fillstyle solid border lt 2 lw 2 \n", cp);
  fprintf(gnuplotPipe, "plot for [col=2:%d] '%s' using 1:col with lines title columnheader \n", datacolumn, datafile);
  if(strcmp(outputfile,"")!=0){
    fprintf(gnuplotPipe, "set term png size 1024,720 enhanced font \"Helvetica,20\" \n");
    fprintf(gnuplotPipe, "set output '%s' \n", outputfile);
    fprintf(gnuplotPipe, "replot \n");
  }
}


int main(int argc, char **argv)
{
  if(argc ^ 2){
    printf("Usage: ./sort filename \n");
    exit(1);
  }
  int i, j, h, k;
  int *A, *B;
  FILE *fp;
  int N = 20;                    // no. of points for plotting
  clock_t s, e;
  fp    = fopen(argv[1], "w");
  k     = 10;                    // intial length of array

  fprintf(fp, "%s\t%s\t%s\n", "arraylength", "heapsort", "kth_order_stastics");
  for(h = 0; h < N; h++){    // test for N number of times
    A = malloc(k*sizeof(int));
    B = malloc(k*sizeof(int));
    i = k;
    while(i--){   // initializing arrays A and B with same random values
      A[i] = rand()%(10000);
      B[i] = A[i];
    }
    s = clock();
    HEAPSORT(A, k);  
    e = clock();
    fprintf(fp, "%d\t", k/1000);
    fprintf(fp, "%.4lf\t", (e-s)/(double)CLOCKS_PER_SEC);
    s = clock();
    j = SELECT(B, 0, k-1, rand()%(k/2));
    e = clock();
    fprintf(fp," %.4lf\n",(e-s)/(double)CLOCKS_PER_SEC);
    free(A);
    free(B);  
    k = k*2;
    printf("%d\n", h);
  }
  fclose(fp); 
  FILE * gnuplotPipe = popen ("gnuplot -persistent", "w"); // open gnuplot in persistent mode
  plot(gnuplotPipe, 0, argv[1], 3, "heap sort vs selection sort", "no. of elements in thousands", "runtime in sec", "heap_kthorder_select.png");  
  fflush(gnuplotPipe); 
  pclose(gnuplotPipe);
  
  return 0;
}

/*
  gcc -o select select.c
  ./select outfile.txt
*/

