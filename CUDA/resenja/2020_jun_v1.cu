#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <stdio.h>
#include <math.h>

#define N 64
#define BLOCKSIZE 4

__global__ void kernel_poz_neg(int *A, int *C);
__global__ void kernel_add(int *C, int *B);
__global__ void kernel_zam(int *A, int *B);

void mainjun2020()
{
	int A[N][N], B[N], *A_dev, *B_dev, *C_dev, C[N][N / (2 * BLOCKSIZE)];

	cudaMalloc((void **)&A_dev, N * N * sizeof(int));
	cudaMalloc((void **)&B_dev, N * sizeof(int));
	cudaMalloc((void **)&C_dev, N * (N / (2 * BLOCKSIZE)) * sizeof(int));

	for (int i = 0; i < N; i++)
		for (int j = 0; j < N; j++)
			A[i][j] = i + j - 3;

	for (int i = 0; i < N; i++)
	{
		B[i] = 0;
		for (int j = 0; j < N; j++)
			if (A[i][j] > 0)
				B[i]++;
			else if (A[i][j] < 0)
				B[i]--;
		printf("%d, ", B[i]);
	}
	printf("\n\n");

	cudaMemcpy(A_dev, A, N * N * sizeof(int), cudaMemcpyHostToDevice);

	dim3 grid1(N / (2 * BLOCKSIZE), N / BLOCKSIZE);
	dim3 block1(BLOCKSIZE, BLOCKSIZE);
	kernel_poz_neg<<<grid1, block1>>>(A_dev, C_dev);

	cudaMemcpy(C, C_dev, N * (N / (2 * BLOCKSIZE)) * sizeof(int), cudaMemcpyDeviceToHost);
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N / (2 * BLOCKSIZE); j++)
			printf("%d, ", C[i][j]);
		printf("\n");
	}
	printf("\n");

	dim3 grid2(1, N / BLOCKSIZE);
	dim3 block2(N / (4 * BLOCKSIZE), BLOCKSIZE);
	kernel_add<<<grid2, block2>>>(C_dev, B_dev);

	cudaMemcpy(B, B_dev, N * sizeof(int), cudaMemcpyDeviceToHost);
	for (int i = 0; i < N; i++)
		printf("%d, ", B[i]);
	printf("\n\n");

	dim3 grid3(N / BLOCKSIZE, N / BLOCKSIZE);
	dim3 block3(BLOCKSIZE, BLOCKSIZE);
	kernel_zam<<<grid2, block3>>>(A_dev, B_dev);

	cudaMemcpy(A, A_dev, N * N * sizeof(int), cudaMemcpyDeviceToHost);
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N; j++)
			printf("%d, ", A[i][j]);
		printf("\n");
	}
	printf("\n");

	cudaFree(A_dev);
	cudaFree(B_dev);
	cudaFree(C_dev);
}

__device__ int getval(int val)
{
	if (val < 0)
		return -1;
	else if (val > 0)
		return 1;
	return val;
}

__global__ void kernel_poz_neg(int *A, int *C)
{
	int x = threadIdx.x + blockIdx.x * blockDim.x;
	int y = threadIdx.y + blockIdx.y * blockDim.y;

	__shared__ int loc[BLOCKSIZE][BLOCKSIZE];
	loc[threadIdx.y][threadIdx.x] = getval(A[y * N + x]) + getval(A[y * N + x + N / 2]);
	__syncthreads();

	int len = BLOCKSIZE;
	while (len > 1)
	{
		if (threadIdx.x < len / 2)
			loc[threadIdx.y][threadIdx.x] += loc[threadIdx.y][threadIdx.x + len / 2];

		len /= 2;
		__syncthreads();
	}

	if (threadIdx.x == 0)
		C[y * N / (2 * BLOCKSIZE) + blockIdx.x] = loc[threadIdx.y][0];
}

__global__ void kernel_add(int *C, int *B)
{
	int x = threadIdx.x + blockIdx.x * blockDim.x;
	int y = threadIdx.y + blockIdx.y * blockDim.y;

	__shared__ int loc[BLOCKSIZE][N / (4 * BLOCKSIZE)];
	loc[threadIdx.y][threadIdx.x] = C[y * 2 * gridDim.x * blockDim.x + x] + C[y * 2 * gridDim.x * blockDim.x + x + gridDim.x + blockDim.x];
	__syncthreads();

	int len = blockDim.x;
	while (len > 1)
	{
		if (threadIdx.x < len / 2)
			loc[threadIdx.y][threadIdx.x] += loc[threadIdx.y][threadIdx.x + len / 2];

		len /= 2;
		__syncthreads();
	}

	if (threadIdx.x == 0)
		B[y] = loc[threadIdx.y][0];
}

__global__ void kernel_zam(int *A, int *B)
{
	int x = threadIdx.x + blockIdx.x * blockDim.x;
	int y = threadIdx.y + blockIdx.y * blockDim.y;

	int val = A[y * N + x];
	if (val < 0)
		A[y * N + x] = B[y];
}