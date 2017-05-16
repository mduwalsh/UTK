#include <stdio.h>
#include <stdlib.h>
#include <time.h>


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

void SELECTIONSORT(int *A, int al)
{
  int i, j, min, temp;
  for(j = 0; j < al-1; j++) {
    min = j;    
    for(i = j+1; i < al; i++) {    // find the smallest 
      ns++;                    // increase counter for comparison 
      if (A[i] < A[min]) {
	min = i;
      }
    } 
    if(min != j) {
      // swap elements at min and j
      temp   = A[min];
      A[min] = A[j];
      A[j]   = temp;
    }
    
  }
}

// plot data using lines for columns as y axis data
void plot(FILE *gnuplotPipe, int wid, char *datafile, int datacolumn, char *title, char *xlabel, char *ylabel, char *outputfile )
{
  fprintf(gnuplotPipe, "set key outside \n");        
  fprintf(gnuplotPipe, "set term x11 %d \n", wid);
  fprintf(gnuplotPipe, "set title '%s' \n", title);
  fprintf(gnuplotPipe, "set xlabel '%s' \n", xlabel);
  fprintf(gnuplotPipe, "set ylabel '%s' \n", ylabel);
  fprintf(gnuplotPipe, "set object rectangle at first %d,0 size char 1, char 1 fillcolor rgb 'blue' fillstyle solid border lt 2 lw 2 \n", cp);
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
  int N = 30;
  fp    = fopen(argv[1], "w");
  k     = 1;                    // intial length of array

  fprintf(fp, "%s\t%s\t%s\n", "arraylength", "heapsort", "selectionsort");
  for(h = 0; h < N; h++, k++){    // test for N number of times
    ns = 0; nh = 0;
    A = malloc(k*sizeof(int));
    B = malloc(k*sizeof(int));
    i = k;
    while(i--){   // initializing arrays A and B with same random values
      A[i] = rand()%(1000);
      B[i] = A[i];
    }
    HEAPSORT(A, k);  
    fprintf(fp, "%d\t", k);
    fprintf(fp, "%d\t", nh);
    SELECTIONSORT(B, k);
    printf("%d \t%d\t %d\n", k, ns, nh);
    fprintf(fp," %d\n",ns);
    if(cp == -1 && nh < ns)
      cp = k;
    free(A);
    free(B);      
  }
  fclose(fp); 
  FILE * gnuplotPipe = popen ("gnuplot -persistent", "w"); // open gnuplot in persistent mode
  plot(gnuplotPipe, 0, argv[1], 3, "heap sort vs selection sort", "no. of elements", "runtime", "heap_select_sort.png");  
  fflush(gnuplotPipe); 
  pclose(gnuplotPipe);
  
  return 0;
}

/*
  gcc -o sort sort.c
  ./sort outfile.txt
*/


