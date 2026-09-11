// Na programskom jeziku C/C++, koriscenjem CUDA tehnologije, napisati program koji u nizu
// A, duzine n, koji predstavlja RNK sekvencu, pronalazi trazenu podsekvencu, duzine m.
// Svaka RNK sekvenca i podsekvenca se sastoje od nukleotida (A, U, C, G). Rezultat upisati
// u niz B, koji na pocetku svake pronadjene sekvence upisuje 1, a na ostalim pozicijama 0.
// Obratiti paznju na efikasnost paralelizacije. Omoguciti povezivanje kernela za matrice
// proizvoljne velicine

% % cuda

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_OF_THREADS 256
#define N 4000
#define M 23

        __global__ void findSequence(char *A, char *S, int *B, int n, int m)
{
  __shared__ char s_S[M];

  int tid = threadIdx.x;

  // niti bloka zajedno ucitaju S u shared (svaka po nekoliko znakova)
  for (int i = tid; i < m; i += blockDim.x)
    s_S[i] = S[i];
  __syncthreads(); // sacekaj da je ceo S ucitan

  int gid = blockDim.x * blockIdx.x + tid;
  int stride = blockDim.x * gridDim.x;

  while (gid < n)
  {
    if (gid <= n - m)
    {
      int val = 1;
      for (int i = 0; i < m; i++)
      {
        if (A[gid + i] != s_S[i]) // citanje iz shareda, brzo
        {
          val = 0;
          break;
        }
      }
      B[gid] = val;
    }
    else
      B[gid] = 0;

    gid += stride;
  }
}

int main()
{
  char *A = (char *)malloc(N * sizeof(char));
  char *S = (char *)malloc(M * sizeof(char));
  int *B = (int *)malloc(N * sizeof(int));

  printf("Unesite RNK sekvencu: ");
  for (int i = 0; i < N; i++)
  {
    scanf("%c", &A[i]);
  }

  printf("Unesite podsekvencu: ");
  for (int i = 0; i < M; i++)
  {
    scanf("%c", &S[i]);
  }

  char *d_A, *d_S;
  int *d_B;

  cudaMalloc((void **)&d_A, N * sizeof(char));
  cudaMalloc((void **)&d_S, M * sizeof(char));
  cudaMalloc((void **)&d_B, N * sizeof(int));

  cudaMemcpy(d_A, A, N * sizeof(char), cudaMemcpyHostToDevice);
  cudaMemcpy(d_S, S, M * sizeof(char), cudaMemcpyHostToDevice);

  int num_of_block = min(N / NUM_OF_THREADS + 1, NUM_OF_THREADS);
  findSequence<<<num_of_block, NUM_OF_THREADS>>>(d_A, d_S, d_B, N, M);

  cudaMemcpy(B, d_B, N * sizeof(int), cudaMemcpyDeviceToHost);

  printf("Rezultati: \n");

  for (int i = 0; i < N; i++)
  {
    printf("%d, ", B[i]);
  }

  cudaFree(d_A);
  cudaFree(d_S);
  cudaFree(d_B);

  free(A);
  free(S);
  free(B);

  return 0;
}