#include <stdio.h>

#include <stdlib.h>

#include <mpi.h>

#define n 8
#define p 16
#define MCW MPI_COMM_WORLD #include<math.h>

void main(int argc, char *argv[])
{

  int A[n][n], B[n][n], C[n][n], *local_1a, *local_1b, *local_2a,
      *local_2b, *local_c, *part_c, k, q, rowcolor, colcolor, row_id,
      col_id, i, j, z, rank;

  MPI_Status st;

  MPI_Datatype new_type, new_type_r, newB, newB_r;
  MPI_Comm rowcomm, colcomm;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MCW, &rank);

  q = sqrt(p);
  k = n / q;

  local_1a = (int *)malloc(k * k * sizeof(int));
  local_1b = (int *)malloc(k * k * sizeof(int));
  local_2a = (int *)malloc(k * n * sizeof(int));
  local_2b = (int *)malloc(n * k * sizeof(int));
  local_c = (int *)malloc(k * k * sizeof(int));
  part_c = (int *)malloc(k * n * sizeof(int));

  rowcolor = rank / q;
  colcolor = rank % q;

  MPI_Comm_split(MCW, rowcolor, 7, &rowcomm);
  MPI_Comm_split(MCW, colcolor, 7, &colcomm);

  MPI_Comm_rank(rowcomm, &row_id);
  MPI_Comm_rank(colcomm, &col_id);

  MPI_Type_vector(k, k, n, n, MPI_INT, &new_type);
  MPI_Type_commit(&new_type);
  MPI_Type_create_resized(new_type, 0, k * sizeof(int), &new_type_r);
  MPI_Type_commit(&new_type_r);

  MPI_Type_vector(n, k, n, MPI_INT, &newB);
  MPI_Type_commit(&newB);
  MPI_Type_create_resized(newB, 0, k * sizeof(int), &newB_r);
  MPI_Type_commit(&newB_r);

  if (rank == 0)
  {
    for (i = 0; i < n; i++)
    {
      for (j = 0; j < n; j++)
      {
        A[i][j] = rand() % 100;
        B[i][j] = rand() % 100;
      }
    }

    for (i = 0; i < k; i++)
    {
      for (j = 0; j < k; j++)
      {
        local_1a = A[i][j];
        local_1b = B[i][j];
      }
    }

    for (i = 1; i < p; i++)
    {
      MPI_Send(&A[(i / q) * k][(i % q) * k], 1, new_type, i, 7, MCW);
      MPI_Send(&B[(i / q) * k][(i % q) * k], 1, new_type, i, 9, MCW);
    }
  }
  else
  {
    MPI_Recv(local_1a, k * k, MPI_INT, 0, 7, MCW, &st);
    MPI_Recv(local_1b, k * k, MPI_INT, 0, 9, MCW, &st);
  }

  if (row_id == 0)
  {
    MPI_Gather(local_1a, k * k, MPI_INT, local_2a, k * n, MPI_INT, 0, rowcomm);
  }

  MPI_Bcast(local_2a, k * n, MPI_INT, 0, rowcomm);

  if (col_id == 0)
  {
    MPI_Gather(local_1b, k * k, MPI_INT, local_2b, 1, newB_r, 0, colcomm);
  }

  MPI_Bcast(local_2b, n * k, MPI_INT, 0, colcomm);

  for (i = 0; i < k; i++)
  {
    for (j = 0; j < k; j++)
    {
      local_c[i * k + j] = 0;

      for (z = 0; z < n; z++)
      {
        local_c[i * k + j] += local_2a[i * n + z] * local_2b[z * k + j];
      }
    }
  }

  MPI_Gather(local_c, k * k, MPI_INT, part_c, 1, new_type_r, 0, rowcomm);

  if (row_id == 0)
  {
    MPI_Gather(part_c, k * n, MPI_INT, C, n * n, MPI_INT, 0, colcomm);
  }

  if (rank == 0)
  {
    for (i = 0; i < n; i++)
    {
      for (j = 0; j < n; j++)
      {
        printf("%d", C[i][j]);
      }
      printf("\n");
    }
  }

  free(local_1a);
  free(local_1b);
  free(local_2a);
  free(local_2b);
  free(local_c);
  free(part_c);

  MPI_Finalize();
}