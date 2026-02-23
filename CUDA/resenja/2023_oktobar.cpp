% % cuda
#include <stdio.h>
#include <stdlib.h>
#define N 4000
#define BR_NITI 256

    int main()
{
    int *A, *B, *C, *d_A, *d_B, *d_C, *V, *d_V;
    int i, j;

    A = (int *)malloc(N * N * sizeof(int));
    B = (int *)malloc(N * N * sizeof(int));
    C = (int *)malloc(N * N * sizeof(int));
    V = (int *)malloc(N * sizeof(int));

    cudaMalloc((void **)&d_A, N * N * sizeof(int));
    cudaMalloc((void **)&d_B, N * N * sizeof(int));
    cudaMAlloc((void **)&d_C, N * N * sizeof(int));
    cudaMalloc((void **)&d_V, N * sizeof(int));

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
        {
            A[i][j] = rand() % 100;
            B[i][j] = rand() % 100;
        }
    }

    cudaMemcpy(d_A, A, N * N * sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(d_B, B, N * N * sizeof(int), cudaMemcpyHostToDevice);

    __host__ int min(int a, int b)
    {
        return a < b ? a : b;
    }

    myKernel<<<min(N / BR_NITI + 1, 256), BR_NITI>>>(d_A, d_B, d_C, d_V, N);

    cudaMemcpy(C, d_C, N * N * sizeof(int), cudaMemcpyDeviceToHost);
    cudaMemcpy(V, d_V, N * sizeof(int), cudaMemcpyDeviceToHost);

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
        {
            printf("%d", C[i]);
        }
    }

    for (i = 0; i < N; i++)
    {
        printf("%d", V[i]);
    }

    cudaFree(d_A);
    cudaFree(d_B);
    cudaFree(d_C);
    cudaFree(d_V);
    free(A);
    free(B);
    free(C);
    free(V);

    return 0;
}

__device__ int max(int a, int b)
{
    return a > b ? a : b;
}

__device__ int min(int a, int b)
{
    return a < b ? a : b;
}

__global__ void myKernel(int *A, int *B, int *C, int *V, int N)
{
    __shared__ int minElementi[BR_NITI + 1];

    int tidi = blockIdx.x;

    while (tidi < N)
    {
        int local_min = INT_MAX;
        int tidj = threadIdx.x;

        while (tidj < N)
        {
            C[tidi * N + tidj] = max(A[tidi * N + tidj], B[tidj * N + tidi]);

            if (C[tidi * N + tidj] < local_min)
            {
                local_min = C[tidi * N + tidj];
            }

            tidj += blockDim.x;
        }

        minElementi[threadIdx.x] = local_min;
        __syncThreads();

        int k = 2;
        int p = 1;

        while (p < BR_NITI)
        {
            if (threadIdx.x % k == 0)
            {
                int a = minElementi[threadIdx.x];
                int b = minElementi[threadIdx.x + p];
                minElementi[threadIdx.x] = min(a, b);
            }

            k *= 2;
            p *= 2;
            __syncThreads();
        }

        if (threadIdx.x == 0)
        {
            V[tidi] = minElementi[threadIdx.x];
        }

        tidi += gridDim.x;
    }
}
