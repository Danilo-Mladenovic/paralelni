#include <stdio.h>
#include <stdlib.h>

#define ROWS 6
#define COLUMNS 5

#define BLOCKSIZE 16
#define GRIDSIZE 2

__global__ void transposeMatrixImproved(double *in, double *out, int rows, int cols)
{
    __shared__ double tile[BLOCKSIZE][BLOCKSIZE + 1];
    int x, y;
    for (int blockY = blockIdx.y; blockY < (rows + BLOCKSIZE - 1) / BLOCKSIZE; blockY += gridDim.y)
    {
        for (int blockX = blockIdx.x; blockX < (cols + BLOCKSIZE - 1) / BLOCKSIZE; blockX += gridDim.x)
        {
            x = blockX * BLOCKSIZE + threadIdx.x;
            y = blockY * BLOCKSIZE + threadIdx.y;

            if (x < cols && y < rows)
            {
                tile[threadIdx.y][threadIdx.x] = in[y * cols + x];
            }

            __syncthreads();

            x = blockY * BLOCKSIZE + threadIdx.x;
            y = blockX * BLOCKSIZE + threadIdx.y;

            if (x < rows && y < cols)
            {
                out[y * rows + x] = tile[threadIdx.x][threadIdx.y];
            }

            __syncthreads();
        }
    }
}

__host__ void kernelCallWrapper(double *h_in, double *h_out, size_t matrixSize)
{
    double *d_in, *d_out;

    dim3 grid_size(GRIDSIZE, GRIDSIZE);
    dim3 block_size(BLOCKSIZE, BLOCKSIZE);

    cudaMalloc(&d_in, matrixSize);
    cudaMalloc(&d_out, matrixSize);

    cudaMemcpy(d_in, h_in, matrixSize, cudaMemcpyHostToDevice);

    transposeMatrixImproved<<<grid_size, block_size>>>(d_in, d_out, ROWS, COLUMNS);

    cudaMemcpy(h_out, d_out, matrixSize, cudaMemcpyDeviceToHost);

    cudaFree(d_in);
    cudaFree(d_out);
}

__host__ void printMatrix(double *matrix, int rows, int columns)
{
    int index = 0;
    while (index < rows * columns)
    {

        if ((index + 1) % columns == 0)
        {
            printf("%f\n", matrix[index]);
        }
        else
        {
            printf("%f ", matrix[index]);
        }

        index += 1;
    }

    printf("\n\n");
}

__host__ void populateMatrix(double *h_in)
{

    for (int i = 0; i < ROWS * COLUMNS; i++)
    {
        h_in[i] = (double)(rand() % 100);
    }
}

int main()
{
    double *h_in, *h_out;

    size_t matrixSize = ROWS * COLUMNS * sizeof(double);

    h_in = (double *)malloc(matrixSize);
    h_out = (double *)malloc(matrixSize);

    populateMatrix(h_in);

    printMatrix(h_in, ROWS, COLUMNS);

    kernelCallWrapper(h_in, h_out, matrixSize);

    printMatrix(h_out, COLUMNS, ROWS);

    free(h_in);
    free(h_out);
}
