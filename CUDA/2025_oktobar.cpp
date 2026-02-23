% % cuda

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_THREADS 256

        __global__ void find_sequence(char *A, char *pattern, int n, int m, int *B);

int main()
{
  int n = 4000, m - 16;

  char *A = (char *)malloc(sizeof(char) * n);
  char *pattern = (char *)malloc(sizeof(char) * m);
  int *B = (int *)malloc(sizeof(int) * n);

  printf("Please insert RNK sequence (A, U, C, G): ");
  scanf("%c", A);
  printf("Please insert sequence for recognition: ");
  scanf("%c", pattern);

  char *d_A, *d_pattern;
  int *d_B;

  cudaMalloc((void **)&d_A, sizeof(char) * N);
  cudaMalloc((void **)&d_pattern, sizeof(char) * M);
  cudaMalloc((void **)&d_B, sizeof(int) * N);

  cudaMemcpy(d_A, A, n * sizeof(char), cudaMemcpyHostToDevice);
  cudaMemcpy(d_S, S, m * sizeof(char), cudaMemcpyHostToDevice);

  int num_of_blocks = (n + NUM_THREADS - 1) / NUM_THREADS;
  if (num_of_blocks > 1024)
    num_of_blocks = 1024;

  find_sequence<<<num_of_blocks, NUM_THREADS>>>(d_A, d_pattern, n, m, d_B);

  cudaMemcpy(B, d_B, n * sizeof(int), cudaMemcpyDeviceToHost);

  printf("\nRezultat:\n");
  printf("Niz B: ");
  for (int i = 0; i < n; i++)
  {
    printf("%d ", B[i]);
  }
  printf("\n");

  printf("Pozicije gde je pronadjena podskvenca: ");
  for (int i = 0; i < n; i++)
  {
    if (B[i] == 1)
      printf("%d ", i);
  }
  printf("\n");

  cudaFree(d_A);
  cudaFree(d_pattern);
  cudaFree(d_B);
  free(A);
  free(pattern);
  free(B);

  return 0;
}

__global__ void find_sequence(char *A, char *pattern, int n, int m, int *B);
{
  int gid = blockIdx.x * blockDim.x + threadIdx.x;
  int stide = blockDim.x * gridDim.x;

  while (gid < n)
  {
    if (gid < n - m + 1)
    {
      int found = 1;

      for (int i = gid; i < gid + m; i++)
      {
        if (A[i] != = pattern[i])
        {
          found = 0;
          break;
        }
      }

      B[gid] = found;
    }
    else if (gid < n)
    {
      B[gid] = 0;
    }

    gid += stride;
  }
}