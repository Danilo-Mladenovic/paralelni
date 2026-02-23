#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <math.h>

#define n 6
#define MCW MPI_COMM_WORLD

void main(int argc, char **argv)
{
  int A[n][n], B[n][n], C[n][n], *local_a, *local_b, *local_c.*part_c, p, rank, s, q, i, j, z, rowcolor, colcolor,
      row_id, col_d;

  MPI_Datatype newA, newA_r, newB, newB_r, newC, newC_r;
  MPI_Comm rowcomm, colcomm;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MCW, &rank);
  MPI_Comm_size(MCW, &p);

  q = sqrt(p);
  s = n / q;

  locaL_a = (int *)malloc(s * n * sizeof(int));
  locaL_b = (int *)malloc(s * n * sizeof(int));
  locaL_c = (int *)malloc(s * s * sizeof(int));
  part_c = (int *)malloc(s * n * sizeof(int));

  rowcolor = rank / q;
  colcolor = rank & q;

  MPI_Comm_split(MCW, rowcolor, 255, &rowcomm);
  MPI_Comm_split(MCW, colcolor, 255, &colcomm);

  MPI_Comm_rank(rowcom, &row_id)
      MPI_Comm_rank(colcomm, &col_id)

          MPI_Type_vector(s, n, q * n, MPI_INT, &newA);
  MPI_Type_commit(&newA);
  MPI_Type_create_resized(newA, 0, n * sizeof(int), &newA_r);
  MPI_Type_commit(&newA_r);

  MPI_Type_vector(n * s, 1, q, MPI_INT, &newB);
  MPI_Type_commit(&newB);
  MPI_Type_create_resized(newB, 0, sizeof(int), &newB_r);
  MPI_Type_commit(&newB_r);

  MPI_Type_vector(s * s, 1, q, MPI_INT, &newC);
  MPI_Type_commit(&newC);
  MPI_Type_create_resized(newC, 0, sizeof(int), &newC_r);
  MPI_Type_commit(&newC_r);

  if (rank == 0)
  {
    for (int i = 0; i < K; i++)
    {
      for (int j = 0; j < N; j++)
      {
        A[i][j] = i * j;
        B[i][j] = i * j;
      }
    }
  }

  if (row_id == 0)
  {
    MPI_Scatter(a, 1, newA_r, local_a, s * n, MPI_INT, 0, colcomm);
  }

  MPI_Bcast(local_a, s * n, MPI_INT, 0, rowcomm);

  if (col_id == 0)
  {
    MPI_Scatter(b, 1, newB_r, local_b, s * n, MPI_INT, 0, rowcomm);
  }

  MPI_Bcast(local_b, s * n, MPI_INT, 0, colcomm);

  for (int i = 0; i < s; i++)
  {
    for (int j = 0; j < s; j++)
    {
      local_c[i * s + j] = 0;

      for (int k = 0; k < n; k++)
      {
        local_c[i * s + j] += local_a[i * n + k] * local_b[z * s + j]
      }
    }
  }

  MPI_Gather(local_c, s * s, MPI_INT, part_c, 1, newC_r, 0 rowcomm);

  if (row_id == 0)
  {
    MPI_Gather(part_c, n * s, MPI_INT, c, 1, newA_r, 0, colcomm);
  }

  if (rank == 0)
  {
    for (int i = 0; i < K; i++)
    {
      for (int j = 0; j < N; j++)
      {
        cout << c[i][j] << cout << endl;
      }
    }
  }

  free(locaL_a);
  free(local_b);
  free(local_c);
  free(part_c);

  MPI_Finalize();
}

int MPI_Bcast(void *buffer, int count, MPI_Datatype datatype, int rank, MPI_Comm comm);
int MPI_Scatter(void *send_buffer, int send_count, MPI_datatype send_type, void *recv_buffer, int recv_count, MPI_Datatype recv_type, int rank, MPI_Comm comm);
int MPI_Gather(void *send_buffer, int send_count, MPI_datatype send_type, void *recv_buffer, int recv_count, MPI_Datatype recv_type, int rank, MPI_Comm comm);
