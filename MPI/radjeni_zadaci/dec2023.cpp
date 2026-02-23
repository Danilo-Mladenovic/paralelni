#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#define MCW MPI_COMM_WORLD
#define k 4
#define m 6

void main(int argc, char *argv[])
{
   int A[k][m], b[m], c[k], *local_a, *local_b, local_c[k], rank, p, s, i, j;

   MPI_Init(&argc, &argv);
   MPI_Comm_size(MCW, &p);
   MPI_Comm_rank(MCW, &rank);

   MPI_Datatype newA, newA_r, newB, newB_r;

   s = m / p;

   local_a = (int *)malloc(k * s * sizeof(int));
   local_b = (int *)malloc(s * sizeof(int));

   if (rank == 0)
   {
      for (i = 0; i < k; i++)
      {
         for (j = 0; j < m; j++)
         {
            A[i][j] = rand() % 100;
         }
      }

      for (i = 0; i < m; i++)
      {
         b[i] = rand() % 10;
      }
   }

   MPI_Type_vector(k * s, 1, p, MPI_INT, &newA);
   MPI_Type_commit(&newA);
   MPI_Type_create_resized(newA, 0, 1 * sizeof(int), &newA_r);
   MPI_Type_commit(&newA_r);

   MPI_Type_vector(s, 1, p, MPI_INT, &newB);
   MPI_Type_commit(&newB);
   MPI_Type_create_resized(newB, 0, 1 * sizeof(int), &newB_r);
   MPI_Type_commit(&newB_r);

   MPI_Scatter(A, 1, newA_r, local_a, k * s, MPI_INT, 0, MCW);
   MPI_Scatter(b, 1, newB_r, local_b, s, MPI_INT, 0, MCW);

   for (i = 0; i < k; i++)
   {
      local_c[i] = 0;
   }

   for (i = 0; i < k; i++)
   {
      for (j = 0; j < s; j++)
      {
         local_c[i] += local_a[i * s + j] * local_b[j];
      }
   }

   MPI_Reduce(local_c, c, k, MPI_INT, MPI_SUM, 0, MCW);

   if (rank == 0)
   {
      for (i = 0; i < k; i++)
      {
         printf("%d", c[i]);
      }
   }

   free(local_a);
   free(local_b);

   MPI_Finalize();
}
