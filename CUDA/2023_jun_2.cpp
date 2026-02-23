// Koristeci CUDA tehnologiju, napisati program koji za dati niz A[n] racuna niz B[n-2] na sledeci nacin:
// B[i] = (A[i]*A[i+1]*A[i+2])/(A[i]+A[i+1]+A[i+2])
// Velicinu niz A unosi korisnik. Maksimalno redukovati broj pristupa globalnoj memoriji.
// Obratiti paznju na efikasnost paralelizacije. Omoguciti rad programa za nizove proizvoljne velicine.

% % cuda
#include <stdio.h>
#include <stdlib.h>
#include <climits>
#define N 4000
#define NUM_OF_THREADS 256

        __global__ void calculateVector(int *A, int *B, int n)
{
  __shared__ int shared_A[NUM_OF_THREADS + 2];

  int thread_id = threadIdx.x;
  int grid_id = blockIdx.x * blockDim.x + thread_id;

  while (grid_id < n - 2)
  {
    if (grid_id < n)
    {
      shared_A[thread_id] = A[grid_id];
    }

    if (thread_id == 0)
    {
      if (grid_id + NUM_OF_THREADS < n)
      {
        shared_A[NUM_OF_THREADS] = A[grid_id + NUM_OF_THREADS];
      }
      if (grid_id + NUM_OF_THREADS + 1 < n)
      {
        shared_A[NUM_OF_THREADS + 1] = A[grid_id + NUM_OF_THREADS + 1];
      }
    }

    __syncthreads();

    if (grid_id < n - 2 && thread_id < NUM_OF_THREADS)
    {
      int a1 = shared_A[thread_id];
      int a2 = shared_A[thread_id + 1];
      int a3 = shared_A[thread_id + 2];

      int suma = a1 * a2 * a3;
      int zbir = a1 + a2 + a3;

      B[grid_id] = zbir == 0 ? 0 : (suma / zbir);
    }

    __syncthreads();
    grid_id += blockDim.x * gridDim.x;
  }
}

int main()
{
  size_t vector_a_size = N * sizeof(int);
  size_t vector_b_size = (N - 2) * sizeof(int);

  int *A = (int *)malloc(vector_a_size);
  int *B = (int *)malloc(vector_b_size);

  int *d_A, *d_B;

  cudaMalloc((void **)&d_A, vector_a_size);
  cudaMalloc((void **)&d_B, vector_b_size);

  for (int i = 0; i < N; i++)
  {
    A[i] = rand() % 100 + i;
  }

  cudaMemcpy(d_A, A, vector_a_size, cudaMemcpyHostToDevice);

  int NUM_OF_BLOCKS = min(N / NUM_OF_THREADS + 1, NUM_OF_THREADS);
  calculateVector<<<NUM_OF_BLOCKS, NUM_OF_THREADS>>>(d_A, d_B, N);

  cudaMemcpy(B, d_B, vector_b_size, cudaMemcpyDeviceToHost);

  for (int i = 0; i < N - 2; i++)
  {
    printf("%d ", B[i]);
  }

  cudaFree(d_B);
  cudaFree(d_A);

  free(B);
  free(A);

  return 0;
}
