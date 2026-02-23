% % cuda

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define NUM_THREADS 256

        __global__ void find_interceptions(int *A, int *B, int N);

int main()
{
  int N;

  printf("Unesite N: ");
  scanf("%d", &N);

  int *A = (int *)malloc(sizeof(int) * 4 * N);
  int *B = (int *)malloc(sizeof(int) * N);

  // Inicijalizacija podataka
  printf("Unesite koordinate pravougaonika (a, b, c, d):\n");
  for (int i = 0; i < N; i++)
  {
    printf("Pravougaonik %d: ", i + 1);
    scanf("%d %d %d %d", &A[i], &A[N + i], &A[2 * N + i], &A[3 * N + i]);
  }

  int *d_A, *d_B;

  cudaMalloc((void **)&d_A, sizeof(int) * 4 * N);
  cudaMalloc((void **)&d_B, sizeof(int) * N);

  cudaMemcpy(d_A, A, sizeof(int) * 4 * N, cudaMemcpyHostToDevice);

  int num_of_blocks = (N + NUM_THREADS - 1) / NUM_THREADS;
  if (num_of_blocks > 1024)
    num_of_blocks = 1024;

  find_interceptions<<<num_of_blocks, NUM_THREADS>>>(d_A, d_B, N);

  cudaMemcpy(B, d_B, sizeof(int) * N, cudaMemcpyDeviceToHost);

  printf("Resenje: \n");
  for (int i = 0; i < N; i++)
    printf("Pravougaonik %d %s preklapanja\n", i + 1, B[i] == 1 ? "ima" : "nema");

  cudaFree(d_A);
  cudaFree(d_B);
  free(A);
  free(B);
}

__global__ void find_interceptions(int *A, int *B, int N)
{
  int tid = threadIdx.x;
  int gid = blockIdx.x * blockDim.x + tid;

  if (gid < N)
  {
    int a_i = A[gid];
    int b_i = A[N + gid];
    int c_i = A[2 * N + gid];
    int d_i = A[3 * N + gid];

    B[gid] = 0;

    for (int j = 0; j < N; j++)
    {
      if (gid != j)
      {
        int a_j = A[j];
        int b_j = A[N + j];
        int c_j = A[2 * N + j];
        int d_j = A[3 * N + j];

        if (max(a_i, a_j) < min(c_i, c_j) && max(b_i, b_j) < min(d_i, d_j))
        {
          B[gid] = 1;
          break;
        }
      }
    }
  }
}
