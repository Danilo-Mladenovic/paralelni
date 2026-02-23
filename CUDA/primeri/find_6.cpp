#include <stdio.h>
#include <stdlib.h>

#define BLOCKSIZE 16
#define GRIDSIZE 16

__global__ void find_6(int *d_arr_in, int *d_arr_out, int N)
{
    int global_tid = threadIdx.x + blockDim.x * blockIdx.x;
    int stride = blockDim.x * gridDim.x;

    int out_i = global_tid;

    d_arr_out[out_i] = 0;

    while (global_tid < N)
    {
        if (d_arr_in[global_tid] == 6)
        {
            d_arr_out[out_i] += 1;
        }

        global_tid += stride;
    }
}

__host__ void prepare_and_call_kernel(int *h_arr_in, int *h_arr_out, int N)
{

    int *d_arr_in, *d_arr_out;

    cudaMalloc(&d_arr_in, N * sizeof(int));
    cudaMalloc(&d_arr_out, BLOCKSIZE * GRIDSIZE * sizeof(int));

    cudaMemcpy((void *)d_arr_in, (void *)h_arr_in, N * sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy((void *)d_arr_out, (void *)h_arr_out, BLOCKSIZE * GRIDSIZE * sizeof(int), cudaMemcpyHostToDevice);

    find_6<<<GRIDSIZE, BLOCKSIZE>>>(d_arr_in, d_arr_out, N);

    cudaMemcpy((void *)h_arr_out, (void *)d_arr_out, BLOCKSIZE * GRIDSIZE * sizeof(int), cudaMemcpyDeviceToHost);

    int sum = 0;

    for (int i = 0; i < BLOCKSIZE * GRIDSIZE; i++)
    {
        // printf("el %d: %d\n",i, h_arr_out[i]);

        sum += h_arr_out[i];
    }

    printf("Sum: %d", sum);

    cudaFree(d_arr_in);
    cudaFree(d_arr_out);
}

int main()
{
    int N = 1024;

    int *h_arr_in, *h_arr_out;

    h_arr_in = (int *)malloc(N * sizeof(int));

    h_arr_out = (int *)malloc(BLOCKSIZE * GRIDSIZE * sizeof(int));

    for (int i = 0; i < N; i++)
    {
        h_arr_in[i] = i;
    }

    prepare_and_call_kernel(h_arr_in, h_arr_out, N);

    free(h_arr_in);
    free(h_arr_out);

    return 0;
}