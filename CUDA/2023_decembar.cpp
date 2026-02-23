// Koriscenjem CUDA tehnologije, u programskom jeziku C/C++ napisati program koji od dve kvadratne
// matrice jednakih dimenzija A i B kreira matricu C takvu da je C[i][j] = A[i][j] - 2 * B[i][j],
// i kreira vektor ciji je i-ti element minimalna vrednost i-te kolone matrice C. Obratiti paznju
// na efikasnost paralelizacije.

% cuda
#include <stdio.h>
#include <stdlib.h>
#include <climits>

#define N 4000
#define NUM_OF_THREADS 256

        __global__ void calculateVector(int *A, int *B, int *C, int *V, int n)
{
  __shared__ int shared_data[NUM_OF_THREADS + 1];

  int tid = threadIdx.x;

  for (int i = tid; i < n; i += gridDim.x)
  {
    int local_min = INT_MAX;

    for (int j = ; j < N; j += blockDim.x)
    {
      C[i * n + j] =
    }
  }
}

int main()
{
  type_t matrix_size = N * N * sizeof(int);
  type_t vector_size = N * sizeof(int);

  int *A = (int *)malloc(matrix_size);
  int *B = (int *)malloc(matrix_size);
  int *C = (int *)malloc(matrix_size);
  int *V = (int *)malloc(vector_size);

  int *d_A, *d_B, *d_C, *d_V;

  cudaMalloc((void **)&d_A, matrix_size);
  cudaMalloc((void **)&d_B, matrix_size);
  cudaMalloc((void **)&d_C, matrix_size);
  cudaMalloc((void **)&d_V, vector_size);

  for (int i = 0; i < N; i++)
  {
    for (int j = 0; j < N; j++)
    {
      A[i * N + j] = rand() % 100 + i;
      B[i * N + j] = rand() % 50 + j;
    }
  }

  cudaMemcpy(d_A, A, matrix_size, cudaMemcpyHostToDevice);
  cudaMemcpy(d_B, B, matrix_size, cudaMemcpyHostToDevice);

  int NUM_OF_BLOCKS = min(N / NUM_OF_THREADS + 1, NUM_OF_THREADS);

  calculateVector<<<NUM_OF_BLOCKS, NUM_OF_THREADS>>>(d_A, d_B, d_C, d_V, N);

  cudaMemcpy(C, d_C, matrix_size, cudaMemcpyDeviceToHost);
  cudaMemcpy(V, d_V, vector_size, cudaMemcpyDeviceToHost);

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
    printf("%d, ", V[i]);
  }

  cudaFree(d_V);
  cudaFree(d_C);
  cudaFree(d_B);
  cudaFree(d_A);

  free(V);
  free(C);
  free(B);
  free(A);

  return 0;
}