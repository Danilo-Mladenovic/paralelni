#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define N 64
#define offset 4

int main(int argc, char **argv)
{
  omp_set_num_threads(omp_get_num_procs());

  int sep_s = 1, sep_p = 1;

  int *a_s = (int *)malloc(N * sizeof(int));
  int *a_p = (int *)malloc(N * sizeof(int));
  int *b = (int *)malloc((N - 1) * sizeof(int));
  int *a_copy = (int *)malloc((N - 1) * sizeof(int));
  int *c = (int *)malloc((N + offset - 1) * sizeof(int));
  int *t = (int *)malloc(N * sizeof(int));

  // nicijalizacija
  for (int i = 0; i < N; i++)
  {
    a_s[i] = i;
    a_p[i] = i;
    t[i] = rand() % 50;
    c[offset + i - 1] = rand() % 100;
    if (i != N - 1)
    {
      b[i] = rand() % 10;
    }
  }

  // sekvencijalno
  for (int i = 0; i < N; i++)
  {
    a_s[i] = a_s[i + 1] + b[i] * c[offset + N - i - 1];
    sep_s *= t[i + 1];
  }

// paralelno
#pragma omp parallel {
#pragma omp for
  for (int i = 0; i < N - 1; i++)
  {
    a_copy[i] = a_p[i + 1];
  }

#pragma omp for reduction(* : sep_p)
  for (int i = 0; i < N - 1; i++)
  {
    a_p[i] = a_c[i] + b[i] * c[offset + N - i - 1];
    sep_p *= t[i + 1];
  }
}

free(a_s);
free(a_copy);
free(a_p);
free(b);
free(c);
free(t);

return 1;
}