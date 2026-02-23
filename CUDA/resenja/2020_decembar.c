__global__ void uzastopniElementi(int *a, int *p);
__global__ void minimum(int *p, int *b);

int main()
{
	int a[n], p[n - m + 1], b, *a_dev, *p_dev, *b_dev;

	cudaMalloc(a_dev, n * sizeof(int));
	cudaMalloc(p_dev, (n - m + 1) * sizeof(int));
	cudaMalloc(b_dev, sizeof(int));
	cudaMemcpy(a_dev, a, n * sizeof(int), cudaMemcpyHostToDevice);

	uzastopniElementi<<<(n - m + 1) / (BLOCKSIZE - m + 1), BLOCKSIZE>>>(a_dev, p_dev);
	cudaMemcpy(p, p_dev, (n - m + 1) * sizeof(int), cudaMemcpyDeviceToHost);

	minimum<<<(n - m + 1) / BLOCKSIZE, BLOCKSIZE>>>(p_dev, p_dev);
	minimum<<<1, (n - m + 1) / BLOCKSIZE>>>(p_dev, b_dev);
	cudaMemcpy(b, b_dev, sizeof(int), cudaMemcpyDeviceToHost);

	cudaFree(a_dev);
	cudaFree(p_dev);
	cudaFree(b(dev);
}

__global__ void uzastopniElementi(int *a, int *p)
{
	__shared__ int loc_a[BLOCKSIZE];
	__shared__ int loc_p[BLOCKSIZE - m + 1];
	int x = blockIdx.x * (blockDim.x - m + 1) + threadIdx.x;

	loc_a[threadIdx.x] = a[x];
	__syncthreads();

	if (threadIdx.x < BLOCKSIZE - m + 1)
	{
		loc_p[threadIdx.x] = 0;
	}
	__syncthreads();

	for (int i = threadIdx.x - m + 1; i <= threadIdx.x; i++)
	{
		if (i >= 0 && i < BLOCKSIZE - m + 1)
		{
			loc_p[i] += loc_a[threadIdx.x];
		}
		__syncthreads();
	}

	if (threadIdx.x < BLOCKSIZE - m + 1)
	{
		p[x] = loc_p[threadIdx.x];
	}
}

__global__ void minimum(int *p, int *b)
{
	__shared__ int loc_p[BLOCKSIZE];
	int x = blockDim.x * blockIdx.x + threadIdx.x;

	loc_p[threadIdx.x] = p[x];
	__syncthreads();

	int len = blockDim.x;
	while (len > 0)
	{
		if (threadIdx.x < len / 2)
		{
			loc_p[threadIdx.x] = minimalno(loc_p[threadIdx.x], loc_p[threadIdx.x + len / 2]);
		}
		len /= 2;
		__syncthreads();
	}
	if (threadIdx.x == 0)
	{
		b[threadIdx.x] = loc_p[0];
	}
}

__device__ int minimalno(int a, int b)
{
	return a < b ? a : b;
}
