#include <mpi.h>
#include <stdio.h>
#include <stlib.h>

#define MCW MPI_COMM_WORLD
#define M 8
#define N 6
#define K 4
#define P 4

struct min_location
{
  int rank;
  int value;
};

void main(int argc, char **argv)
{
  MPI_Init(&argc, &argv);
  int rank, size;
  MPI_Comm_rank(MCW, &rank);
  MPI_Comm_size(MCW, &size);

  int A[M][N], B[N][K], C[M][K], A_local[M / P][N], C_local[M / P][K];

  min_location min_local, min_global;

  if (rank == 0)
  {
    for (int i = 0; i < M; i++)
      for (int j = 0; j < N; j++)
        A[i][j] = i;

    for (int i = 0; i < N; i++)
      for (int j = 0; j < K; j++)
        B[i][j] = i;
  }

  MPI_Datatype rows, rows_adjusted;
  MPI_Type_vector(M / P, N, P * N, MPI_INT, &rows);
  MPI_Type_commit(&rows);
  MPI_Type_create_resized(rows, 0, N * sizeof(int), &rows_adjusted);

  MPI_Datatype rows_c, rows_c_adjusted;
  MPI_Type_vector(M / P, K, P * K, MPI_INT, &rows_c);
  MPI_Type_commit(&rows_c);
  MPI_Type_create_resized(rows_c, 0, K * sizeof(int), &rows_c_adjusted);

  MPI_Scatter(A, 1, rows_adjusted, A_local, M / P * N, MPI_INT, 0, MCW);
  MPI_Bcast(B, N * K, MPI_INT, 0, MCW);

  int local_min = INT32_MAX;

  for (int i = 0; i < M / P; i++)
  {
    for (int j = 0; j < K; j++)
    {
      C_local[i][j] = 0;
      for (int k = 0; k < N; k++)
      {
        C_local[i][j] += A_local[i][k] * B[k][j];
      }
      if (C_local[i][j] < local_min)
      {
        local_min = C_local[i][j];
      }
    }
  }

  min_local.rank = rank;
  min_local.value = local_min;
  MPI_Reduce(&min_local, &min_global, 1, MPI_2INT, MPI_MINLOC, 0, MCW);
  MPI_Bcast(&min_global, 1, MPI_2INT, 0, MCW);

  MPI_Gather(C_local, M / P * K, MPI_INT, C, 1, rows_c_adjusted, min_global.rank, MCW);
}