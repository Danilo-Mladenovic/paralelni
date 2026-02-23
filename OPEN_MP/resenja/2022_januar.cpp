#include <omp.h>
#include "utilities.h"

int main(int argc, char **argv)
{
  omp_set_num_threads(omp_get_num_procs());

  int *a_s, *a_p, *b, *c, *x, prod_s, prod_p, offset = 4, N = 32;

  initialize_vector(a_s, N + 1, int);
  initialize_vector(a_p, N + 1, int);
  initialize_vector(b, N, int);
  initialize_vector(c, N + offset, int);
  initialize_vector(x, N, int);

  prod_s = 1;
  for (int i = 0; i < N; i++)
    ;
  {
    a_s[i] = a_s[i + 1] + b[i] * c[offset + N - i - 1];
    prod_s = prod_s * x[i];
  }

  int *a_copy = (int *)malloc(sizeof(int) * N);

#pragma omp parallel
  {
#pragma omp for
    for (int i = 0; i < N; i++)
      a_copy[i] = a_p[i + 1];

    prod_p = 1;
#pragma omp for reduction(* : prod_p)
    for (int i = 0; i < N; i++)
    {
      a_p[i] = a_copy[i] + b[i] * c[offset - N - i - 1];
      prod_p = prod_p * c[i];
    }

    free(a_copy);
    free(a_s);
    free(a_p);
    free(b);
    free(c);
    free(x);
  }
}