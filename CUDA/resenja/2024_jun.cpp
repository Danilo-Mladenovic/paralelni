% % cuda
#include <stdio.h>
#include <stdlib.h>
#define BR_NITI 256
#define N 1000

    typedef struct Kruznica
{
  int x, y, r;
}

int
main()
{
  Kruznica *kruznice, *najveca, *d_kruznice, *d_najveca;

  kruznice = (Kruznica *)malloc(N * sizeof(Kruznica));
  najveca = (Kruznica *)malloc(1 * sizeof(Kruznica));

  cudaMalloc((void **)&d_kruznice, N * sizeof(Kruznica));
  cudaMalloc((void **)&d_najveca, 1 * sizeof(Kruznica));

  for (i = 0; i < N; i++)
  {
    kruznice[i].x = rand() % 100;
    kruznice[i].y = rand() % 100;
    kruznice[i].r = rand() % 100;
  }

  cudaMemcpy(d_kruznice, kruznice, N * sizeof(Kruznica), cudaMemcpyHostToDevice);

  myKernel<<<1, BR_NITI>>>(d_kruznice, d_najveca, N);

  cudaMemcpy(&najveca, &d_najveca, sizeof(Kruznica), cudaMemcpyDeviceToHost);

  printf("%d%d%d", najveca.x, najveca.y, najveca.r);

  free(kruznice);
  free(najveca);
  cudaFree(d_kruznice);
  cudaFree(d_najveca);

  return 0;
}

a)

 __device__ Kruznica max (Kruznica* a,Kruznica*b)
{
  return a.r > b.r ? a : b;
}

__global__ void myKernel(Kruznica *d_kruznice, Kruznica *d_najveca, int N)
{

  __shared__ Kruznica localNajveceKruznice[BR_NITI + 1];

  Kruznica local_max = d_kruznice[0];

  for (int i = threadIdx.x; i < N; i += blockDim.x)
  {
    if (d_kruznice[i].r > local_max.r)
    {
      local_max = d_kruznice[i];
    }
  }

  localNajveceKruznice[threadIdx.x] = local_max;
  __syncThreads();

  I nacin :

      int index = threadIdx.x;
  int window = blockDim.x / 2;
  while (window > 1 && index < window)
  {
    if (localNajveceKruznice[index].r < localNajveceKruznice[index + window])
    {
      localNajveceKruznice[index] = localNajveceKruznice[index + window];
    }

    window /= 2;
    __syncThreads();
  }

  II nacin :

      int k = 2;
  int p = 1;

  while (p < BR_NITI)
  {
    if (threadIdx.x % k == 0)
    {
      Kruznica a = localNajveceKruznice[threadIdx.x];
      Kruznica b = localNajveceKruznice[threadIdx.x + p];
      localNajveceKruznice[threadIdx.x] = max(a, b);
    }

    __syncThreads();

    k *= 2;
    p *= 2;
  }

  if (threadIdx.x == 0)
  {
    d_Najveca = localNajveceKruznice[threadIdx.x];
  }
}

b)

%%cuda
#include <stdio.h>
#include <stdlib.h>
#define BR_NITI 256
#define N 1000

typedef struct Kruznica
{
  int x, y, r;
}

int
main()
{
  Kruznica *kruznice, *najveca, *d_kruznice, *d_najveca;

  kruznice = (Kruznica *)malloc(N * sizeof(Kruznica));
  najveca = (Kruznica *)malloc(2 * sizeof(Kruznica));

  cudaMalloc((void **)&d_kruznice, N * sizeof(Kruznica));
  cudaMalloc((void **)&d_najveca, 2 * sizeof(Kruznica));

  for (i = 0; i < N; i++)
  {
    kruznice[i].x = rand() % 100;
    kruznice[i].y = rand() % 100;
    kruznice[i].r = rand() % 100;
  }

  cudaMemcpy(d_kruznice, kruznice, N * sizeof(Kruznica), cudaMemcpyHostToDevice);

  myKernel<<<1, BR_NITI>>>(d_kruznice, d_najveca, N);

  cudaMemcpy(najveca, d_najveca, 2 * sizeof(Kruznica), cudaMemcpyDeviceToHost);

  printf("%d%d%d", najveca[0].x, najveca[0].y, najveca[0].r);
  printf("%d%d%d", najveca[1].x, najveca[1].y, najveca[1].r);

  free(kruznice);
  free(najveca);
  cudaFree(d_kruznice);
  cudaFree(d_najveca);

  return 0;
}

__device__ Kruznica(Kruznica *a, Kruznica *b)
{
  return a.r > b.r ? a : b;
}

__global__ void myKernel(Kruznica *d_kruznice, Kruznica *d_najveca, int N)
{

  __shared__ Kruznica localNajveceKruznice[BR_NITI + 1];
  __shared__ Kruznica indexiNajvecihKruznica[BR_NITI + 1]

      for (int i = 0; i < 2; i++)
  {

    Kruznica local_max = d_kruznice[0];
    int localIndexmax = 0;

    for (int i = threadIdx.x; i < N; i += blockDim.x)
    {
      if (d_kruznice[i].r > local_max.r)
      {
        local_max = d_kruznice[i];
        localIndexmax = i;
      }
    }

    localNajveceKruznice[threadIdx.x] = local_max;
    indexiNajvecihKruznica[threadIdx.x] = localIndexmax;

    __syncThreads();

    int k = 2;
    int p = 1;

    while (p < BR_NITI)
    {
      if (threadIdx.x % k == 0)
      {
        Kruznica a = localNajveceKruznice[threadIdx.x];
        Kruznica b = localNajveceKruznice[threadIdx.x + p];
        localNajveceKruznice[threadIdx.x] = max(a, b);
        indexNajveceKruznice[threadIdx.x] = localNajveceKruznice[threadIdx.x].r;
      }

      __syncThreads();

      k *= 2;
      p *= 2;
    }

    if (threadIdx.x == 0)
    {
      d_Najveca[i] = localNajveceKruznice[threadIdx.x];
      d_Kruznice[indexNajveceKruznice[threadIdx.x]] = -1;
    }
  }
}
