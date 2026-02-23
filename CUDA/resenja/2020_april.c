__global__ void kernel(int *a, int *k, int *a_r);

int main()
{
	int r[n][m], g[n][m], b[n][m], k[3][3];
	int rr[n][m], gr[n][m], br[n][m];
	int *dev, *dev_r, *k_dev;

	cudaMalloc(&dev, n * m * sizeof(int));
	cudaMalloc(&dev_r, n * m * sizeof(int));
	cudaMalloc(&k_dev, n * m * sizeof(int));
	cudaMemcpy(dev, r, n * m * sizeof(int), cudaMemcpyHostToDevice);
	cudaMemcpy(k_dev, k, n * m * sizeof(int), cudaMemcpyHostToDevice);

	dim3 gridDim(m / (BLOCKSIZE - 2), n / (BLOCKSIZE - 2));
	dim3 blockDim(BLOCKSIZE, BLOCKSIZE);
	kernel<<<gridDim, blockDim>>>(dev, k_dev, dev_r);
	cudaMemcpy(rr, dev_r, n * m * sizeof(int), cudaMemcpyDeviceToHost);

	cudaFree(dev);
	cudaFree(dev_r);
	cudaFree(k_dev);
}

__global__ void kernel(int *a, int *k, int *a_r)
{
	int x = blockIdx.x * (blockDim.x - 2) + threadIdx.x;
	int y = blockIdx.y * (blockDim.y - 2) + threadIdx.y;

	__shared__ int loc_a[BLOCKSIZE][BLOCKSIZE];
	__shared__ int loc_k[3][3];

	if (threadIdx.x < 3 && threadIdx.y < 3)
	{
		loc_k[threadIdx.y][threadIdx.x] = k[threadIdx.y * 3 + threadIdx.x];
	}
	__syncthreads();

	loc_a[threadIdx.y][threadIdx.x] = a[y * m + x];
	__syncthreads();

	if (threadIdx.x < BLOCKSIZE - 2 && threadIdx.y < BLOCKSIZE - 2 && threadIdx.x > 0 && threadIdx.y > 0)
	{
		int val;
		int i = threadIdx.y + 1;
		int j = threadIdx.x + 1;
		if (x > 0 && y > 0 && x < m && y < n)
		{
			val = loc_a[i - 1][j - 1] * loc_k[0][0] + .....
		}
		else
		{
			val = loc[threadIdx.y][threadIdx.x];
		}
		a_r[y * n + x] = val;
	}
}