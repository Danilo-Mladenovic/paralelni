
#include <stdlib.h>
#include <stdio.h>

#define N 12
#define M 15
#define P 16
#define GRIDDIM 3
#define BLOCKSIZE 2

__host__ void printMatrix(int *matrix, int rows, int columns)
{
    int index = 0;
    while (index < rows * columns)
    {

        if ((index + 1) % columns == 0)
        {
            printf("%d\n", matrix[index]);
        }
        else
        {
            printf("%d ", matrix[index]);
        }

        index += 1;
    }

    printf("\n\n");
}

__host__ void populateMatrix(int *matrix, int rows, int columns)
{

    for (int i = 0; i < rows * columns; i++)
    {
        matrix[i] = rand() % 10;
    }
}

__global__ void matrixMul(int *dA, int *dB, int *dC)
{
    __shared__ int sA[BLOCKSIZE][BLOCKSIZE + 1];
    __shared__ int sB[BLOCKSIZE][BLOCKSIZE + 1];

    int x, y;
    for (int blockRow = blockIdx.y; blockRow < (M + BLOCKSIZE - 1) / BLOCKSIZE; blockRow += gridDim.y)
    {
        for (int blockCol = blockIdx.x; blockCol < (P + BLOCKSIZE - 1) / BLOCKSIZE; blockCol += gridDim.x)
        {

            int row = blockRow * BLOCKSIZE + threadIdx.y; // indeks u C po M
            int col = blockCol * BLOCKSIZE + threadIdx.x; // indeks u C po P

            float sum = 0.0f;

            for (int tileIdx = 0; tileIdx < (N + BLOCKSIZE - 1) / BLOCKSIZE; tileIdx++)
            {
                int aCol = tileIdx * BLOCKSIZE + threadIdx.x; // indeks u A po N
                int bRow = tileIdx * BLOCKSIZE + threadIdx.y; // indeks u B po N

                // učitavanje pločice iz A
                if (row < M && aCol < N)
                    sA[threadIdx.y][threadIdx.x] = dA[row * N + aCol];
                else
                    sA[threadIdx.y][threadIdx.x] = 0.0f;

                // učitavanje pločice iz B
                if (bRow < N && col < P)
                    sB[threadIdx.y][threadIdx.x] = dB[bRow * P + col];
                else
                    sB[threadIdx.y][threadIdx.x] = 0.0f;

                __syncthreads();

                // računanje parcijalnog zbira
                for (int k = 0; k < BLOCKSIZE; k++)
                {
                    sum += sA[threadIdx.y][k] * sB[k][threadIdx.x];
                }

                __syncthreads();
            }

            // upis u C
            if (row < M && col < P)
            {
                dC[row * P + col] = sum;
            }
        }
    }
}

int main()
{
    int *A, *B, *C, *dA, *dB, *dC;

    A = (int *)malloc(M * N * sizeof(int));
    B = (int *)malloc(N * P * sizeof(int));
    C = (int *)malloc(M * P * sizeof(int));

    populateMatrix(A, M, N);
    populateMatrix(B, N, P);

    printMatrix(A, M, N);
    printMatrix(B, N, P);

    dim3 grid_size(GRIDDIM, GRIDDIM);
    dim3 block_size(BLOCKSIZE, BLOCKSIZE);

    cudaMalloc(&dA, M * N * sizeof(int));
    cudaMalloc(&dB, N * P * sizeof(int));
    cudaMalloc(&dC, M * P * sizeof(int));

    cudaMemcpy(dA, A, M * N * sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(dB, B, N * P * sizeof(int), cudaMemcpyHostToDevice);

    matrixMul<<<grid_size, block_size>>>(dA, dB, dC);

    cudaMemcpy(C, dC, M * P * sizeof(int), cudaMemcpyDeviceToHost);

    printMatrix(C, M, P);

    free(A);
    free(B);
    free(C);
    cudaFree(dA);
    cudaFree(dB);
    cudaFree(dC);
}