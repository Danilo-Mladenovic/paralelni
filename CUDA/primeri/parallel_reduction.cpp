#include <stdio.h>

#define N 65536
#define BLOCKSIZE 256

__global__ void parallel_reducion_sum(int *niz, int *o_niz)
{
    __shared__ int s_mem[BLOCKSIZE];

    int i = (blockDim.x * 2) * blockIdx.x + threadIdx.x;

    s_mem[threadIdx.x] = niz[i] + niz[i + blockDim.x];

    __syncthreads();

    for (int s = blockDim.x / 2; s > 0; s >>= 1)
    {

        if (threadIdx.x < s)
        {
            s_mem[threadIdx.x] += s_mem[threadIdx.x + s];
        }

        __syncthreads();
    }

    if (threadIdx.x == 0)
    {
        o_niz[blockIdx.x] = s_mem[threadIdx.x];
    }
}

int main()
{
    int *niz, *d_niz, *out, *d_out;

    size_t out_bytes = (N / BLOCKSIZE / 2) * sizeof(int);

    niz = (int *)malloc(N * sizeof(int));
    out = (int *)malloc(out_bytes);
    cudaMalloc(&d_niz, N * sizeof(int));
    cudaMalloc(&d_out, (N / (BLOCKSIZE * 2)) * sizeof(int));

    for (int i = 0; i < N; i++)
        niz[i] = 1;

    cudaMemcpy(d_niz, niz, N * sizeof(int), cudaMemcpyHostToDevice);

    parallel_reducion_sum<<<N / (BLOCKSIZE * 2), BLOCKSIZE>>>(d_niz, d_out);
    cudaDeviceSynchronize();
    parallel_reducion_sum<<<1, N / (BLOCKSIZE * 2 * 2)>>>(d_out, d_out);

    cudaMemcpy(out, d_out, (N / (BLOCKSIZE * 2)) * sizeof(int), cudaMemcpyDeviceToHost);

    printf("SUM: %d", out[0]);

    cudaFree(d_out);
    cudaFree(d_niz);
    free(out);
    free(niz);
    return 0;
}