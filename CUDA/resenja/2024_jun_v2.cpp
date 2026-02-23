
#include <stdio.h>

#define N 4096
#define BLOCKSIZE 256

struct Point
{
    int x;
    int y;
};

struct Circle
{
    Point o;
    int r;
};

__device__ Circle min(Circle c1, Circle c2)
{
    if (c1.r > c2.r)
    {
        return c2;
    }

    return c1;
}

__device__ void min2(Circle c1, Circle c2, Circle c3, Circle c4, Circle *rez)
{
    Circle min1, min2;
    if (c1.r < c2.r)
    {
        min1 = c1;
        min2 = c2;
    }
    else
    {
        min1 = c2;
        min2 = c1;
    }

    if (c3.r < min1.r)
    {
        min2 = min1;
        min1 = c3;
    }
    else if (c3.r < min2.r)
    {
        min2 = c3;
    }

    if (c4.r < min1.r)
    {
        min2 = min1;
        min1 = c4;
    }
    else if (c4.r < min2.r)
    {
        min2 = c4;
    }

    rez[0] = min1;
    rez[1] = min2;
}

// jedan od nacina (mozes da razmislis i o tome da shared mamori bude matrica [BLOCKSIZE][2])
__global__ void minCircle(Circle *in_arr, Circle *out_arr)
{
    __shared__ Circle shr_mem[2 * BLOCKSIZE];

    Circle rez[2];

    // int ind = (blockDim.x * 2) * blockIdx.x + threadIdx.x;

    int ind2 = (blockDim.x * 4) * blockIdx.x + threadIdx.x;

    // shr_mem[threadIdx.x] = min(in_arr[ind], in_arr[ind + blockDim.x]);

    min2(in_arr[ind2], in_arr[ind2 + blockDim.x], in_arr[ind2 + 2 * blockDim.x], in_arr[ind2 + 3 * blockDim.x], rez);

    shr_mem[2 * threadIdx.x] = rez[0];
    shr_mem[2 * threadIdx.x + 1] = rez[1];

    __syncthreads();

    for (int i = blockDim.x / 2; i > 0; i >>= 1)
    {

        if (threadIdx.x < i)
        {
            min2(shr_mem[2 * threadIdx.x], shr_mem[2 * threadIdx.x + 1], shr_mem[2 * (threadIdx.x + i)], shr_mem[2 * (threadIdx.x + i) + 1], rez);
            // shr_mem[threadIdx.x] = min(shr_mem[threadIdx.x], shr_mem[threadIdx.x + i]);

            shr_mem[2 * threadIdx.x] = rez[0];
            shr_mem[2 * threadIdx.x + 1] = rez[1];
        }

        __syncthreads();
    }

    if (threadIdx.x == 0)
    {
        out_arr[2 * blockIdx.x] = shr_mem[0];
        out_arr[2 * blockIdx.x + 1] = shr_mem[1];
    }
}

int main()
{
    Circle *in_arr, *out_arr;
    Circle *d_in_arr, *d_out_arr;

    in_arr = (Circle *)malloc(N * sizeof(Circle));
    out_arr = (Circle *)malloc((N / BLOCKSIZE) * sizeof(Circle));

    for (int i = 0; i < N; i++)
    {
        in_arr[i].o.x = i;
        in_arr[i].o.y = i;
        in_arr[i].r = i;
    }

    cudaMalloc(&d_in_arr, N * sizeof(Circle));
    cudaMalloc(&d_out_arr, (N / BLOCKSIZE) * sizeof(Circle));

    cudaMemcpy(d_in_arr, in_arr, N * sizeof(Circle), cudaMemcpyHostToDevice);

    minCircle<<<N / BLOCKSIZE / 4, BLOCKSIZE>>>(d_in_arr, d_out_arr);

    cudaDeviceSynchronize();

    minCircle<<<1, N / BLOCKSIZE / 16>>>(d_out_arr, d_out_arr);

    cudaMemcpy(out_arr, d_out_arr, (N / BLOCKSIZE) * sizeof(Circle), cudaMemcpyDeviceToHost);

    printf("MIN: O(%d, %d), r = %d\n", out_arr[0].o.x, out_arr[0].o.y, out_arr[0].r);
    printf("MIN2: O(%d, %d), r = %d\n", out_arr[1].o.x, out_arr[1].o.y, out_arr[1].r);

    cudaFree(d_out_arr);
    cudaFree(d_in_arr);
    free(in_arr);
    free(out_arr);

    return 0;
}
