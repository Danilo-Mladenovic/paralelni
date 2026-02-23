% % cuda
#include <stdio.h>
#include <stdlib.h>
#define BR_NITI 4
#define N 20

    int main()
{
   int *A, *B, *d_A, *d_B, i, j;

   A = (int *)malloc(N * sizeof(int));
   B = (int *)malloc((N - 2) * sizeof(int));

   cudaMalloc((void **)&d_A, N * sizeof(int));
   cudaMalloc((void **)&d_B, (N - 2) * sizeof(int));

   for (i = 0; i < N; i++)
   {
      A[i] = rand() % 20;
   }

   cudaMemcpy(d_A, A, N * sizeof(int), cudaMemcpyHostToDevice);

   myKernel<<<N / BR_NITI + 1, BR_NITI>>>(d_A, d_B, N);

   cudaMemcpy(B, d_B, (N - 2) * sizeof(int), cudaMemcpyDeviceToHost);

   for (i = 0; i < N - 2; i++)
   {
      printf("%d", B[i]);
   }

   free(A);
   free(B);
   cudaFree(d_A);
   cudaFree(d_B);

   return 0;
}

__global__ void myKernel(int *A, int *B, int N)
{

   int tid = blockIdx.x * blockDim.x + threadIdx.x;

   int localIndex = threadIdx.x;
   __shared__ int alocal[BR_NITI + 2];

   if (tid < N)
   {
      alocal[localIndex] = A[tid]; // ovo localIndex ide uvek 0,1,2,3 ide na nivou bloka a A[tid] na globalnom nivou
   }

   if (localIndex == 0)
   {
      if (blockIdx.x * blockDim.x + BR_NITI < N)
      {
         alocal[BR_NITI] = A[blockIdx.x * blockDim.x + BR_NITI];
      }
      if (blockIdx.x * blockDim.x + BR_NITI + 1 < N)
      {
         alocal[BR_NITI + 1] = A[blockIdx.x * bloxkDim.x + BR_NITI + 1];
      }
   }

   __syncTreads();

   if (tid < N - 2)
   {
      B[tid] = (alocal[localIndex] * alocal[localIndex+2] * alocal[localIndex+2] / (alocal[localIndex] + alocal[localIndex+1] + alocal[localIndex+2]);
   }
}
