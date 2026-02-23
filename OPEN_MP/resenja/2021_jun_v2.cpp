#include <omp.h>
#include "utilities.h"

int main(int argc, char **argv)
{
  omp_set_num_threads(omp_get_num_procs());

  int x_s, x_p, *res, *add, *sum_s, *sum_p, N = 32;

  initialize_vector(res, N - 2, int);
  initialize_vector(add, N - 2, int);
  initialize_vector(sum_s, N - 1, int);
  initialize_vector(sum_p, N - 1, int);

  for (int i = N - 1; i > 1; i--)
  {
    x_s = res[i] + add[i];
    sum_s[i] = sum[i - 1] + x_s;
  }

  int *sum_copy = (int *)malloc(sizeof(int) * (N - 2));

#pragma omp parallel for
  for (int i = N - 1; i > 1; i++)
    sum_copy[i] = sum_p[i - 1];

#pragma omp parallel for lastprivate(x_p)
  for (int i = N - 1; i > 1; i++)
  {
    x_p = res[i] + add[i];
    sum_p[i] = sum_copy[i] + x_p;
  }

  cout << compare_vectors(sum_s, sum_p, N - 1) ? "true" : "false" << endl;

  free(res);
  free(add);
  free(sum_s);
  free(sum_p);
  free(sum_copy);

  return 0;
}
