#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <device_launch_parameters.h>
// #include <device_functions.h>
#include <cuda.h>
#include <cuda_runtime_api.h>
// #include <cutil.h>
#include <Math.h>
#define N 10
#define br_niti 5
#define br_blokova 2
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
		printf("ERROR>line:%i.%d' � %s\n", __LINE__, ret,
					 cudaGetErrorString(ret));
		exit(-1);
		break;
	}
}
__global__ void kernel(int **mat, int *niz)
{
	int brBloka = blockIdx.x;
	while (brBloka < N)
	{

		int tid = threadIdx.x;

		__shared__ int broj_pozitivnih[br_niti];

		int slice = N / br_niti;
		int start = threadIdx.x * slice;
		int end = start + slice;

		if (tid == br_niti - 1)
			end += N % br_niti;

		int lok_br_poz = 0;
		for (int i = start; i < end; i++)
			if (mat[brBloka][i] > 0)
				lok_br_poz++;
		broj_pozitivnih[tid] = lok_br_poz;
		__syncthreads();

		int half = br_niti / 2;
		while (half > 0)
		{
			__syncthreads();
			if (tid < half)
				broj_pozitivnih[tid] += broj_pozitivnih[tid +
																								half];
			half /= 2;
		}
		if (tid == 0)
			niz[brBloka] = broj_pozitivnih[0] - (N -
																					 broj_pozitivnih[0]);
		__syncthreads();
		for (int i = start; i < end; i++)
			if (mat[brBloka][i] < 0)
				mat[brBloka][i] = broj_pozitivnih[0] - (N -
																								broj_pozitivnih[0]);
		__syncthreads();
		brBloka += gridDim.x;
	}
}
int main()
{
	int **h_matrix, **d_matrix, *h_niz, *d_niz;
	h_niz = (int *)malloc(N * sizeof(int));
	h_matrix = (int **)malloc(N * sizeof(int *));
	for (int i = 0; i < N; i++)
		h_matrix[i] = (int *)malloc(N * sizeof(int));
	printf("\n");
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N; j++)
		{
			h_matrix[i][j] = rand() % 100 - 50;
			printf("%d ", h_matrix[i][j]);
		}
		printf("\n");
	}
	printf("\n");
	HANDLE_ERROR(cudaMalloc((void **)&d_matrix, N * N * sizeof(int)));
	HANDLE_ERROR(cudaMalloc((void **)&d_niz, N * sizeof(int)));
	HANDLE_ERROR(cudaMemcpy(d_matrix, h_matrix, N * N * sizeof(int),
													cudaMemcpyHostToDevice));
	kernel<<<br_blokova, br_niti>>>(d_matrix, d_niz);
	HANDLE_ERROR(cudaMemcpy(h_niz, d_niz, N * sizeof(int),
													cudaMemcpyDeviceToHost));
	HANDLE_ERROR(cudaMemcpy(h_matrix, d_matrix, N * N * sizeof(int),
													cudaMemcpyDeviceToHost));
	for (int i = 0; i < N; i++)
		printf("%d ", h_niz[i]);
	printf("\n");
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N; j++)
		{
			printf("%d ", h_matrix[i][j]);
		}
		printf("\n");
	}
	cudaFree(d_matrix);
	cudaFree(d_niz);
}