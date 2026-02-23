#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <math.h>
#define MCW MPI_COMM_WORLD
#define k 7
#define n 4

void main(int argc, char *argv[])
{
   int A[k][n], b[n], c[n], size, rank, *local_a, *local_b, i, j;
   MPI_Status st;

   MPI_Init(&argc, &argv);
   MPI_Comm_size(MCW, &size);
   MPI_Comm_rank(MCW, &rank);

   int local_rows = pow(2, rank);

   local_a = (int *)malloc(local_rows * n * sizeof(int));
   local_c = (int *)malloc(local_rows * sizeof(int));

   if (rank == 0)
   {
      for (i = 0; i < k; i++)
      {
         for (j = 0; j < n; j++)
         {
            A[i][j] = rand() % 20;
         }
      }

      for (i = 0; i < n; i++)
      {
         b[i] = rand() % 10;
      }

      for (i = 0; i < local_rows; i++)
      {
         for (j = 0; j < n; j++)
         {
            local_a[i * n + j] = A[i][j];
         }
      }

      for (i = 1; i < size; i++)
      {
         int s = pow(2, i);
         MPI_Send(&A[s - 1][0], s * n, MPI_INT, i, 7, MCW);
      }
   }

   else // P1,P2
   {
      MPI_Recv(local_a, local_rows * n, MPI_INT, 0, 7, MCW, &st);
   }

   MPI_Bcast(b, n, MPI_INT, 0, MCW);

   for (i = 0; i < local_rows; i++)
   {
      local_c[i] = 0;
      for (j = 0; j < n; j++)
      {
         local_c[i] += local_a[i * n + j] * b[j];
      }
   }

   if (rank != size - 1)
      P0 i P1 salju
      {
         MPI_Send(local_c, local_rows, MPI_INT, size - 1, 8, MCW);
      }

   else
   { // P2  local_rows znam za proces P2 samo!!!

      for (i = 0; i < size - 1; i++)
      {
         int offset = pow(2, i);
         MPI_Recv(&c[offset - 1], offset, MPI_INT, i, 8, MCW, &st);
      }

      for (i = local_rows - 1; i < n; i++)
      {
         c[i] = local_c[i - size];
      }

      for (i = 0; i < k; i++)
      {
         printf("%d", c[i]);
      }
   }

   free(local_a);
   free(local_c);

   MPI_Finalize();
}
