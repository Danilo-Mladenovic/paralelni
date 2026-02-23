#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <stdboo.h>

#define N 32
#define offset 4

int main(int argc, char **argv)
{
  omp_set_num_threads(omp_get_num_procs());

  int sep_s = 1, sep_p = 1;
  int *a_s, *a_p, *a_copy, *b, *c, *t;

  a_s = (int *)malloc(N * sizeof(int));
  a_p = (int *)malloc(N * sizeof(int));
  a_copy = (int *)malloc((N - 1) * sizeof(int));
  b = (int *)malloc((N - 1) * sizeof(int));
  c = (int *)malloc((N + offset) * sizeof(int));
  t = (int *)malloc(N * sizeof(int));

  // Inicijalizacija
  for (int i = 0; i < N; i++)
  {
    a_s[i] = i;
    a_p[i] = i;
    c[offset + i] = i + offset;
    t[i] = i * 4;

    if (i < N - 1)
    {
      b[i] = i * 2;
    }
  }

  // Sekvencijalno izvrsenje
  for (int i = 0; i < N - 1; i++)
  {
    a_s[i] = a_s[i + 1] + b[i] * c[offset + N - i - 1];
    sep_s = sep_s * t[i + 1];
  }

// Kopiranje niza a
#pragma omp parallel for
  for (int i = 0; i < N - 1; i++)
  {
    a_copy[i] = a_p[i + 1];
  }

// Paralelno izvrsenje
#pragma omp parallel for reduction(* : sep_p)
  for (int i = 0; i < N - 1; i++)
  {
    a_p[i] = a_copy[i] + b[i] * c[offset + N - i - 1];
    sep_p = sep_p * t[i + 1];
  }

  bool equal = true;

  for (int i = 0; i < N - 1; i++)
  {
    if (a_s[i] != a_p[i])
    {
      equal = false;
    }
  }

  if (equal)
  {
    printf("Nizovi su isti.");
  }
  else
  {
    printf("Nizovi nisu isti.");
  }

  free(a_s);
  free(a_p);
  free(a_copy);
  free(b);
  free(c);
  free(t);

  return 0;
}