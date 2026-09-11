// Koriscenjem CUDA tehnologije, u programskom jeziku C/C++ napisati program koji medju N
// pravougaonika upamcenih u nizu A u sledecem formatu: [a1, ..., an, b1, ..., bn, c1, ...,
// cn, d1, ..., dn], odredjuje niz B, dimenzije N. U niz B, na i-tu proziciju, upisuje se
// 1 ukoliko se pravougaonik sa tim indeksom preklapa sa bilo kojim drugim pravougaonikom,
// a 0 ukoliko se ne preklapa. Broj pravougaonika unosi korisnik. Obratiti paznju na
// efikasnost paralelizacije. Omoguciti povezivanje kernela za matrice prozivoljne velicine.

% % cuda

#include <stdio.h>
#include <math.h>

#define NUM_OF_THREADS 256

        __device__ int intersection(int a1, int a2, int a3, int a4, int b1, int b2, int b3, int b4)
{
  // logika gde se proveri da li se preklapaju dva pravougaonika
  // vraca 1 ako se preklapaju i 0 ako se ne preklapaju
}

__global__ void squareIntersection(int *A, int *B, int n)
{
  int tid = threadIdx.x;
  int gid = blockDim.x * blockIdx.x + threadIdx.x;
  int stride = blockDim.x * gridDim.x;

  while (gid < n)
  {
    int hasIntersect = B[gid];

    // Posto for petlja upisuje 1 za oba kvadrata, moguce je da
    // je B[gid] vec postavljen na 1
    if (hasIntersect)
    {
      gid += stride;
      continue;
    }

    for (int i = gid * 4; i < n; i += 4)
    {

      int response = intersection(A[gid], A[gid + 1], A[gid + 2], A[gid + 3], A[i], A[i + 1], A[i + 2], A[i + 3]);

      if (response == 1)
      {
        B[gid] = 1;
        B[i / 4] = 1;
      }
    }

    gid += stride;
  }
}

int main()
{
  int n;

  printf("Unesite broj pravougaonika: ");
  scanf("%d", &n);

  size_t vector_size = n * sizeof(int);

  int *A = (int *)malloc(4 * vector_size);
  int *B = (int *)malloc(vector_size);

  for (int i = 0; i < n; i++)
  {
    printf("\n\nUnesite temena za %d pravougaonik:\n", i);

    for (int j = 0; j < 4; j++)
    {
      printf("Unesite teme %d", j);
      scanf("%d", &A[i * n + j]);
    }

    B[i] = 0;
  }

  int *d_A, *d_B;
  cudaMalloc((void **)&d_A, 4 * vector_size);
  cudaMalloc((void **)&d_B, vector_size);
  cudaMemcpy(d_A, A, 4 * vector_size, cudaMemcpyHostToDevice);
  cudaMemcpy(d_B, B, vector_size, cudaMemcpyHostToDevice);

  int num_of_blocks = min(n / NUM_OF_THREADS + 1, NUM_OF_THREADS);
  squareIntersection<<<num_of_blocks, NUM_OF_THREADS>>>(d_A, d_B, n);

  cudaMemcpy(B, d_B, vector_size, cudaMemcpyDeviceToHost);

  printf("Rezultujuci vektor: \n");
  for (int i = 0; i < n; i++)
  {
    printf("%d, ", B[i]);
  }

  cudaFree(d_A);
  cudaFree(d_B);

  free(A);
  free(B);

  return 0;
}