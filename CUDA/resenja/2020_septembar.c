int main()
{
	int a[n][n], b[n - 2][n - 2], *a_dev, *b_dev;
	cudaMalloc(a_dev, n * n * sizeof(int));
	cudaMemcpy(a_dev, a, n * n * sizeof(int), cudaMemcpyHostToDevice);
	cudaMalloc(b_dev, (n - 2) * (n - 2) * sizeof(int));

	dim3 gridDim(n / (BLOCKSIZE - 2), n / (BLOCKSIZE - 2));
	dim3 blockDim(BLOCKSIZE, BLOCKSIZE);
	kernel<<<gridDim, blockDim>>>(a_dev, b_dev);
	cudaMemcpy(b, b_dev, (n - 2) * (n - 2) * sizeof(int));

	cudaFree(a_dev);
	cudaFree(b_dev);
}

__global__ void kernel(int *a, int *b)
{
	__shared__ int loc_a[BLOCKSIZE][BLOCKSIZE];
	int x = (blockDim.x - 2) * blockIdx.x + threadIdx.x;
	int y = (blockDim.y - 2) * blockIdx.y + threadIdx.y;

	loc_a[threadIdx.y][threadIdx.x] = a[y * n + x];
	__syncthreads();

	int value;
	if (x > 0 && x < n - 1 && y > 0 && y < n - 1)
	{
		if (threadIdx.x > 0 && threadIdx.x < BLOCKSIZE - 2 && threadIdx.y > 0 &&
				threadIdx.y < BLOCKSIZE - 2)
		{
			value = (loc_a[threadIdx.x - 1][threadIdx.y - 1] + loc_a[threadIdx.x][threadIdx.y - 1] + ......) / 9;
			b[(n - 2) * (y - 1) + x - 1] = value;
		}
		__syncthreads();
	}
}