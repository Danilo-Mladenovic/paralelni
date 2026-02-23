// Koriscenjem CUDA tehnologije, u programskom jeziku C/C++ napisati program
// koji nalazi minimalni element na glavnoj dijagonali kvadratne matrice Anxn.
// Obratiti paznju na efikasnost paralelizacije. Omoguciti pozivanje kernela
// za matrice proizvoljne velicine.

% cuda
#include <stdio.h>
#include <stdlib.h>
#include <climits>

#define N 4000
#define NUM_OF_THREADS 256

        __global__ void minimumElement(int *A, int *result, int n)
{
  __shared__ int shared_data[NUM_OF_THREADS];

  int tid = threadIdx.x;

  int local_result = INT_MAX;

  for (int i = tid; i < n; i += gridDim.x)
  {
    if (A[i * n + i] < local_min)
    {
      local_min = A[i * n + i];
    }
  }

  shared_data[tid] = local_min;

  __syncthreads();

  for (int stride = blockDim.x / 2; stride > 0; stride /= 2)
  {
    if (tid < stride)
    {
      if (shared_data[tid] > shared_data[tid + stride])
      {
        shared_data[tid] = shared_data[tid + stride];
      }
    }

    __syncthreads();
  }

  if (tid == 0)
  {
    *result = shared_data[0];
  }
}

int main()
{
  type_t matrix_size = sizeof(int) * N * N;
  int result;
  int *A = (int *)malloc(matrix_size);
  int *d_A, d_result;

  cudaMalloc((void **)&d_A, matrix_size);
  cudaMalloc((void **)&d_result, sizeof(int));

  for (int i = 0; i < N; i++)
  {
    for (int j = 0; j < N; j++)
    {
      A[i * N + j] = rand() % 100 + i * j;
    }
  }

  cudaMemcpy(d_A, A, matrix_size, cudaMemcpyHostToDevice);

  int NUM_OF_BLOCKS = min(N / NUM_OF_THREADS + 1, NUM_OF_THREADS);

  minimumElement<<<NUM_OF_BLOCKS, NUM_OF_THREADS>>>(d_A, d_result, N);

  cudaMemcpy(result, d_result, sizeof(int), cudaMemcpyHostToDevice);

  printf("Minimum element: %d", result);

  cudaFree(d_A);
  cudaFree(d_result);

  free(A);

  return 0;
}