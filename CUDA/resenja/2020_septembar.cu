#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <stdio.h>
#include <math.h>

#define N 34
#define BLOCKSIZE 18

__global__ void kernel_izraz_mat(int *A, int *B);

void mainsep2020()
{
	int A[N][N], B[N - 2][N - 2], *A_dev, *B_dev;

	cudaMalloc((void **)&A_dev, N * N * sizeof(int));
	cudaMalloc((void **)&B_dev, (N - 2) * (N - 2) * sizeof(int));

	for (int i = 0; i < N; i++)
		for (int j = 0; j < N; j++)
			A[i][j] = i + j;

	for (int i = 0; i < N - 2; i++)
	{
		for (int j = 0; j < N - 2; j++)
		{
			B[i][j] = (A[i][j] + A[i][j + 2] + A[i + 1][j] + A[i + 1][j + 1] + A[i + 1][j + 2] + A[i + 2][j] + A[i + 2][j + 1] + A[i + 2][j + 2]) / 9.f;
			printf("%d, ", B[i][j]);
		}
		printf("\n");
	}
	printf("\n");

	cudaMemcpy(A_dev, A, N * N * sizeof(int), cudaMemcpyHostToDevice);

	dim3 block(BLOCKSIZE, BLOCKSIZE);
	dim3 grid((N - 2) / (BLOCKSIZE - 2), (N - 2) / (BLOCKSIZE - 2));

	kernel_izraz_mat<<<grid, block>>>(A_dev, B_dev);

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

__global__ void kernel_izraz_mat(int *A, int *B)
{
	int x = threadIdx.x + blockIdx.x * (blockDim.x - 2);
	int y = threadIdx.y + blockIdx.y * (blockDim.y - 2);

	__shared__ int loc_niz[BLOCKSIZE][BLOCKSIZE];

	loc_niz[threadIdx.y][threadIdx.x] = A[y * N + x];
	__syncthreads();
	int i = threadIdx.y;
	int j = threadIdx.x;

	if (threadIdx.y < BLOCKSIZE - 2 && threadIdx.x < BLOCKSIZE - 2)
		B[y * (N - 2) + x] = (loc_niz[i][j] + loc_niz[i][j + 2] + loc_niz[i + 1][j] + loc_niz[i + 1][j + 1] + loc_niz[i + 1][j + 2] + loc_niz[i + 2][j] + loc_niz[i + 2][j + 1] + loc_niz[i + 2][j + 2]) / 9.f;
}