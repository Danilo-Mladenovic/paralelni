#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#define MCW MPI_COMM_WORLD
#define N 6

void main(int argc, char *argv[])
{
  int A[N][N], rank, p, i, j;
  MPI_Datatype newtype;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MCW, &rank);
  MPI_Comm_size(MCW, &p);

  if (rank == 0)
  {
    for (i = 0; i < N; i++)
    {
      for (j = 0; j < N; j++)
      {
        A[i][j] = rand() % 10;
      }
    }
  }

  int count = 10;

  int blocklenght[] = {2, 1, 2, 1, 2, 1, 2, 1, 2, 1};
  int displacement[] = {1, 4, 8, 11, 15, 18, 22, 25, 29, 32};
  MPI_Type_indexed(count, blocklenghr, displacement, MPI_INT, &new_type);
  MPI_Type_commit(&new_type);

  MPI_Group svi, nova_grupa;
  MPI_Comm novi_comm;

  MPI_Comm_group(MCW, &svi);
  int members[] = {0, 1, 3, 5};
  MPI_Group_incl(svi, 4, members, &nova_grupa);
  MPI_Comm_create(MCW, nova_grupa, &novi_comm);

  MPI_Bcast(A, 1, new_type, 0, novi_comm);

  int novi_rank;
  MPI_Comm_rank(novi_comm, &novi_rank);

  I nacin :

      if (rank == 1 || rank == 3 || rank == 5)
  {
    for (i = 0; i < n; i++)
    {
      for (j = 0; j < n; j++)
      {
        printf("%d", A[i][j]);
      }
    }
  }

  II nacin : if (novi_rank == 1 || novi_rank == 2 || novi_rank == 3)
  {
    {
      for (i = 0; i < n; i++)
      {
        for (j = 0; j < n; j++)
        {
          printf("%d", A[i][j]);
        }
      }
    }

    MPI_Finalize();
  }
