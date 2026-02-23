#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <stdio.h>
#include <math.h>

#define N 34
#define BLOCKSIZE 18

__global__ void kernel_izraz(int *A, int *B);

void mainapr2021()
{
	int A[N][N], B[N - 2][N - 2], *A_dev, *B_dev;

	cudaMalloc((void **)&A_dev, N * N * sizeof(int));
	cudaMalloc((void **)&B_dev, (N - 2) * (N - 2) * sizeof(int));

	for (int i = 0; i < N; i++)
		for (int j = 0; j < N; j++)
			A[i][j] = i + j;

	for (int i = 0; i < N - 2; i++)
		for (int j = 0; j < N - 2; j++)
			B[i][j] = A[i][j] / 4.f + A[i][j + 2] / 4.f + A[i + 2][j] / 4.f + A[i + 2][j + 2] / 4.f;

	for (int i = 0; i < N - 2; i++)
	{
		for (int j = 0; j < N - 2; j++)
			printf("%d, ", B[i][j]);
		printf("\n");
	}
	printf("\n");

	cudaMemcpy(A_dev, A, N * N * sizeof(int), cudaMemcpyHostToDevice);

	dim3 grid((N - 2) / (BLOCKSIZE - 2), (N - 2) / (BLOCKSIZE - 2));
	dim3 block(BLOCKSIZE, BLOCKSIZE);

	kernel_izraz<<<grid, block>>>(A_dev, B_dev);

	cudaMemcpy(B, B_dev, (N - 2) * (N - 2) * sizeof(int), cudaMemcpyDeviceToHost);

	for (int i = 0; i < N - 2; i++)
	{
		for (int j = 0; j < N - 2; j++)
			printf("%d, ", B[i][j]);
		printf("\n");
	}
	printf("\n");

	cudaFree(A_dev);
	cudaFree(B_dev);
}

__global__ void kernel_izraz(int *A, int *B)
{
	int x = threadIdx.x + blockIdx.x * (blockDim.x - 2);
	int y = threadIdx.y + blockIdx.y * (blockDim.y - 2);

	__shared__ int local_mem[BLOCKSIZE][BLOCKSIZE];
	local_mem[threadIdx.y][threadIdx.x] = A[y * N + x];
	__syncthreads();

	if (threadIdx.x < blockDim.x - 2 && threadIdx.y < blockDim.y - 2)
		B[y * (N - 2) + x] = local_mem[threadIdx.y][threadIdx.x] / 4.f +
												 local_mem[threadIdx.y][threadIdx.x + 2] / 4.f +
												 local_mem[threadIdx.y + 2][threadIdx.x] / 4.f +
												 local_mem[threadIdx.y + 2][threadIdx.x + 2] / 4.f;
}