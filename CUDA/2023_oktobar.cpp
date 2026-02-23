// Koriscenjem CUDA tehnologije u programskom jeziku C++ napisati program koji od dve kvadratne matrice
// jadnakih dimenzija A i B kreira matricu C takvu da je C[i][j] = max(A[i][j], B[j][i]) i kreira vektor
// ciji je i=ti element minimalna vrednost i=te vrste matrice C. Velicinu matrica unosi korisnik.
// Obratiti paznju na efikasnost paralelizacije. Omoguciti pozivanje kernela za matrice proizvoljne velicine.
// Pretpostaviti velicinu bloka od 256 niti i broj blokova ne veci od 256.

% % cuda
#include <stdio.h>
#include <stdlib.h>
#include <climits>
#define N 4000
#define NUM_OF_THREADS 256

        __global__ void processMatrix(int *A, int *B, int *C, int *V, int N)
{
  __shared__ int min_elementi[NUM_OF_THREADS + 1];

  int thread_id = threadIdx.x;
  int block_id = blockIdx.x;

  int tid_i = block_id;

  while (tid_i < N)
  {
    int local_min = INT_MAX;
    int tid_j = thread_id;

    while (tid_j < N)
    {
      C[tid_i * N + tid_j] = max(A[tid_i * N + tid_j], B[tid_j * N + tid_i]);

      if (C[tid_i * N + tid_j] < local_min)
      {
        local_min = C[tid_i * N + tid_j];
      }

      tid_j += blockDim.x;
    }

    min_elementi[thread_id] = local_min;
    __syncThreads();

    int k = 2;
    int p = 1;

    while (p < NUM_OF_THREADS)
    {
      if (thread_id % k == 0)
      {
        int a = min_elementi[thread_id];
        int b = min_elementi[thread_id + p];
        min_elementi[thread_id] = min(a, b);
      }

      k *= 2;
      p *= 2;
      __syncThreads();
    }

    if (thread_id == 0)
    {
      V[tid_i] = min_elementi[0];
    }

    tid_i += gridDim.x;
  }
}

int main()
{
  size_t matrix_size = N * N * sizeof(int);
  size_t vector_size = N * sizeof(int);

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
      A[i * N + j] = rand() % 100 + 1;
      B[i * N + j] = rand() % 100 + 1;
    }
  }

  // Kopiranje na device
  cudaMemcpy(d_A, A, matrix_size, cudaMemcpyHostToDevice);
  cudaMemcpy(d_B, B, matrix_size, cudaMemcpyHostToDevice);

  int NUM_OF_BLOCKS = min(N / NUM_OF_THREADS + 1, 256);
  processMatrix<<<NUM_OF_BLOCKS, NUM_OF_THREADS>>>(d_A, d_B, d_C, d_V, N);

  // Kopiranje rezultata nazad na host
  cudaMemcpy(C, d_C, matrix_size, cudaMemcpyDeviceToHost);
  cudaMemcpy(V, d_V, vector_size, cudaMemcpyDeviceToHost);

  for (int i = 0; i < N; i++)
  {
    for (int j = 0; j < N; j++)
    {
      printf("%d", C[i * N + j]);
    }
  }

  for (int i = 0; i < N; i++)
  {
    printf("%d", V[i]);
  }

  free(A);
  free(B);
  free(C);
  free(V);
  cudaFree(d_A);
  cudaFree(d_B);
  cudaFree(d_C);
  cudaFree(d_V);

  return 0;
}

__device__ int min(int a, int b)
{
  return a < b ? a : b;
}

__device__ int max(int a, int b)
{
  return a > b ? a : b;
}
