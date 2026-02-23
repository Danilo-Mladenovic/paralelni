#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#define N 32
#define M 24

int main(int argc, char **argv)
{
  omp_set_num_threads(omp_get_num_procs());

  int i, j, h_s = 0, h_p = 0;
  int *a_s, *a_p;

  a_s = (int *)malloc(sizeof(int) * N * M);
  a_p = (int *)malloc(sizeof(int) * N * M);

  // Initijalizacija
  for (i = 0; i < N; i++)
  {
    for (j = 0; j < M; j++)
    {
      a_s[i * M + j] = i;
      a_p[i * M + j] = i;
    }
  }

  // Sekvencijalno
  for (i = 2; i < N; i++)
  {
    for (j = 0; j < M; j++)
    {
      h_s += a_s[i * M + j];
      a_s[i * M + j] = a_s[(i - 2) * M + j];
    }
  }

// Paralelno
#pragma omp parallel for private(i) reduction(+ : h_p)
  for (j = 0; j < M; j++)
  {
    for (i = 2; i < N; i++)
    {
      h_p += a_p[i * M + j];
      a_p[i * M + j] = a_p[(i - 2) * M + j];
    }
  }

  free(a_s);
  free(a_p);

  return 0;
}