#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <math.h>

#define n 6
#define p 16
#define MCW MPI_COMM_WORLD

void main(int argc, char **argv)
{
  int A[n][n], B[n][n], C[n][n], *local_1a, *local_2a, *local_1b,
      *local_2b, *local_c.*part_c, p, rank, s, q, i, j, z, k, q,
      rank, rowcolor, colcolor, row_id, col_d;

  MPI_Datatype new_type, new_type_r, newB, newB_r;
  MPI_Comm rowcomm, colcomm;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MCW, &rank);

  q = sqrt(p);
  k = n / q;

  locaL_1a = (int *)malloc(k * k * sizeof(int));
  locaL_2a = (int *)malloc(k * n * sizeof(int));
  locaL_1a = (int *)malloc(k * k * sizeof(int));
  locaL_2a = (int *)malloc(k * n * sizeof(int));
  locaL_c = (int *)malloc(k * k * sizeof(int));
  part_c = (int *)malloc(k * n * sizeof(int));

  rowcolor = rank / q;
  colcolor = rank & q;

  MPI_Comm_split(MCW, rowcolor, 255, &rowcomm);
  MPI_Comm_split(MCW, colcolor, 255, &colcomm);

  MPI_Comm_rank(rowcom, &row_id)
      MPI_Comm_rank(colcomm, &col_id)

          MPI_Type_vector(k, k, n, MPI_INT, &new_type);
  MPI_Type_commit(&new_type);
  MPI_Type_create_resized(new_type, 0, k * sizeof(int), &new_type_r);
  MPI_Type_commit(&new_type_r);

  MPI_Type_vector(n, k, n, MPI_INT, &new_B);
  MPI_Type_commit(&new_B);
  MPI_Type_create_resized(new_B, 0, k * sizeof(int), &new_B_r);
  MPI_Type_commit(&new_B_r);
}

int MPI_Bcast(void *buffer, int count, MPI_Datatype datatype, int rank, MPI_Comm comm);
int MPI_Scatter(void *send_buffer, int send_count, MPI_datatype send_type, void *recv_buffer, int recv_count, MPI_Datatype recv_type, int rank, MPI_Comm comm);
int MPI_Gather(void *send_buffer, int send_count, MPI_datatype send_type, void *recv_buffer, int recv_count, MPI_Datatype recv_type, int rank, MPI_Comm comm);
