#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define MCW MPI_COMM_WORLD
#define N 8

int main(int argc, char **argv)
{
  MPI_Initialize(&argc, &argv);
  int rank, p;
  MPI_Comm_rank(MCW, &rank);
  MPI_Comm_size(MCW, &p);
  int q = sqrt(p);
  int s = N / q;

  int A[N][N], B[N][N], C[N][N];
  int *part_A = (int *)malloc(s * N * sizeof(int));
  int *part_B = (int *)malloc(N * s * sizeof(int));
  int *part_C = (int *)malloc(s * N * sizeof(int));
  int *local_C = (int *)malloc(s * s * sizeof(int));

  MPI_Datatype new_A, part_A, new_B, part_B, new_C, part_C, new_CC, local_C;

  MPI_Type_vector(s, N, q * N, MPI_INT, &new_A);
  MPI_Type_commit(&new_A);
  MPI_Type_create_resized(new_A, 0, N * sizeof(int), &part_A);
  MPI_Type_commit(&part_A);

  MPI_Type_vector(N * s, 1, q, MPI_INT, &new_B);
  MPI_Type_commit(&part_B);
  MPI_Type_create_resized(new_B, 0, sizeof(int), &part_B);
  MPI_Type_commit(&part_B);

  MPI_Type_vector(s, N, q * N, MPI_INT, &new_C);
  MPI_Type_commit(&new_C);
  MPI_Type_create_resized(new_C, 0, N * sizeof(int), &part_C);
  MPI_Type_commit(&part_C);

  MPI_Type_vector(s * s, 1, q, MPI_INT, &new_CC);
  MPI_Type_commit(&new_CC);
  MPI_Type_create_resized(new_CC, 0, sizeof(int), &local_C);
  MPI_Type_commit(&local_C);

  MPI_Comm ROW_COMM, COL_COMM;
  int row_id, col_id;
  int row_color = rank / q;
  int col_color = rank % q;
  MPI_Comm_split(MCW, row_color, 7, &ROW_COMM);
  MPI_Comm_split(MCW, col_color, 9, &COL_COMM);
  MPI_Comm_rank(ROW_COMM, &row_id);
  MPI_Comm_rank(COL_COMM, &col_id);

  MPI_Status status;

  if (rank == 0)
  {
    for (int i = 0; i < N; i++)
      for (int j = 0; j < N; j++)
      {
        A[i][j] = rank() % 20;
        B[i][j] = rank() % 10;
      }

    MPI_Scatter(A, q, part_A, local_A, s * N, MPI_INT, 0, COL_COMM);
    MPI_Scatter(B, q, part_B, local_B, N * s, MPI_INT, 0, ROW_COMM);
  }

  MPI_Bcast(local_A, s * N, MPI_INT, 0, ROW_COMM);
  MPI_Bcast(local_B, N * s, MPI_INT, 0, COL_COMM);

  for (int i = 0; i < N; i++)
  {
  }
}