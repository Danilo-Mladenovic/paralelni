#include <omp.h>
#include "utilities.h"

int main(int argc, char **argv)
{
  int th_num = omp_get_num_procs();
  int t_s, t_p, *a_s, *a_p, *b, *c, N = 32;
  int *a_p_copy = (int *)malloc(sizeof(int) * (N - 1))

      for (int i = 0; i < N - 1; i++)
  {
    a_s[i] = a_s[i + 1] + b[i] * c[i];
    t_s = t_s * a_s[i];
  }

  omp_set_num_thread(th_num);

#pragma omp parallel
  {
#pragma omp for
    for (int i = 0; i < N - 1; i++)
      a_p_copy[i] = a_p[i + 1];

#pragma omp for reduction(* : t_p)
    for (int i = 0; i < N - 1; i++)
    {
      a_p[i] = a_p_copy[i] + b[i] * c[i];
      t_p = t_p * a_p[i];
    }
  }

  free(a_p_copy);
  free(a_s);
  free(a_p);
  free(b);
  free(c);
}