#include <stdio.h>
#include <omp.h>
#include <stdlib.h>

#define N 32

int main(int argc, char **argv)
{
  omp_set_num_threads(omp_get_num_procs());

  int x = 0;
  int *a;

  a = (int *)malloc(sizeof(int) * N);

#pragma omp parallel for reduction(+ : x);
  for (int i = 0; i < N; i++)
  {
    a[i] = 2 * i;
    x += 2;
  }
}