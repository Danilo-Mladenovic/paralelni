#include <stdio.h>
#include <stdlib>

#define N 32

int main(int argc, char **argv)
{
  omp_set_num_threads(omp_get_num_procs());

  int g_s = 0, g_p = 0;

  type_t vector_size = N * sizeof(int);

  int *y_s = (int *)malloc(matrix_size);
  int *y_p = (int *)malloc(matrix_size);
  int *x_s = (int *)malloc(matrix_size);
  int *x_p = (int *)malloc(vector_size);
  int *z = (int *)malloc(vector_size);

  for (int i = 0; i < N; i++)
  {
    y_s[i] = i;
    y_p[i] = i;
    x_s[i] = i + i;
    x_p[i] = i + i;
    z = i * i;
  }

  for (int i = 1; i < N; i++)
  {
    y_s[i] = y_s[i] + x_s[i - 1];
    x_s[i] = x_s[i] + z[i];
    g_s += z[i - 1];
  }

  y_p[1] = y_p[1] + x_p[0];
  g_p += z[0];

#pragma omp parallel for reduction(+ : g_p)
  for (int i = 2; i < N; i++)
  {
    x_p[i - 1] = x_p[i - 1] + z[i - 1];
    y_p[i] = y_p[i] + x_p[i - 1];
    g_p += z[i - 1];
  }

  x_p[N - 1] = x_p[N - 1] + z[N - 1];

  for (int i = 0; i < N; i++)
  {
    printf("y_s: %d --- x_p: %d", y_s[i], y_p[i]);
    printf("x_s: %d --- x_p: %d", x_s[i], x_p[i]);
  }

  printf("g_s: %d --- g_p: %d", &g_s, &g_p);

  free(y_s);
  free(y_p);
  free(x_s);
  free(x_p);
  free(z);

  return 0;
}