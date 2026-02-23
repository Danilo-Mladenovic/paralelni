#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <stdio.h>
#include <math.h>

#define N 32
#define M 64
#define BLOCKSIZE 18
#define K00 -1
#define K01 -2
#define K02 -1
#define K10 0
#define K11 0
#define K12 0
#define K20 1
#define K21 2
#define K22 3
#define K 2

__global__ void kernel_sobel(int *A, int *B);

void main()
{
	int R[N][M], G[N][M], B[N][M], *M_dev, *R_dev;

	cudaMalloc((void **)&M_dev, N * M * sizeof(int));
	cudaMalloc((void **)&R_dev, N * M * sizeof(int));

	for (int i = 0; i < N; i++)
		for (int j = 0; j < M; j++)
		{
			R[i][j] = i + j;
			G[i][j] = i * j;
			B[i][j] = i - j;
		}

	cudaMemcpy(M_dev, R, N * M * sizeof(int), cudaMemcpyHostToDevice);
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < M; j++)
		{
			if (i == 0 || j == 0 || i == N - 1 || j == M - 1)
				printf("%d, ", R[i][j]);
			else
			{
				int val = R[i - 1][j - 1] * K00 + R[i - 1][j] * K01 + R[i - 1][j + 1] * K02 + R[i][j - 1] * K10 + R[i][j] * K11 + R[i][j + 1] * K12 + R[i + 1][j - 1] * K20 + R[i + 1][j] * K21 + R[i + 1][j + 1] * K22;
				printf("%d, ", val);
			}
		}
		printf("\n");
	}
	printf("\n");

	dim3 grid(M / (BLOCKSIZE - 2), N / (BLOCKSIZE - 2));
	dim3 block(BLOCKSIZE, BLOCKSIZE);

	kernel_sobel<<<grid, block>>>(M_dev, R_dev);
	cudaMemcpy(R, R_dev, N * M * sizeof(int), cudaMemcpyDeviceToHost);

	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < M; j++)
			printf("%d, ", R[i][j]);
		printf("\n");
	}
	printf("\n");

	cudaFree(M_dev);
	cudaFree(R_dev);
}

__global__ void kernel_sobel(int *A, int *B)
{
	int x = threadIdx.x - 1 + blockIdx.x * (blockDim.x - 2);
	int y = threadIdx.y - 1 + blockIdx.y * (blockDim.y - 2);

	__shared__ int loc[BLOCKSIZE][BLOCKSIZE];
	if (x >= 0 && y >= 0 && x < M && y < N)
		loc[threadIdx.y][threadIdx.x] = A[y * M + x];
	__syncthreads();

	if (threadIdx.x < BLOCKSIZE - 2 && threadIdx.y < BLOCKSIZE - 2)
	{
		int val;
		int i = threadIdx.y + 1;
		int j = threadIdx.x + 1;
		if (x >= 0 && y >= 0 && x < M && y < N)
			val = loc[i - 1][j - 1] * K00 + loc[i - 1][j] * K01 + loc[i - 1][j + 1] * K02 + loc[i][j - 1] * K10 + loc[i][j] * K11 + loc[i][j + 1] * K12 + loc[i + 1][j - 1] * K20 + loc[i + 1][j] * K21 + loc[i + 1][j + 1] * K22;
		else
			val = loc[threadIdx.y + 1][threadIdx.x + 1];

		B[(y + 1) * N + x + 1] = val;
	}
}