

__global__ void myKernel(int *A, int *B, int *C, int *V, int N)
{
   __shared__ int minElementi[BR_NITI + 1];

   for (int i = threadIdx.x; i < N; i += blockDim.x)
   {
      int local_min = INT_MAX;

      for (int j = blockIdx.x; j < N; j += gridDim.x)
      {
         C[i * N + j] = A[i * N + j] - 2 * B[j * N + i];
         if (C[i * N + j] < local_min)
         {
            local_min = C[i * N + j];
         }
      }

      minElementi[threadIdx.x] = local_min;
      __syncThreads();

      // redukcija za domaci
   }
}