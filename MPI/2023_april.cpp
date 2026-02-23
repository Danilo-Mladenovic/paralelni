#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>

#define MCW MPI_COMM_WORLD
#define N 8

int main(int argc, char **argv)
{
  MPI_Initialize(&argc, &argv);
  int rank, size;
  MPI_Comm_rank(MCW, &rank);
  MPI_Comm_size(MCW, &size);

  int q = sqrt(size);
  int s = N / q;
  int A[N][N], b[N], c[N], local_min, global_min;

  int *part_A = (int *)malloc(s * s * sizeof(int));
  int *part_b = (int *)malloc(s * sizeof(int));
  int *part_c = (int *)malloc(s * sizeof(int));
  int *local_c = (int *)malloc(s * sizeof(int));

  MPI_Datatype new_A, resized_A, new_B, resized_B;
  MPI_Status st;

  MPI_Type_vector(s * s, 1, q, MPI_INT, &new_A);
  MPI_Type_commit(&new_A);
  MPI_Type_create_resized(&new_A, 0, 1 * sizeof(int), &resized_A);
  MPI_Type_commit(&resized_A);

  MPI_Type_vector(s, 1, q, MPI_INT, &new_B);
  MPI_Type_commit(&new_B);
  MPI_Type_create_resized(&new_B, 0, 1 * sizeof(int), &resized_B);
  MPI_Type_commit(&resized_B);

  MPI_Status = st;
  MPI_Comm ROW_COMM, COL_COMM;
  int row_color, col_color, row_id, col_id;
  row_color = rank / q;
  col_color = rank % q;
  MPI_Comm_split(MCW, row_color, 7, &ROW_COMM);
  MPI_Comm_split(MCW, col_color, 9, &COL_COMM);
  MPI_Comm_rank(ROW_COMM, &row_id);
  MPI_Comm_rank(COL_COMM, &col_id);

  if (rank == 0)
  {
    for (int i = 0; i < N; i++)
    {
      b[i] = i;
      for (int j = 0; j < N; j++)
        A[i][j] = i * j;
    }

    for (int i = 0; i < s; i++)
      for (int j = 0; j < s; j++)
        part_a[i * s + j] = A[i][j * q];

    for (int i = 1; i < size; i++)
      MPI_Send(&A[i / q * s][i % q], 1, resized_A, i, 0, MCW);
  }
  else
  {
    MPI_Recv(part_A, s * s, MPI_INT, 0, 0, MCW, &st);
  }
}