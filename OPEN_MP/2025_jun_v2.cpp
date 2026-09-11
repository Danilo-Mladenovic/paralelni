#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 256

int main(int argc, char **argv)
{
  omp_set_num_threads(omp_get_num_procs());

  int a_s = 6, a_p = 6;

  int *B_s = (int *)malloc(N * sizeof(int));
  int *B_p = (int *)malloc(N * sizeof(int));

  // Inicijalizacija
  for (int i = 0; i < N; i++)
  {
    B_s[i] = i;
    B_p[i] = i;
  }

  // Sekvencijalni deo
  for (int i = 0; i < N; i++)
  {
    if (i >= N / 2)
    {
      B_s[i] = B_s[i - 1];
    }

    B_s[i]++;
    a_s *= 4;
  }

// Postoji loop-carry RAW zavisnost izmedju b[i] i b[i - 1]
// Paralelni deo:
#pragma omp parallel for
  for (int i = 0; i < N / 2; i++)
  {
    B_p[i]++;
  }

  int base_value = B_p[N / 2 - 1];

#pragma omp parallel for
  for (int i = 0; i < N / 2; i++)
  {
    B[N / 2 + i] = base_value + i + 1;
  }

#pragma omp parallel for reduction(* : a_p)
  for (int i = 0; i < N; i++)
  {
    a_p *= 4;
  }

  // Provera resenja:
  for (int i = 0; i < N; i++)
  {
    printf("%d: Sequental - %d | Parallel - %d.\n", i, B_s[i], B_p[i]);
  }

  printf("Sequental a: %d | Parallel a: %d.", a_s, a_p);

  free(B_s);
  free(B_p);

  return 0;
}