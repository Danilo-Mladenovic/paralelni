// Koriscenjem CUDA tehnologije na programskom jeziku C/C++ napisati program koji detektuje i
// prebrojava otkucaje srca iz niza podataka koji predstavlja EKG signal. Signal je niz realnih
// brojeva koji predstavljaju napon u jedinici vremena procitan sa EKG uredjaja. Otkucaj srca
// je predstavljen lokalnim maksimumom, cija je vrednost veca od susednih vrednosti sa leve
// i desne strane i premesuje unapred definisan prag alfa, kako bi se izbegli sumovi. Obratiti
// praznju na efikasnost paralelizacije. Omoguci ti povezivanje kernela za nizove proizvoljne
// velicine.

% % cuda
#include <stdio.h>
#include <math.h>

#define N 4000
#define NUM_OF_THREADS 256

        __global__ void countHeartbeats(float *ekg, int n, float alpha, int *count)
{
  int tid = threadIdx.x;
  int gid = blockIdx.x * blockDim.x + tid;
  int stride = blockDim.x * gridDim.x;

  while (gid < n)
  {
    if (gid == 0 || gid == n - 1)
    {
      gid += stride;
      continue;
    }

    float value = ekg[gid];
    float prevValue = ekg[gid - 1];
    float nextValue = ekg[gid + 1];

    if (value > prevValue && value > nextValue && value > alpha)
    {
      atomicAdd(count, 1);
    }

    gid += stride;
  }
}

int main()
{
  float *ekg = (float *)malloc(N * sizeof(float));
  int heartbeats = 0;
  float alpha;

  printf("Unesite prag alfa: ");
  scanf("%f", &alpha);

  printf("Unesite EKG vrednosti: ");
  for (int i = 0; i < N; i++)
  {
    scanf("%f", &ekg[i]);
  }

  float *d_ekg;
  int *d_heartbeats;
  cudaMalloc((void **)&d_ekg, N * sizeof(float));
  cudaMalloc((void **)&d_heartbeats, sizeof(int));
  cudaMemcpy(d_ekg, ekg, N * sizeof(float), cudaMemcpyHostToDevice);
  cudaMemcpy(d_heartbeats, &heartbeats, sizeof(int), cudaMemcpyHostToDevice);

  int num_of_blocks = min(N / NUM_OF_THREADS + 1, NUM_OF_THREADS);
  countHeartbeats<<<num_of_blocks, NUM_OF_THREADS>>>(d_ekg, N, alpha, d_heartbeats);

  cudaMemcpy(&heartbeats, d_heartbeats, sizeof(int), cudaMemcpyDeviceToHost);

  printf("Broj otkucaja je: %d", heartbeats);

  cudaFree(d_ekg);
  cudaFree(d_heartbeats);
  free(ekg);

  return 0;
}
