#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define NUM_THREADS 256

__global__ void monochrome(int *A, int *B, int n, int m);

int main()
{
  int rows, cols;
  printf("Unesite broj redova: \n");
  scanf("%d", &rows);
  printf("Unesite broj kolona: \n");
  scanf("%d", &cols);

  int *A = (int *)malloc(sizeof(int) * rows * cols);
  int *B = (int *)malloc(sizeof(int) * rows * cols);

  printf("Unesite vrednosti slike: \n");
  for (int i = 0; i < rows; i++)
    for (int j = 0; j < cols; j++)
      scanf("%d", A[i * cols + j]);

  int *d_A, *d_B;

  cudaMalloc((void **)&d_A, sizeof(int) * rows * cols);
  cudaMalloc((void **)&d_B, sizeof(int) * rows * cols);

  cudaMemcpy(d_A, A, sizeof(int) * rows * cols, cudaMemcpyHostToDevice);

  int num_of_blocks = (rows * cols + NUM_THREADS - 1) / NUM_THREADS;
  if (num_of_blocks > 1024)
    num_of_blocks = 1024;
  monochrome<<<num_of_blocks, NUM_THREADS>>>(d_A, d_B, rows, cols);

  cudaMemcpy(B, d_B, sizeof(int) * rows * cols, cudaMemcpyDeviceToHost);

  printf("Rezultujuca monohromatska slika: \n");
  for (int i = 0; i < rows; i++)
  {
    for (int j = 0; j < cols; j++)
      printf("%d ", B[i * cols + j]);

    printf("\n");
  }

  cudaFree(d_A);
  cudaFree(d_B);
  free(A);
  free(B);

  return 0;
}

__global__ void monochrome(int *A, int *B, int n, int m)
{
  int gid = blockIdx.x * blockDim.x + threadIdx.x;
  int tid = threadIdx.x;
  int stride = blockDim.x * gridDim.x;
  int total_elements = n * m;

  while (gid < total_elements)
  {

    int i = gid / m;
    int j = gid % m;

    int min_i = max(0, i - 1);
    int min_j = max(0, j - 1);
    int max_i = min(n - 1, i + 1);
    int max_j = min(m - 1, j + 1);

    if (A[i * m + j] <= 128)
    {
      B[i * m + j] = 0;
      gid += stride;

      continue;
    }

    int sum = 0;
    int count = 0;

    for (int ii = min_i; ii < max_i; ii++)
    {
      for (int jj = min_j; jj < max_j; jj++)
      {
        sum += A[ii * m + jj];
        count++;
      }
    }

    int average = sum / count;

    if (average > 64)
    {
      B[i * m + j] = 1;
    }
    else
    {
      B[i * m + j] = 0;
    }

    gid += stride;
  }
}