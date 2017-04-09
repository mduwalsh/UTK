#include <stdio.h>
#include <time.h>


int blockSize=256; 
int gridSize=256;

__global__ void gameOfLife(int *indata, int *outdata, int width, int height) 
{
	__shared__ int sodata[2566];
	__shared__ int sidata[256*3];
	int tSize=width*height;
	int x, y, x0,x1,y0,y1, n;
	int tid;
	int bid;
	int cid;
	for(bid=0;bid<gridDim.x;bid++)
	{
		tid=threadIdx.x;
		cid=bid*blockDim.x+threadIdx.x;
		if(tid<width)
		{
			
			y=(blockIdx.x*blockDim.x+threadIdx.x) / width; // y position
			y0=(y-1+height) % height; // one row up y positional value
			y1=(y+1) % height; // one row below y positional value
			// load into shared memory
			sidata[tid]=indata[y0*width+tid]; // upper row
			sidata[width+tid]=indata[y*width+tid]; // computing cell containing row
			sidata[2*width+tid]=indata[y1*width+tid]; // lower row		
				
		}
		__syncthreads();
		
	}

	for(bid=0;bid<height;bid++)
	{
		if(threadIdx.x<width)
		{
			x=(bid*blockDim.x+threadIdx.x) % width; // x position
			x0=(x-1+width) % width; // one left x positional value assuming circular edge connecting every cell in edges too
			x1=(x+1) % width; // one right x positional value
			
			n=sidata[2*width+x0]+sidata[width+x0]+sidata[x0]+sidata[x]+sidata[x1]+sidata[width+x1]+sidata[2*width+x1]+sidata[2*width+x]; // no. of alive neighbor cells
			if(n==3 || (n==2 && sidata[width+x]==1))
			{
				sodata[threadIdx.x]=1;
			}
			else
			{
				sodata[threadIdx.x]=0;
			}
			
		}	
		__syncthreads();
		//cid+=gridDim.x*blockDim.x;	
	}
	__syncthreads();
	outdata[cid]=sodata[tid];
	__syncthreads();
}

int main()
{
	clock_t sTime=clock();
	
	int width=256;
	int height=256;
	int dsize=width*height;	
	int iteration=1;
	int i, j;

	int *data;
	int *d_indata, *d_outdata, *temp;

	// allocate memory for data in host 
	data=(int *)malloc(dsize*sizeof(int));
	// allocate memory for data in device
	cudaMalloc(&d_indata, dsize*sizeof(int));
	cudaMalloc(&d_outdata, dsize*sizeof(int));

	// initialize data in host as randomly 0 or 1
	for(i=0;i<dsize;i++)
	{
		data[i]=rand()%2;
	}

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
