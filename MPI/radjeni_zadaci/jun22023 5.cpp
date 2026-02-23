#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#define MCW MPI_COMM_WORLD
#define k 6
#define n 4

void main(int argc, char *argv[])
{
   int A[k][n], b[n], c[k], *local_a, *local_c, s, p, rank, i, j;
   MPI_Datatype newA, newA_r, newC, newC_r;

   MPI_Init(&argc, &argv);
   MPI_Comm_size(MCW, &p);
   MPI_Comm_rank(MCW, &rank);

   s = k / p;

   local_a = (int *)malloc(s * n * sizeof(int));
   local_c = (int *)malloc(s * sizeof(int));

   MPI_Type_vector(s, n, p * n, MPI_INT, &newA);
   MPI_Type_commit(&newA);
   MPI_Type_create_resized(newA, 0, n * sizeof(int), &newA_r);
   MPI_Type_commit(&newA_r);

   MPI_Type_vector(s, 1, p, MPI_INT, &newC);
   MPI_Type_commit(&newC);
   MPI_Type_create_resized(newC, 0, 1 * sizeof(int), &newC_r);
   MPI_Type_commit(&newC_r);

   struct
   {

      float value;
      int rank;
   } local_max, global_max;

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
   }

   MPI_Scatter(A, 1, newA_r, local_a, s * n, MPI_INT, 0, MCW);
   MPI_Bcast(b, n, MPI_INT, 0, MCW);

   local_max.value = INT_MIN;
   local_max.rank = rank;

   for (i = 0; i < s; i++)
   {
      for (j = 0; j < n; j++)
      {
         if (local_max.value < local_a[i * n + j])
         {
            local_max.value = local_a[i * n + j];
         }
      }
   }

   MPI_Reduce(&local_max, &global_max, 1, MPI_2INT, MPI_MAXLOC, 0, MCW);
   MPI_Bcast(&global_max, 1, MPI_2INT, 0, MCW);

   for (i = 0; i < s; i++)
   {
      local_c[i] = 0;
      for (j = 0; j < n; j++)
      {
         local_c[i] += local_a[i * n + j] * b[j];
      }
   }

   MPI_Gather(local_c, s, MPI_INT, c, 1, newC_r, global_max.rank, MCW);

   if (rank == global_max.rank)
   {
      for (i = 0; i < k; i++)
      {
         printf("%d", c[i]);
      }
   }

   free(local_a);
   free(local_c);

   MPI_Finalize();
}
