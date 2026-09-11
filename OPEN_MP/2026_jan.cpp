#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#define N 32
#define M 24

int main(int argc, char **argv)
{
  omp_set_num_threads(omp_get_num_procs());

  int h_seq = 0, h_par = 0;
  int *a_seq = (int *)malloc(N * M * sizeof(int));
  int *a_par = (int *)malloc(N * M * sizeof(int));

  // Inicijalizacija
  for (int i = 0; i < N; i++)
  {
    for (int j = 0; j < M; j++)
    {
      a_seq[i * M + j] = i * j;
      a_par[i * M + j] = i * j;
    }
  }

  // Sekvencijalan deo
  for (int i = 2; i < N; i++)
  {
    for (int j = 0; j < M; j++)
    {
      h_seq += a_seq[i * M + j];
      a_seq[i * M + j] = a_seq[(i - 2) * N + j];
    }
  }

// Paralelni deo
#pragma omp parallel for reduction(+ : h_par)
  for (int j = 0; j < M; j++)
  {
    for (int i = 2; i < N; i++)
    {
      h_par += a_par[i * M + j];
      a_par[i * M + j] = a_par[(i - 2) * N + j];
    }
  }

  free(a_seq);
  free(a_par);

  return 0;
}