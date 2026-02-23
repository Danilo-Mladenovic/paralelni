#include <omp.h>
#include "utilities.h"

int main(int argc, char **argv)
{
  omp_set_num_threads(omp_get_num_procs());

  int x_s, x_p, *a, *b_s, *b_p, N = 3, M = 4;

  initialize_matrix(a, N, M, int);
  initialize_matrix(b_s, N, M, int);
  initialize_matrix(b_p, N, M, int);

  for (int i = 2; i < N; i++)
    for (int j = 0; j < M; j++)
      a[i][j] = i * j + j;

  x_s = 0;

  for (int i = 2; i < N; i++)
  {
    for (int j = 0; j < M; j++)
    {
      x_s = x_s + a[i][j];
      b_s[i][j] = a[i - 2][j];
    }
  }

  x_p = 0;

#pragma omp parallel for reduction(+ : x_p)
  for (int l = 2 * M; l < N * M; l++)
  {
    int i = l / M;
    int j = l % M;

    x_p = x_p + a[i][j];
    b_p[i][j] = a[i - 2][j];
  }

  cout << compare_matrix(c_s, c_b) ? 'same matrixes' : 'different matrixes' << endl;
  cout << x_s == x_p ? 'same sum' : 'different sum' << endl;

  destroy_matrix(a, N);
  destroy_matrix(b_s, N);
  destroy_matrix(c_p, N);

  return 0;
}