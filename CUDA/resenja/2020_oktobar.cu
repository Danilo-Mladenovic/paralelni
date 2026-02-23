#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <stdio.h>
#include <math.h>

#define N 32
#define BLOCKSIZE 4

__global__ void kernel_mnozenje(int *A, int *X);
__global__ void kernel_sabiranje(int *A, int *B);

void mainokt2020()
{
	int A[N][N], X[N], B[N], *A_dev, *X_dev, *B_dev;

	cudaMalloc((void **)&A_dev, N * N * sizeof(int));
	cudaMalloc((void **)&X_dev, N * sizeof(int));
	cudaMalloc((void **)&B_dev, N * sizeof(int));

	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N; j++)
			A[i][j] = i + j;
		X[i] = i + 2;
	}

	for (int i = 0; i < N; i++)
	{
		B[i] = 0;
		for (int j = 0; j < N; j++)
			B[i] += A[i][j] * X[j];
		printf("%d, ", B[i]);
	}
	printf("\n\n");

	cudaMemcpy(A_dev, A, N * N * sizeof(int), cudaMemcpyHostToDevice);
	cudaMemcpy(X_dev, X, N * sizeof(int), cudaMemcpyHostToDevice);

	dim3 grid1(N / (2 * BLOCKSIZE), N / BLOCKSIZE);
	dim3 block1(BLOCKSIZE, BLOCKSIZE);

	kernel_mnozenje<<<grid1, block1>>>(A_dev, X_dev);

	cudaMemcpy(A, A_dev, N * N * sizeof(int), cudaMemcpyDeviceToHost);
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N / (2 * BLOCKSIZE); j++)
			printf("%d, ", A[i][j]);
		printf("\n");
	}
	printf("\n");

	dim3 grid2(1, N / BLOCKSIZE);
	dim3 block2(N / (4 * BLOCKSIZE), BLOCKSIZE);

	kernel_sabiranje<<<grid2, block2>>>(A_dev, B_dev);

	cudaMemcpy(B, B_dev, N * sizeof(int), cudaMemcpyDeviceToHost);

	for (int i = 0; i < N; i++)
		printf("%d, ", B[i]);
	printf("\n\n");

	cudaFree(A_dev);
	cudaFree(B_dev);
	cudaFree(X_dev);
}

__global__ void kernel_mnozenje(int *A, int *X)
{
	int x = threadIdx.x + blockIdx.x * blockDim.x;
	int y = threadIdx.y + blockIdx.y * blockDim.y;

	__shared__ int X_loc[2 * BLOCKSIZE];
	__shared__ int niz_loc[BLOCKSIZE][BLOCKSIZE];

	if (threadIdx.y < 2)
		X_loc[threadIdx.y * BLOCKSIZE + threadIdx.x] = X[x + threadIdx.y * N / 2];
	__syncthreads();

	niz_loc[threadIdx.y][threadIdx.x] = A[y * N + x] * X_loc[threadIdx.x] + A[y * N + x + N / 2] * X_loc[threadIdx.x + BLOCKSIZE];
	__syncthreads();

	int len = BLOCKSIZE;
	while (len > 1)
	{
		if (threadIdx.x < len / 2)
			niz_loc[threadIdx.y][threadIdx.x] += niz_loc[threadIdx.y][threadIdx.x + len / 2];

		len /= 2;
		__syncthreads();
	}

	if (threadIdx.x == 0)
		A[y * N + blockIdx.x] = niz_loc[threadIdx.y][0];
}

__global__ void kernel_sabiranje(int *A, int *B)
{
	int x = threadIdx.x + blockIdx.x * blockDim.x;
	int y = threadIdx.y + blockIdx.y * blockDim.y;

	__shared__ int niz_loc[BLOCKSIZE][N / (4 * BLOCKSIZE)];

	niz_loc[threadIdx.y][threadIdx.x] = A[y * N + x] + A[y * N + x + blockDim.x];
	__syncthreads();

	int len = blockDim.x;
	while (len > 1)
	{
		if (threadIdx.x < len / 2)
			niz_loc[threadIdx.y][threadIdx.x] += niz_loc[threadIdx.y][threadIdx.x + len / 2];

		len /= 2;
		__syncthreads();
	}

	if (threadIdx.x == 0)
		B[y] = niz_loc[threadIdx.y][0];
}