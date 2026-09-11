// Koriscenjem CUDA tehnologije, u programskom jeziku C/C++ napisati program koji od dve kvadratne
// matrice jednakih dimenzija A i B kreira matricu C takvu da je C[i][j] = A[i][j] - 2 * B[i][j],
// i kreira vektor ciji je i-ti element minimalna vrednost i-te vrste matrice C. Obratiti paznju
// na efikasnost paralelizacije.

% % cuda
#include <stdio.h>
#include <stdlib.h>

#define N 4000
#define NUM_OF_THREADS 256

        __global__ void calculateVector(int *A, int *B, int *C, int *D, int N)
{
  __shared__ int shared_data[NUM_OF_THREADS];

  int tid = threadIdx.x;

  // grid-stride: jedan blok obradjuje jednu vrstu, pa se pomera za gridDim.x
  int row = blockIdx.x;
  while (row < N)
  {
    int local_min = INT_MAX;

    // niti bloka dele kolone ove vrste, racunaju C i svoj lokalni min
    for (int col = tid; col < N; col += blockDim.x)
    {
      int c = A[row * N + col] - 2 * B[row * N + col];
      C[row * N + col] = c;
      if (c < local_min)
        local_min = c;
    }

    shared_data[tid] = local_min;
    __syncthreads();

    // redukcija minimuma unutar bloka
    for (int stride = blockDim.x / 2; stride > 0; stride /= 2)
    {
      if (tid < stride && shared_data[tid + stride] < shared_data[tid])
        shared_data[tid] = shared_data[tid + stride];
      __syncthreads();
    }

    if (tid == 0)
      D[row] = shared_data[0];

    __syncthreads(); // da sledeca vrsta ne prepise shared_data prerano

    row += gridDim.x;
  }
}

int main()
{
  type_t matrix_size = N * N sizeof(int);
  type_t vector_size = N * sizeof(int);

  int *A = (int *)malloc(matrix_size);
  int *B = (int *)malloc(matrix_size);
  int *C = (int *)malloc(matrix_size);
  int *D = (int *)malloc(vector_size);

  int *d_A, *d_B, *d_C, *d_D;

  cudaMalloc((void **)&d_A, matrix_size);
  cudaMalloc((void **)&d_B, matrix_size);
  cudaMalloc((void **)&d_C, matrix_size);
  cudaMalloc((void **)&d_D, vector_size);

  for (int i = 0; i < N; i++)
  {
    for (int j = 0; j < N; j++)
    {
      A[i * N + j] = rand() % 100;
      B[i * N + j] = rand() % 100;
    }
  }

  cudaMemCpy(d_A, A, matrix_size, cudaMemCpyHostToDevice);
  cudaMemCpy(d_B, B, matrix_size, cudaMemCpyHostToDevice);

  int NUM_OF_BLOCKS = min(N / NUM_OF_THREADS + 1, NUM_OF_THREADS);
  calculateVector<<<NUM_OF_BLOCKS, NUM_OF_THREADS>>>(d_A, d_B, d_C, d_D, N);

  cudaMemCpy(C, d_C, matrix_size, cudaMemCpyDeviceToHost);
  cudaMemCpy(D, d_D, vector_size, cudaMemCpyDeviceToHost);

  for (int i = 0; i < N; i++)
  {
    for (int j = 0; j < N; j++)
    {
      printf("%d, ", C[i * N + j]);
    }
    printf("\n");
  }

  for (int i = 0; i < N; i++)
  {
    prinf("%d, ", D[i]);
  }

  cudaFree(d_A);
  cudaFree(d_B);
  cudaFree(d_C);
  cudaFree(d_D);

  free(A);
  free(B);
  free(C);
  free(D);

  return 1;
}