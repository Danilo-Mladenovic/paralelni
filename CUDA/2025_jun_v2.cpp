// Na programskom jeziku C/C++, koriscenjem CUDA tehnologije napisati program koji u matrici A,
// koja predstavlja monohromatsku sliku nocnog neba, pronalazi piksele koji potencijalno predsta-
// vljaju zvezde. Matrica se sastoji od piksela koji imaju vrednosti izmedju 0 i 255. Za svaki
// piksel koji ima vrednost vecu od 128, treba proveriti da li je prosecna vrednost njegovih
// susednih piksela (u okruzenju od po jednog piksela levo, gore, desno, dole i dijagonalno),
// veca od 64. Ukoliko jeste, na poziciju piksela u rezultujucoj matrici B upisati 1, a ukoliko
// nije, 0. Obratiti paznju na efikasnost paralelizacije. Omoguciti povezivanje kernela za
// matrice proizvoljne velicine.

% % cuda

#include <stdio.h>
#include <math.h>

#define N 4000
#define NUM_OF_THREADS 256

        __global__ void monochrome(int *A, int *B, int n)
{
  int tid = threadIdx.x;
  int gid = blockIdx.x * blockDim.x + tid;
  int stride = blockDim.x * gridDim.x;

  while (gid < n * n)
  {
    int i = gid / n;
    int j = gid % n;

    if (A[i * n + j] > 128)
    {
      int min_i = max(0, i - 1);
      int min_j = max(0, j - 1);
      int max_i = min(n - 1, i + 1);
      int max_j = min(n - 1, j + 1);

      int num_of_elements = 0;
      int sum = 0;

      for (int ii = min_i; ii <= max_i; ii++)
      {
        for (int jj = min_j; jj <= max_j; jj++)
        {
          if (ii == i && jj == j)
            continue;

          sum += A[ii * n + jj];
          num_of_elements++;
        }
      }

      int average = sum / num_of_elements;
      B[i * n + j] = average > 64 ? 1 : 0;
    }
    else
    {
      B[i * n + j] = 0;
    }

    gid += stride;
  }
}

int main()
{
  size_t matrix_size = N * N * sizeof(int);

  int *A = (int *)malloc(matrix_size);
  int *B = (int *)malloc(matrix_size);

  printf("Unesite vrednosti matrice A u opsegu od 0 do 255: ");
  for (int i = 0; i < N; i++)
  {
    for (int j = 0; j < N; j++)
    {
      scanf("%d", &A[i * N + j]);
    }
  }

  int *d_A, *d_B;
  cudaMalloc((void **)&d_A, matrix_size);
  cudaMalloc((void **)&d_B, matrix_size);
  cudaMemcpy(d_A, A, matrix_size, cudaMemcpyHostToDevice);

  int num_of_blocks = min(N / NUM_OF_THREADS + 1, NUM_OF_THREADS);
  monochrome<<<num_of_blocks, NUM_OF_THREADS>>>(d_A, d_B, N);

  cudaMemcpy(B, d_B, matrix_size, cudaMemcpyDeviceToHost);

  printf("Rezultujuca matrica B: ");
  for (int i = 0; i < N; i++)
  {
    for (int j = 0; j < N; j++)
    {
      printf("%d, ", B[i * N + j]);
    }
    printf("\n");
  }

  cudaFree(d_A);
  cudaFree(d_B);

  free(A);
  free(B);

  return 0;
}