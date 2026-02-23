#include <omp.h>
#include "utilities.h"

int main(int argc, char **argv)
{
  omp_set_num_threads(omp_get_num_procs());

  int *x_s, *x_p, *y_s, *y_p, *z, g_s, g_p, N = 32;

  initialize_vector(x_s, N, int);
  initialize_vector(x_p, N, int);
  initialize_vector(y_s, N, int);
  initialize_vector(y_p, N, int);
  initialize_vector(z, N, int);

  g_s = 0;

  for (int i = 1; i < N; i++)
  {
    y_s[i] = y_s[i] + x_s[i - 1];
    x_s[i] = x_s[i] + z[i];
    g_s += z[i - 1];
  }

  g_p = 0;

  y_p[1] = y_p[1] + x_p[0];
  g_p += z[0];

#pragma omp parallel for reduction(+ : g_p)
  for (int i = 2; i < N; i++)
    [x_p[i - 1] = x_p[i - 1] + z[i - 1];
        y_p[i] = y_p[i] + x_p[i - 1];
        g_p += z[i - 1];
    ] x_p[N - 1] = x_p[N - 1] + z[N - 1];

  // print and compare

  free(x_s);
  free(x_p);
  free(y_s);
  free(y_p);
  free(z);

  return 0;
}