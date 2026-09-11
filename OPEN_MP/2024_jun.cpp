#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#define N 32
#define POM 4

// c se indeksira sa POM + N - i - 1, za i od N-2 do 1 -> opseg [POM+1, POM+N-2]
#define C_SIZE (POM + N - 1)

int main(int argc, char **argv)
{
  int d_s = 42, d_p = 42;

  int *A_s = (int *)malloc(N * sizeof(int));
  int *A_p = (int *)malloc(N * sizeof(int));
  int *A_c = (int *)malloc(N * sizeof(int)); // kopija niza a
  int *B = (int *)malloc(N * sizeof(int));
  int *Z = (int *)malloc(N * sizeof(int));
  int *C = (int *)malloc(C_SIZE * sizeof(int));

  // Inicijalizacija
  for (int i = 0; i < N; i++)
  {
    A_s[i] = i;
    A_p[i] = i;
    B[i] = i + 1;
    Z[i] = 1 + (i % 2); // male vrednosti da ne dodje do prelivanja d
  }
  for (int i = 0; i < C_SIZE; i++)
  {
    C[i] = i * 2;
  }

  // Sekvencijalni deo
  for (int i = N - 2; i > 0; i--)
  {
    d_s = d_s * Z[i];
    A_s[i] = B[i] * C[POM + N - i - 1] - A_s[i - 1];
  }

  // Zavisnosti:
  //   1) d = d * z[i] -> loop-carried flow zavisnost po skalaru d.
  //      Rec je o multiplikativnoj redukciji -> reduction(* : d).
  //   2) a[i] = ... - a[i-1] -> loop-carried WAR (anti) zavisnost po a,
  //      distance 1. Petlja ide UNAZAD (i = N-2 .. 1), pa iteracija i CITA
  //      a[i-1] pre nego sto ga iteracija i-1 UPISE. Sekvencijalno se, dakle,
  //      uvek cita ORIGINALNA vrednost; paralelno taj redosled nije zagarantovan.
  //      Uklanja se kopijom niza a iz koje se cita.
  //   b, z, c se samo citaju -> nema zavisnosti po njima.

  // Paralelni deo - korak 1: kopija originalnih vrednosti niza a
#pragma omp parallel for
  for (int i = 0; i < N; i++)
  {
    A_c[i] = A_p[i];
  }
  // Implicitna barijera na kraju parallel for garantuje da je kopija
  // kompletna pre nego sto sledeca petlja pocne da pise po A_p.

  // Paralelni deo - korak 2
#pragma omp parallel for reduction(* : d_p)
  for (int i = N - 2; i > 0; i--)
  {
    d_p = d_p * Z[i];
    A_p[i] = B[i] * C[POM + N - i - 1] - A_c[i - 1];
  }

  // Provera resenja
  int ok = 1;
  for (int i = 0; i < N; i++)
  {
    if (A_s[i] != A_p[i])
    {
      ok = 0;
      printf("Razlika na i=%d: sekvencijalno %d | paralelno %d\n", i, A_s[i], A_p[i]);
    }
  }

  printf("Niz a: %s\n", ok ? "IDENTICAN" : "RAZLICIT");
  printf("Sekvencijalno d: %d | Paralelno d: %d -> %s\n",
         d_s, d_p, (d_s == d_p) ? "OK" : "RAZLIKA");

  free(A_s);
  free(A_p);
  free(A_c);
  free(B);
  free(Z);
  free(C);

  return 0;
}
