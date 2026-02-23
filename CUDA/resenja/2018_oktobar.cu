#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <stdio.h>
#include <cmath>

#define threadsPerBlock 10
#define tile 5

__global__ void findMax(int *a, int *b, int n)
{
	int blockId = blockIdx.x;
	int threadId = blockIdx.x * threadsPerBlock + threadIdx.x;
	__shared__ int max;
	int localMax = -100;

	__shared__ int shred[threadsPerBlock * tile];

	int start = blockId * threadsPerBlock;
	int end = start + threadsPerBlock * tile;
	int j = 0;

	max = INT_MIN;

	if (threadIdx.x == 0)
	{
		for (int i = start; i < end; i++)
		{
			if (i < n)
			{
				shred[j++] = a[i];
				// printf("blockId = %d, shNiz[%d] = %d\n", blockIdx.x, i, shred[j-1]);
			}
		}
	}

	if (threadIdx.x == 0)
		for (int i = 0; i < threadsPerBlock * tile; i++)
		{
			// printf("blockId = %d, shNiz[%d] = %d\n", blockIdx.x, i, shred[i]);
		}

	__syncthreads();

	for (int i = 0; i < tile; i++)
	{
		int index = threadIdx.x * tile + i;
		if (index < n && shred[index] > max)
		{
			max = shred[index];
			__syncthreads();
		}
	}
	printf("threadIdx.x = %d, blockIdx.x = %d, localMax = %d, max = %d, shred[0] = %d \n", threadIdx.x, blockIdx.x, localMax, max, shred[0]);

	if (shred[threadIdx.x] < max)
		shred[threadIdx.x] = max;

	__syncthreads();
	// if( threadIdx.x == 0)
	//	printf("threadIdx.x = %d, blockIdx.x = %d, max = %d, \n", threadIdx.x, blockIdx.x, max);

	if (threadIdx.x == 0)
		for (int i = 0; i < blockDim.x; i++)
			printf("shred[%d] = %d", i, shred[i]);

	// printf("threadIdx.x = %d, blockIdx.x = %d, localMax = %d, max = %d, shred[0] = %d \n", threadIdx.x, blockIdx.x, localMax, max, shred[0]);

	// if (b[0] < localMax)
	b[0] = max;

	__syncthreads();
}

void HANDLE_ERROR(cudaError_t call)
{
	cudaError_t ret = call;
	switch (ret)
	{
	case cudaSuccess:
		break;
	case cudaErrorInvalidValue:
		printf("ERROR: InvalidValue:%i.\n", __LINE__);
		exit(-1);
		break;
	case cudaErrorInvalidMemcpyDirection:
		printf("ERROR:Invalid memcpy direction:%i.\n", __LINE__);
		exit(-1);
		break;
	default:
		printf("ERROR>line:%i.%d' � %s\n", __LINE__, ret, cudaGetErrorString(ret));
		exit(-1);
		break;
	}
}
int main(int argc, char *argv[])
{
	int *elements, *tmpArray, dim;
	int *cudaElements, *cudaTmpArray;

	printf("Unesite dimenziju niza: ");
	scanf("%d", &dim);

	elements = (int *)malloc(dim * sizeof(int));
	tmpArray = (int *)malloc(dim * sizeof(int));

	printf("\nNiz izgleda ovako:\n");
	for (int i = 0; i < dim; i++)
	{
		elements[i] = i * 10 + 15;
		printf("%5d", elements[i]);
	}

	HANDLE_ERROR(cudaMalloc((void **)&cudaElements, dim * sizeof(int)));
	HANDLE_ERROR(cudaMalloc((void **)&cudaTmpArray, dim * sizeof(int)));

	int dimGrid = dim / (tile * threadsPerBlock);
	if (dim % (tile * threadsPerBlock) > 0)
		dimGrid++;

	int dimBlock = threadsPerBlock;

	HANDLE_ERROR(cudaMemcpy(cudaElements, elements, dim * sizeof(int), cudaMemcpyHostToDevice));

	int dimCopy;
	int arrayLength = dim;

	printf("here");
	// do
	//{
	findMax<<<dimGrid, dimBlock>>>(cudaElements, cudaTmpArray, arrayLength);

	dimCopy = arrayLength;
	arrayLength = arrayLength / tile;
	if (dimCopy % tile > 0)
		arrayLength++;

	printf("\narrLength: %d\n", arrayLength);

	HANDLE_ERROR(cudaMemcpy(cudaElements, cudaTmpArray, arrayLength * sizeof(int), cudaMemcpyDeviceToDevice));
	/*} while (arrayLength > 1);*/

	HANDLE_ERROR(cudaMemcpy(elements, cudaElements, sizeof(int), cudaMemcpyDeviceToHost));
	printf("\nizaso\n");
	printf("Maximum je: %d.\n", elements[0]);

	for (int i = 0; i < dim; i++)
	{
		printf("%d\t", elements[i]);
	}

	free(elements);
	HANDLE_ERROR(cudaFree(cudaElements));
}
