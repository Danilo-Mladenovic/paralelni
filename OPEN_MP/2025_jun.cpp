#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#define N 9

int main(int argc, char **argv)
{
  omp_set_num_threads(omp_get_num_procs());

  int i, a_s = 6, a_p = 6;
  int *b_s, *b_p;

  b_s = (int *)malloc(sizeof(int) * N);
  b_p = (int *)malloc(sizeof(int) * N);

  for (i = 0; i < N; i++)
  {
    b_s[i] = i;
    b_p[i] = i;
  }

  for (i = 0; i < N; i++)
  {
    if (i >= N / 2)
      b_s[i] = b_s[i - 1];

    b_s[i]++;
    a_s *= 4;
  }

  for (int i = 0; i < N; i++)
  {
    if (i >= N / 2)
      b_p[i] = b_p[i - 1];

    b_p[i]++;
  }

#pragma omp parallel for firstprivate(a_p) reduction(* : a_p)
  for (i = 0; i < N; i++)
  {
    a_p *= 4;
  }

  free(b_s);
  free(b_p);

  return 0;
}
