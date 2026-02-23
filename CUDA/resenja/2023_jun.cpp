% % cuda
#include <stdio.h>
#include <stdlib.h>
#define BR_NITI 256
#define N 3000

    int main()
{
    int *A, *d_A, i, j, *minel, *d_minel;

    A = (int *)malloc(N * sizeof(int));
    cudaMalloc((void **)&d_A, N * N * sizeof(int));

    minel = (int *)malloc(1 * sizeof(int));
    cudaMalloc((void **)&d_minel, 1 * sizeof(int));

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
        {
            A[i * N + j] = rand() % 20;
        }
    }

    // Vrsimo prenos podataka sa hosta na device

    cudaMemcpy(d_A, A, N * N * sizeof(int), cudaMemcpyHostToDevice);

    myKernel<<<1, BR_NITI>>>(d_A, d_minel, N);

    cudaMemcpy(&minel, &d_minel, 1 * sizeof(int), cudaMemcpyDeviceToHost);

    printf("%d", minel);

    free(A);
    free(minel);
    cudaFree(d_A);
    cudaFree(d_minel);

    return 0;
}

__device__ int min(int a, int b)
{
    return a < b ? a : b;
}

__global__ void myKernel(int *A, int *minel, int N)
{
    __shared__ int minElementi[BR_NITI + 1];
    int local_min = INT_MAX;

    for (int i = threadIdx.x; i < N; i += blockDim.x)
    {
        if (A[i * N + i] < local_min)
        {
            local_min = A[i * N + i];
        }
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
        __syncTreads(); // mopra da se saceka da sve niti dodju u tu tacku sinhronizacije zbog pristupa deljivoj memoriji i zbog modifikacije deljive memorije!!!
    }

    if (threadIdx.x == 0)
    {
        minel = minElementi[threadIdx.x];
    }
}
