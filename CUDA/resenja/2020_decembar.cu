#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <stdio.h>
#include <math.h>

#define N 1024
#define M 25
#define BLOCKSIZE 50

__global__ void kernel_uzastopni(int *A, int *P);
__global__ void kernel_minimum(int *A, int *B);

void maindec2020()
{
	int A[N], P[N - M + 1], B, *A_dev, *P_dev, *B_dev;

	cudaMalloc((void **)&A_dev, N * sizeof(int));
	cudaMalloc((void **)&P_dev, (N - M + 1) * sizeof(int));
	cudaMalloc((void **)&B_dev, sizeof(int));

	for (int i = 0; i < N; i++)
		A[i] = i;
	for (int i = 0; i < N - M + 1; i++)
	{
		P[i] = 0;
		for (int j = i; j < i + M; j++)
			P[i] += A[j];
		printf("%d, ", P[i]);
	}
	printf("\n\n");

	cudaMemcpy(A_dev, A, N * sizeof(int), cudaMemcpyHostToDevice);

	dim3 grid((N - M + 1) / (BLOCKSIZE - M + 1), 1);
	dim3 block(BLOCKSIZE, M);

	kernel_uzastopni<<<grid, block>>>(A_dev, P_dev);

	cudaMemcpy(P, P_dev, (N - M + 1) * sizeof(int), cudaMemcpyDeviceToHost);
	for (int i = 0; i < N - M + 1; i++)
		printf("%d, ", P[i]);
	printf("\n\n");

	kernel_minimum<<<(N - M + 1) / (2 * BLOCKSIZE), BLOCKSIZE>>>(P_dev, P_dev);
	kernel_minimum<<<1, (N - M + 1) / (4 * BLOCKSIZE)>>>(P_dev, B_dev);

	cudaMemcpy(&B, B_dev, sizeof(int), cudaMemcpyDeviceToHost);
	printf("%d", B);

	cudaFree(A_dev);
	cudaFree(B_dev);
	cudaFree(P_dev);
}

__global__ void kernel_uzastopni(int *A, int *P)
{
	int x = threadIdx.x + blockIdx.x * (blockDim.x - M + 1);
	__shared__ int A_loc[BLOCKSIZE];
	__shared__ int calc[M][BLOCKSIZE - M + 1];

	if (threadIdx.y == 0)
		A_loc[threadIdx.x] = A[x];
	__syncthreads();

	if (threadIdx.x < BLOCKSIZE - M + 1)
		calc[threadIdx.y][threadIdx.x] = A_loc[threadIdx.x + threadIdx.y];
	__syncthreads();

	if (threadIdx.x <= BLOCKSIZE - M)
	{
		int len = M;
		while (len > 1)
		{
			if (threadIdx.y < len / 2)
				calc[threadIdx.y][threadIdx.x] += calc[threadIdx.y + len / 2][threadIdx.x];

			len /= 2;
			__syncthreads();
		}
		/*int val = 0;
		for (int i = threadIdx.x; i < threadIdx.x + M; i++)
			val += A_loc[i];*/

		if (threadIdx.y == 0)
			P[x] = calc[0][threadIdx.x];
	}
}

__device__ int minimalno(int a, int b)
{
	return a < b ? a : b;
}

__global__ void kernel_minimum(int *A, int *B)
{
	int x = threadIdx.x + blockIdx.x * blockDim.x;
	__shared__ int loc[BLOCKSIZE];

	loc[threadIdx.x] = minimalno(A[x], A[x + blockDim.x * gridDim.x]);
	__syncthreads();

	int len = blockDim.x;
	while (len > 0)
	{
		if (threadIdx.x < len / 2)
			loc[threadIdx.x] = minimalno(loc[threadIdx.x], loc[threadIdx.x + len / 2]);

		len /= 2;
		__syncthreads();
	}

	if (threadIdx.x == 0)
		B[blockIdx.x] = loc[0];
}