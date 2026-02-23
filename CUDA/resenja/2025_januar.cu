#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define NUM_THREADS 256

__global__ void standardna_devijacija_kernel(float *A, float *B, int rows, int cols);

int main()
{
  int rows, cols;
  printf("Unesite broj redova: ");
  scanf("%d", &rows);
  printf("Unesite broj kolona: ");
  scanf("%d", &cols);

  float *A = (float *)malloc(rows * cols * sizeof(float));
  float *B = (float *)malloc(rows * cols * sizeof(float));

  printf("Unesite piksele slike (0-255):\n");
  for (int i = 0; i < rows; i++)
  {
    for (int j = 0; j < cols; j++)
    {
      scanf("%f", &A[i * cols + j]);
    }
  }

  // Device memorija
  float *d_A;
  float *d_B;

  cudaMalloc((void **)&d_A, rows * cols * sizeof(float));
  cudaMalloc((void **)&d_B, rows * cols * sizeof(float));

  // Kopiranje na device
  cudaMemcpy(d_A, A, rows * cols * sizeof(float), cudaMemcpyHostToDevice);

  // Kernel launch - omoguciti rad sa proizvoljnom velicinom
  int num_elements = rows * cols;
  int num_blocks = (num_elements + NUM_THREADS - 1) / NUM_THREADS;
  if (num_blocks > 1024)
    num_blocks = 1024;

  standardna_devijacija_kernel<<<num_blocks, NUM_THREADS>>>(d_A, d_B, rows, cols);

  // Kopiranje rezultata nazad
  cudaMemcpy(B, d_B, rows * cols * sizeof(float), cudaMemcpyDeviceToHost);

  // Ispis rezultata
  printf("\nMatrica standardnih devijacija:\n");
  for (int i = 0; i < rows; i++)
  {
    for (int j = 0; j < cols; j++)
    {
      printf("%.2f ", B[i * cols + j]);
    }
    printf("\n");
  }

  // Oslobadjanje memorije
  free(A);
  free(B);
  cudaFree(d_A);
  cudaFree(d_B);

  return 0;
}

__global__ void standardna_devijacija_kernel(float *A, float *B, int rows, int cols)
{
  int idx = blockIdx.x * blockDim.x + threadIdx.x;
  int stride = gridDim.x * blockDim.x;
  int total_elements = rows * cols;

  while (idx < total_elements)
  {
    int i = idx / cols; // red
    int j = idx % cols; // kolona

    // Pronalazi granice 3x3 prozora
    int i_start = max(0, i - 1);
    int i_end = min(rows - 1, i + 1);
    int j_start = max(0, j - 1);
    int j_end = min(cols - 1, j + 1);

    // Izračunaj srednju vrednost
    float sum = 0.0f;
    int count = 0;
    for (int ii = i_start; ii <= i_end; ii++)
    {
      for (int jj = j_start; jj <= j_end; jj++)
      {
        sum += (float)A[ii * cols + jj];
        count++;
      }
    }
    float mean = sum / (float)count;

    // Izračunaj standardnu devijaciju
    float sum_sq_diff = 0.0f;
    for (int ii = i_start; ii <= i_end; ii++)
    {
      for (int jj = j_start; jj <= j_end; jj++)
      {
        float diff = (float)A[ii * cols + jj] - mean;
        sum_sq_diff += diff * diff;
      }
    }

    // σ² = (1/9) * Σ(x_ij - μ)²
    float variance = sum_sq_diff / (float)count;
    float stdev = sqrtf(variance);

    B[idx] = stdev;

    idx += stride;
  }
}
