#include <stdio.h>
#include <time.h>

int blockSize; 
int gridSize;

__global__ void gameOfLife(int *indata, int *outdata, int width, int height) 
{
	__shared__ int sdata[256];
	int tSize=width*height;
	int x, y, x0,x1,y0,y1, n;
	int bid, cid, tid;
	tid = threadIdx.x;
	bid = blockIdx.x;
	for(cid = blockIdx.x*blockDim.x+tid; cid < tSize; cid += gridDim.x*blockDim.x){
	  if(tid<width){
		  x=(cid) % width; // x position
		  y=(cid) / width; // y position
		  x0=(x-1+width) % width; // one left x positional value assuming circular edge connecting every cell in edges too
		  x1=(x+1) % width; // one right x positional value
		  y0=(y-1+height) % height; // one row up y positional value
		  y1=(y+1) % height; // one row below y positional value
		  n=indata[y1*width+x0]+indata[y*width+x0]+indata[y0+x0]+indata[y0*width+x]+indata[y0*width+x1]+indata[y*width+x1]+indata[y1*width+x1]+indata[y1*width+x]; // no. of alive neighbor cells
		  
		  if(n==3 || (n==2 && indata[cid]))  // life created or sustains
		  {
			  sdata[tid]=1;
			  //outdata[cid]=1;
		  }
		  else  // life destroyed or stay died
		  {
			  sdata[tid]=0;
			  //outdata[cid]=0;
		  }						
		  __syncthreads();
		  outdata[cid]=sdata[tid];
	  }	  	  
	}
}

int main(int argc, char **argv)
{
	if(argc ^ 4){
	  printf("Usage: ./gol seed height width iteration\n");
	  exit(1);
	}
	clock_t sTime=clock();
	int width= atoi(argv[3]);
	int height=atoi(argv[2]);
	int dsize=width*height;	
	int iteration=atoi(argv[3]);
	int i, j;

	int *data;
	int *d_indata, *d_outdata, *temp;

	// allocate memory for data in host 
	data=(int *)malloc(dsize*sizeof(int));
	
	// allocate memory for data in device
	cudaMalloc(&d_indata, dsize*sizeof(int));
	cudaMalloc(&d_outdata, dsize*sizeof(int));
	
	srand(atoi(argv[0]));                                   // test seed
	// initialize data in host as randomly 0 or 1
	for(i=0;i<dsize;i++)
	{
		data[i]=rand()%2;
	}

	// copy initialized data to gpu device
	cudaMemcpy( d_indata, data, dsize, cudaMemcpyHostToDevice ); 

	for(i=0;i<iteration;i++)
	{
		// call kernel
		gameOfLife<<<gridSize, blockSize>>>(d_indata, d_outdata, width, height);
		// synchronize between thread blocks
		cudaDeviceSynchronize();
		// swap d_indata and d_outdata for next iteration
		temp=d_indata;
		d_indata=d_outdata;
		d_outdata=temp;
	}

	// copy data back from device to host memory	
	cudaMemcpy( data, d_indata, dsize, cudaMemcpyDeviceToHost); 
	
	// diplay cell status in console
	printf("\n board status # \n");
	for(i=0;i<height;i++)
	{
		for(j=0;j<width;j++)
		{
			printf(" %d",data[i*width+j]);
		}
		printf("\n");
	}
	
	// free memory allocated
	cudaFree(d_indata);
	cudaFree(d_outdata);
	free(data);
	clock_t eTime=clock();
	printf("time taken = %ld",eTime-sTime);
	cudaDeviceReset();
	return 0;
}
