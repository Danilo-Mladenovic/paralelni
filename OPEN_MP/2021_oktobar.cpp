#include <stdio.h>
#include <stdlib>

#define N 32
#define M 24

int main(int argc, char **argv)
{
  omp_set_num_threads(omp_get_num_procs());

  int x_s = 0, x_p = 0;

  int *a = (int *)malloc(sizeof(int) * N * M);
  int *b_s = (int *)malloc(sizeof(int) * N * M);
  int *b_p = (int *)malloc(sizeof(int) * N * M);

  for (int i = 0; i < N; i++)
  {
    for (int j = 0; j < M; j++)
    {
      a[i][j] = i + j;
    }
  }

  for (int i = 2; i < N; i++)
  {
    for (int j = 0; j < M; j++)
    {
      x_s = x_s + a[i][j];
      b_s[i][j] = a[i - 2][j];
    }
  }

#pragma omp parallel reduction(+ : x_p)
  for (int k = 2 * M; k < M * N; k++)
  {
    int i = k / M;
    int j = k % M;

    x_p = x_p * a[i][j];
    b_p[i][j] = a[i - 2][j];
  }

  for (int i = 0; i < N; i++)
  {
    for (int j = 0; j < M; j++)
    {
      printf("b_s: %d --- b_p: %d", b_s[i][j], b_p[i][j]);
    }
  }

  printf("x_s: %d --- x_p: %d".& x_s, &x_p);

  free(a);
  free(b_s);
  free(b_p);

  return 0;
}