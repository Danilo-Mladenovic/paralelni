#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <stdio.h>
#include <math.h>

#define N 64
#define BLOCKSIZE 4

__global__ void kernel_cif(int *A, int *B);
__global__ void kernel_sab(int *A, int *B);

void mainjan2021()
{
	int A[N][N], B[N], *A_dev, *B_mid_dev, *B_dev, B_mid[N][N / (2 * BLOCKSIZE)];

	cudaMalloc((void **)&A_dev, N * N * sizeof(int));
	cudaMalloc((void **)&B_mid_dev, N * (N / (2 * BLOCKSIZE)) * sizeof(int));
	cudaMalloc((void **)&B_dev, N * sizeof(int));

	for (int i = 0; i < N; i++)
		for (int j = 0; j < N; j++)
			A[i][j] = i + j;

	for (int i = 0; i < N; i++)
	{
		int count = 0;
		for (int j = 0; j < N; j++)
		{
			int cif = A[i][j];
			int sum = cif % 10;
			int br = 1;
			while (cif / 10 > 0)
			{
				cif /= 10;
				br++;
				sum += cif % 10;
			}

			if (sum % br == 0)
				count++;
		}
		printf("%d, ", count);
	}
	printf("\n\n");

	cudaMemcpy(A_dev, A, N * N * sizeof(int), cudaMemcpyHostToDevice);

	dim3 grid1(N / (2 * BLOCKSIZE), N / BLOCKSIZE);
	dim3 block1(BLOCKSIZE, BLOCKSIZE);
	kernel_cif<<<grid1, block1>>>(A_dev, B_mid_dev);

	cudaMemcpy(B_mid, B_mid_dev, N * (N / (2 * BLOCKSIZE)) * sizeof(int), cudaMemcpyDeviceToHost);
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N / (2 * BLOCKSIZE); j++)
			printf("%d, ", B_mid[i][j]);
		printf("\n");
	}
	printf("\n");

	dim3 grid2(1, N / BLOCKSIZE);
	dim3 block2(N / (4 * BLOCKSIZE), BLOCKSIZE);
	kernel_sab<<<grid2, block2>>>(B_mid_dev, B_dev);

	cudaMemcpy(B, B_dev, N * sizeof(int), cudaMemcpyDeviceToHost);

	for (int i = 0; i < N; i++)
		printf("%d, ", B[i]);
	printf("\n\n");

	cudaFree(A_dev);
	cudaFree(B_dev);
	cudaFree(B_mid_dev);
}

__device__ int fitsCriteria(int num)
{
	int cif = num;
	int br = 1;
	int sum = cif % 10;
	while (cif / 10 > 0)
	{
		cif /= 10;
		br++;
		sum += num % 10;
	}

	if (sum % br == 0)
		return 1;
	return 0;
}

__global__ void kernel_cif(int *A, int *B)
{
	int x = threadIdx.x + blockIdx.x * blockDim.x;
	int y = threadIdx.y + blockIdx.y * blockDim.y;

	__shared__ int local_niz[BLOCKSIZE][BLOCKSIZE];
	local_niz[threadIdx.y][threadIdx.x] = fitsCriteria(A[y * N + x]) + fitsCriteria(A[y * N + x + N / 2]);
	__syncthreads();

	int len = blockDim.x;
	while (len > 1)
	{
		if (threadIdx.x < len / 2)
			local_niz[threadIdx.y][threadIdx.x] += local_niz[threadIdx.y][threadIdx.x + len / 2];

		len /= 2;
		__syncthreads();
	}

	if (threadIdx.x == 0)
		B[y * gridDim.x + blockIdx.x] = local_niz[threadIdx.y][0];
}

__global__ void kernel_sab(int *A, int *B)
{
	int x = threadIdx.x + blockIdx.x * blockDim.x;
	int y = threadIdx.y + blockIdx.y * blockDim.y;

	__shared__ int local_niz[BLOCKSIZE][N / (4 * BLOCKSIZE)];
	local_niz[threadIdx.y][threadIdx.x] = A[y * gridDim.x * blockDim.x * 2 + x] + A[y * gridDim.x * blockDim.x * 2 + x + gridDim.x * blockDim.x];
	__syncthreads();

	int len = blockDim.x;
	while (len > 1)
	{
		if (threadIdx.x < len / 2)
			local_niz[threadIdx.y][threadIdx.x] += local_niz[threadIdx.y][threadIdx.x + len / 2];

		len /= 2;
		__syncthreads();
	}

	if (threadIdx.x == 0)
		B[y * gridDim.x + blockIdx.x] = local_niz[threadIdx.y][0];
}