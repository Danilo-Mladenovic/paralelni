int main()
{
	int a[n][n], b[n], *a_dev, *b_dev, c[n][n / BLOCKSIZE], *c_dev;

	cudaMalloc(a_dev, n * n * sizeof(int));
	cudaMemcpy(a_dev, a, n * n * sizeof(int), cudaMemcpyHostToDevice);
	cudaMalloc(b_dev, n * sizeof(int));
	cudaMalloc(b_mid_dev, n * (n / BLOCKSIZE) * sizeof(int));

	dim3 gridDim(n / BLOCKSIZE, n / BLOCKSIZE);
	dim3 blockDim(BLOCKSIZE, BLOCKSIZE);
	nadjiMinMax<<<gridDim, blockDim>>>(a_dev, c_dev);
	dim3 gridDim(1, n / BLOCKSIZE);
	dim3 blockDim(n / BLOCKSIZE, BLOCKSIZE);
	saberiMinMax<<<gridDim, blockDim>>>(c_dev, b_dev);
	cudaMemcpy(b, b_dev, n * sizeof(int), cudaMemcpyDeviceToHost);

	dim3 gridDim(n / BLOCKSIZE, n / BLOCKSIZE);
	dim3 blockDim(BLOCKSIZE, BLOCKSIZE);
	zameniVrednosti<<<gridDim, blockDim>>>(a_dev, b_dev);
	cudaMemcpy(a, a_dev, n * sizeof(int), cudaMemcpyDeviceToHost);

	cudaFree(a_dev);
	cudaFree(b_dev);
	cudaFree(c_dev);
}

__global__ void nadjiMinMax(int *a, int *c)
{
	__shared__ int loc_a[BLOCKSIZE][BLOCKSIZE];
	int x = blockIdx.x * blockDim.x + threadIdx.x;
	int y = blockIdx.y * blockDim.y + threadIdx.y;

	loc_a[threadIdx.y][threadIdx.y] = getVal(a[y * n + x]);
	__syncthreads();

	int len = BLOCKSIZE;
	while (len > 1)
	{
		if (threadIdx.x < len / 2)
		{
			loc_a[threadIdx.y][threadIdx.x] += loc_a[threadIdx.y][threadIdx.x + len / 2];
		}
		len /= 2;
		__syncthreads();
	}

	if (threadIdx.x == 0)
	{
		c[n / BLOCKSIZE * y + blockIdx.x] = loc[threadIdx.y][0];
	}
}

__device__ int getVal(int val)
{
	if (val < 0)
		return -1;
	else if (val > 0)
		return 1;
	return val;
}

__global__ void saberiMinMax(int *c, int *b)
{
	__shared__ int loc_c[BLOCKSIZE][n / BLOCKSIZE];
	int x = blockIdx.x * blockDim.x + threadIdx.x;
	int y = blockIdx.y * blockDim.y + threadIdx.y;

	loc_c[threadIdx.y][threadIdx.x] = c[y * (n / BLOCKSIZE) + threadIdx.x];
	__syncthreads();

	int len = blockDim.x;
	while (len > 1)
	{
		if (threadIdx.x < len / 2)
		{
			loc_c[threadIdx.y][threadIdx.x] += loc_c[threadIdx.y][threadIdx.x + len / 2]
		}
		len /= 2;
		__syncthreads();
	}
	if (threadIdx.x == 0)
	{
		b[y] = loc_c[threadIdx.y][0];
	}
}

__global__ void zameniVrednosti(int *a, int *b)
{
	__shared__ int loc_a[BLOCKSIZE][BLOCKSIZE];
	__shared__ int loc_b[BLOCKSIZE];

	int x = blockDim.x * blockIdx.x + threadIdx.x;
	int y = blockDim.y * blockIdx.y + threadIdx.y;

	loc_a[threadIdx.y][threadIdx.x] = a[y * n + x];
	__syncthreads();

	if (threadIdx.y == 0)
	{
		loc_b[threadIdx.x] = b[y];
	}

	if (loc_a[threadIdx.y][threadIdx.x] < 0)
	{
		loc_a[threadIdx.y][threadIdx.x] = loc_b[y];
	}

	a[y * n + x] = loc_a[threadIdx.y][threadIdx.x];
}