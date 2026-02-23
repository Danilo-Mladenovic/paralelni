#define n 64
#define BLOCKSIZE 4

int main()
{
	int a[n][n], b[n], *a_dev, *b_dev, b_mid[n][n / (2 * BLOCKSIZE)], *b_mid_dev;

	cudaMalloc(&a_dev, n * n * sizeof(int));
	cudaMemcpy(a_dev, b, n * n * sizeof(int), cudaMemcpyHostToDevice);
	cudaMalloc(&b_dev, n * sizeof(int));
	cudaMalloc(&b_mid_dev, n * (n / (2 * BLOCKSIZE)) * sizeof(int));

	dim3 dimGrid1(N / (BLOCKSIZE * 2), n / BLOCKSIZE);
	dim3 dimBlock1(BLOCKSIZE, BLOCKSIZE);
	kernel<<<dimGrid1, dimBlock1>>>(a_dev, b_mid_dev);
	cudaMemcpy(b_mid, b_mid_dev, n * (n / (2 * BLOCKSIZE)) * sizeof(int), cudaMemcpyDeviceToHost);

	dim3 dimGrid2(1, n / BLOCKSIZE);
	dim3 dimBlock2(n / (4 * BLOCKSIZE), BLOCKSIZE); // 4 jer opet pribavljamo 2 mem lokacije iz jedne niti
	kernel2<<<dimGrid2, dimBlock2>>>(b_mid_dev, b_dev);
	cudaMemcpy(b, b_dev, n * sizeof(int), cudaMemcpyDeviceToHost);

	cudaFree(a_dev);
	cudaFree(b_dev);
	cudaFree(b_mid_dev);
}

__global__ void kernel1(int *a, int *b)
{
	__shared__ int loc_a[BLOCKSIZE][BLOCKSIZE];
	int x = blockIdx.x * blockDim.x + threadIdx.x;
	int y = blockIdx.y * blockDim.y + threadIdx.y;

	loc_a[threadIdx.y][threadIdx.x] = fitsCriteria(a[y * n + x]) + fitsCriteria(a[y * n + x + n / 2]);
	__syncthreads();

	int len = blockDim.x;
	while (len > 1)
	{
		if (threadIdx.x < len / 2)
		{
			loc_a[threadIdx.y][threadIdx.x] += loc_a[threadIdx.y][threadIdx.x + len / 2];
		}
		len /= 2;
		_syncthreads();
	}

	if (threadIdx.x == 0)
	{
		b[y * gridDim.x + blockIdx.x] = loc_a[threadIdx.y][0];
	}
}

__device__ int fitsCriteria(int num)
{
	int cif = num;
	int br = 1;
	int sum = cif % 10;
	while (cif / 10 > 0)
	{
		cif /= 10;
		br++;
		sum += num % 10;
	}
	if (sum % br == 0)
		return 1;
	return 0;
}

__global__ void kernel2(int *a, int *b)
{
	__shared__ int loc_b_mid[BLOCKSIZE][n / (4 * BLOCKSIZE)] int x = blockIdx.x * blockDim.x + threadIdx.x;
	int y = blockIdx.y * blockDim.y + threadIdx.y;

	loc_b_mid[threadIdx.y][threadIdx.x] = a[y * gridDim.x * blockDim.x * 2 + x] + A[y * gridDim.x * blockDim.x * 2 + x + gridDim.x * blockDim.x];
	__syncthreads();

	int len = blockDim.x;
	while (len > 1)
	{
		if (threadIdx.x < len / 2)
			loc_b_mid[threadIdx.y][threadIdx.x] += loc_b_mid[threadIdx.y][threadIdx.x + len / 2];

		len /= 2;
		__syncthreads();
	}

	if (threadIdx.x == 0)
		b[y * gridDim.x + blockIdx.x] = loc_b_mid[threadIdx.y][0];
}