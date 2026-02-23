% % cuda

#include <stdio.h>
#include <stdlib.h>

#define NUM_THREADS 256

        __global__ void check_heartbeat(float *Signal, int n, float alpha, int heartbeats);

int main()
{
  int n = 4000, heartbeats;
  float alpha;
  float *Signal = (float *)malloc(sizeof(float) * n);

  for (int i = 0; i < n; i++)
  {
    scanf("%f", &Signal[i]);
  }

  printf("Unesite prag za otkucaj srca: ");
  scanf("%f", &alpha);

  float *d_Signal;
  float d_alpha;
  int d_heartbeats;

  cudaMalloc((void **)&d_Signal, sizeof(float) * n);

  cudaMemcpy(d_Signal, Signal, sizeof(float) * n, cudaMemcpyHostToDevice);
  cudaMemcpy(&d_alpha, &alpha, sizeof(float), cudaMemcpyHostToDevice);

  int num_of_blocks = (n + NUM_THREADS - 1) / NUM_THREADS;
  if (num_of_blocks > 1024)
    num_of_blocks = 1024;

  check_heartbeat<<<num_of_blocks, NUM_THREADS>>>(d_Signal, n, d_alpha, d_heartbeats);

  cudaMemcpy(&heartbeats, &d_heartbeats, sizeof(int), cudaMemcpyDeviceToHost);

  printf("Broj otkucaja je: %d", heartbeats);

  cudaFree(d_Signal);
  free(Signal);

  return 0;
}

__global__ void check_heartbeat(float *Signal, int n, float alpha, int heartbeats)
{
  __shared__ int counts[NUM_THREADS];

  int idx = blockDim.x * blockIdx.x + threadIdx.x;
  int tid = threadIdx.x;
  int stride = blockDim.x * gridDim.x;

  int local_count = 0;

  while (idx < n)
  {
    if (idx == 0 || idx == n - 1)
    {
      continue;
    }

    if (Signal[idx] > Signal[idx - 1] && Signal[idx] > Signal[idx + 1] && Signal[idx] > alpha)
      local_count++;

    idx += stride;
  }

  counts[tid] = local_count;
  __syncThreads();

  for (int i = NUM_THREADS / 2; i > 0; i /= 2)
  {
    if (tid < i)
    {
      counts[tid] += counts[tid + i];
      __syncThreads();
    }
  }

  if (tid == 0)
  {
    atomicAdd(&heartbeats, counts[0]);
  }
}