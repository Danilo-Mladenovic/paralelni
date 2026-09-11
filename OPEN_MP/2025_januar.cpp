#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#define M 256
#define K 8

// Najveci indeks kojim se pristupa nizu e je t = 3 + (M*M - 1)*K
#define E_SIZE (3 + (M * M - 1) * K + 1)

int main(int argc, char **argv)
{
  int t_s = 3, t_p = 3, k = K;

  int *C_s = (int *)malloc(M * sizeof(int));
  int *C_p = (int *)malloc(M * sizeof(int));
  int *E = (int *)malloc(E_SIZE * sizeof(int));

  // Inicijalizacija
  for (int j = 0; j < M; j++)
  {
    C_s[j] = j;
    C_p[j] = j;
  }
  for (int i = 0; i < E_SIZE; i++)
  {
    E[i] = i;
  }

  // Sekvencijalni deo
  for (int i = 0; i < M; i++)
  {
    for (int j = 0; j < M; j++)
    {
      C_s[j] += E[t_s];
      t_s += k;
    }
  }

  // Zavisnosti:
  //   1) t += k  -> loop-carried flow zavisnost po skalaru t (indukciona
  //      promenljiva). Uklanja se zatvorenom formulom: za linearizovani
  //      redni broj iteracije n = i*M + j vazi t = 3 + n*k.
  //   2) c[j] += e[t] -> loop-carried flow (RAW) zavisnost po c[j], koju
  //      nosi SPOLJNA petlja po i (isto j, razlicito i). Po j je nema.
  // Zamenom petlji zavisnost po c[j] ostaje unutar jedne niti, pa je
  // spoljna petlja po j paralelizabilna.

  // Paralelni deo
#pragma omp parallel for
  for (int j = 0; j < M; j++)
  {
    for (int i = 0; i < M; i++)
    {
      C_p[j] += E[3 + (i * M + j) * k];
    }
  }

  // t nakon petlje ima istu zatvorenu formu (M*M iteracija)
  t_p = 3 + M * M * k;

  // Provera resenja
  int ok = 1;
  for (int j = 0; j < M; j++)
  {
    if (C_s[j] != C_p[j])
    {
      ok = 0;
      printf("Razlika na j=%d: sekvencijalno %d | paralelno %d\n", j, C_s[j], C_p[j]);
    }
  }

  printf("Nizovi c: %s\n", ok ? "IDENTICNI" : "RAZLICITI");
  printf("Sekvencijalno t: %d | Paralelno t: %d -> %s\n",
         t_s, t_p, (t_s == t_p) ? "OK" : "RAZLIKA");

  free(C_s);
  free(C_p);
  free(E);

  return 0;
}
