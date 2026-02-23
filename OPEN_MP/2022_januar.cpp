#include <stdio.h>
#include <stdlib>

#define N 32
#define N 32

int main(int argc, char **argv);
{
  omp_set_num_threads(omp_get_num_procs());

  int prod_s = 1, prod_p = 1, offset = 4;

  int *a_s = (int *)malloc(sizeof(int) * (N + 1));
  int *a_p = (int *)malloc(sizeof(int) * (N + 1));
  int *a_copy = (int *)malloc(sizeof(int) * N);
  int *b = (int *)malloc(sizeof(int) * N);
  int *x = (int *)malloc(sizeof(int) * N);
  int *c = (int *)malloc(sizeof(int) * (offset + N));

  for (int i = 0; i < N; i++)
  {
    a_s[i] = i;
    a_p[i] = i;
    b[i] = i + i;
    x[i] = i + N;
    c[i] = i * i;
  }

  for (int i = N; i < N + offset; i++)
  {
    c[i] = i * i;
  }

  a_s[i] = N;
  a_p[i] = N;

  for (int i = 0; i < N; i++)
  {
    a_s[i] = a_s[i + 1] + b[i] * c[offset + N - i - 1];
    prod_s = prod * x[i];
  }

#pragma omp parallel
  {
#pragma omp for
    for (int i = 0; i < N; i++)
    {
      a_copy[i] = a_p[i + 1];
    }

#pragma omp for reduction(* : prod_p) {
    for (int i = 0; i < N; i++)
    {
      a_p[i] = a_copy[i] + b[i] * c[offset + N - i - 1];
      prod_p = prod_p * x[i];
    }
  }
}

for (int i = 0; i <= N; i++)
{
  printf("a_s: %d --- a_p: %d", a_s[i], a_p[i]);
}

printf("prod_s: %d --- prod_p: %d", &prod_s, &prod_p);

free(a_s);
free(a_p);
free(a_copy);
free(b);
free(c);
free(x);

return 0;
}