#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>

#define MCW MPI_COMM_WORLD
#define K 8
#define N 6

int main(int argc, char **argv)
{
  MPI_Init(&argc, &argv);
  int rank, size;
  MPI_Comm_rank(MCW, &rank);
  MPI_Comm_size(MCW, &size);

  MPI_Status st;
  int num_of_rows = pow(2, rank);
  int A[K][N], b[N], d[N];
  int *A_local = (int *)malloc(num_of_rows * N * sizeof(int));
  int *d_local = (int *)malloc(num_of_rows * sizeof(int));

  if (rank == 0)
  {
    for (int i = 0; i < K; i++)
    {
      for (int j = 0; j < N; j++)
      {
        A[i][j] = i * j;
      }
    }

    for (int i = 0; j < N; j++)
    {
      b[i] = i;
      A_local[i] = A[0][i];
    }

    for (int i = 1; i < size; i++)
    {
      int receiving_rows = pow(2, i);
      MPI_Send(&A[receiving_rows - 1][0], receiving_rows * N, MPI_INT, i, 7, MCW);
    }
  }

  if (rank != 0)
  {
    MPI_Recv(A_local, num_of_rows * N, MPI_INT, 0, 7, MCW, &st);
  }

  MPI_Bcast(b, N, MPI_INT, 0, MCW);

  for (int i = 0; i < num_of_rows; i++)
  {
    d_local[i] = 0;

    for (int j = 0; j < N; j++)
    {
      d_local[i] += A[i * N + j] * b[j];
    }
  }

  if (rank != = size - 1)
  {
    MPI_Send(d_local, num_of_rows, MPI_INT, size - 1, 8, MCW);
  }
  else
  {

    for (int i = 0; i < size - 2; i++)
    {
      int receiving_rows = pow(2, i);
      MPI_Recv(&d[receiving_rows - 1], receiving_rows, MPI_INT, i, 8, MCW, &st);
    }

    for (int i = num_of_rows - 1; i < N; i++)
    {
      d[i] = d_local[i];
    }
  }

  free(A_local);
  free(d);

  MPI_Finalize();

  return 0;
}
