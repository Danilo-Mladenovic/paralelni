#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#define N 32
#define OFFSET 4

int main(int argc, char **argv)
{
  omp_set_num_threads(omp_get_num_procs());

  int sep_s = 1, sep_p = 1;
  int *A_s = (int *)malloc(N * sizeof(int));
  int *A_c = (int *)malloc((N - 1) * sizeof(int));
  int *A_p = (int *)malloc(N * sizeof(int));
  int *B = (int *)malloc((N - 1) * sizeof(int));
  int *T = (int *)malloc(N * sizeof(int));
  int *C = (int *)malloc((N + OFFSET) * sizeof(int));

  // Inicijalizacija
  for (int i = 0; i < N; i++)
  {
    A_s[i] = i;
    A_p[i] = i;
    T[i] = i * 2;
    C[i] = i * i;
    if (i != N - 1)
    {
      B[i] = i * 3;
    }
  }
  for (int i = N; i < N + OFFSET; i++)
  {
    C[i] = i * i;
  }

  // Sekvencijalni deo
  for (int i = 0; i < N - 1; i++)
  {
    A_s[i] = A_s[i + 1] + B[i] * C[OFFSET + N - i - 1];
    sep_s = sep_s * T[i + 1];
  }

  // Postoji WAR zavisnost izmedju A[i] i A[i + 1].
  // Resenje je da se napravi kopija niza A.
  // Paralelni deo
#pragma omp parallel for
  for (int i = 0; i < N - 1; i++)
  {
    A_c[i] = A_p[i + 1];
  }

#pragma omp parallel for reduction(* : sep_p)
  for (int i = 0; i < N - 1; i++)
  {
    A_p[i] = A_c[i] + B[i] * C[OFFSET + N - i - 1];
    sep_p = sep_p * T[i + 1];
  }

  // Provera resenja
  for (int i = 0; i < N; i++)
  {
    printf("Sequentual %d: %d | Parallel %d: %d.\n", i, A_s[i], i, A_p[i]);
  }

  printf("Sequental sep: %d | Parallel sep: %d", sep_s, sep_p);

  free(A_s);
  free(A_c);
  free(A_p);
  free(B);
  free(C);
  free(T);

  return 0;
}