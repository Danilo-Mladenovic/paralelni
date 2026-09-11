// Koriscenjem CUDA tehnologije, u programskom jeziku C/C++ napisati program
// koji nalazi minimalni element na glavnoj dijagonali kvadratne matrice Anxn.
// Obratiti paznju na efikasnost paralelizacije. Omoguciti pozivanje kernela
// za matrice proizvoljne velicine.

% % cuda

#include <stdio.h>
#include <math.h>

#define N 4000
#define NUM_OF_THREADS 256

        __global__ void findMinimum(int *A, int *min, int n)
{
  int tid = threadIdx.x;
  int gid = blockDim.x * blockIdx.x + tid;
  int stride = blockDim.x * gridDim.x;

  while (gid < n)
  {

    gid += stride;
  }
}

void main()
{
  size_t matrix_size = N * N * sizeof(int);
  int *A = (int *)malloc(matrix_size);
  int min = 0;

  for (int i = 0; i < N; i++)
  {
    for (int j = 0; j < N; j++)
    {
      A[i * N + j] = rand() % 100;
    }
  }

  int *d_A, *min;
  cudaMalloc((void **)&d_A, matrix_size);
  cudaMemcpy(d_A, A, matrix_size, cudaMemcpyHostToDevice);
  cudaMemcpy(d_min, &min, sizeof(int), cudaMemcpyHostToDevice);

  int num_of_blocks = min(N / NUM_OF_THREADS + 1, NUM_OF_THREADS);
  findMinimum<<<num_of_blocks, NUM_OF_THREADS>>>(d_A, &d_min, N);

  cudaMemcpy(&min, d_min, sizeof(int), cudaMemcpyDeviceToHost);

  printf("Minimalni elemnt na glavnoj dijagonali je: %d", &min);

  cudaFree(d_A);
  cudaFree(d_min);

  free(A);

  return 0;
}