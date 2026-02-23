#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#define MCW MPI_COMM_WORLD
#define N 8

void main(int argc, char *argv[])
{
   float A[N], *local_a;
   int rank, p, i, s, C;

   MPI_Datatype new_type, new_type_r;

   MPI_Init(&argc, &argv);
   MPI_Comm_rank(MCW, &rank);
   MPI_Comm_size(MCW, &p);

   s = N / p;

   local_a = (float *)malloc(s * sizeof(float));

   MPI_Type_vector(s, 1, p, MPI_INT, &new_type);
   MPI_Type_commit(&new_type);
   MPI_Type_create_resized(new_type, 0, 1 * sizeof(int), &new_type_r);
   MPI_Type_commit(&new_type_r);

   if (rank == 0)
   {
      for (i = 0; i < N; i++)
      {
         A[i] = rand() % 20;
      }

      C = rand() % 10;
   }

   MPI_Scatter(A, 1, new_type_r, local_a, s, MPI_FLOAT, 0, MCW);

   float local_sum = 0, global_sum;

   for (i = 0; i < s; i++)
   {
      local_sum += local_a[i];
   }

   MPI_Reduce(&local_sum, &global_sum, 1, MPI_FLOAT, MPI_SUM, 0, MCW);
   MPI_Bcast(&global_sum, 1, MPI_FLOAT, 0, MCW);

   float As = global_sum / N;

   MPI_Bcast(&C, 1, MPI_INT, 0, MCW);

   struct
   {

      float value;
      int rank;
   } local_max, global_max;

   local_max.value = FLT_MIN;
   local_max.rank = rank;

   for (i = 0; i < s; i++)
   {
      if (local_a[i] > local_max.value)
      {
         local_max.value = local_a[i];
      }
   }

   MPI_Reduce(&local_max, &global_max, 1, MPI_FLOAT_INT, MPI_MAXLOC, 0, MCW);
   MPI_Bcast(&global_max, 1, MPI_FLOAT_INT, 0, MCW);

   float local_M, global_M;

   local_M = 0;

   for (i = 0; i < s; i++)
   {
      local_M += (local_a[i] + As) / C;
   }

   MPI_Reduce(&local_M, &global_M, 1, MPI_FLOAT, MPI_SUM, global_max.rank, MCW);

   if (rank == global_max.rank)
   {
      printf("%d", global_M);
   }

   free(local_a);

   MPI_Finalize();
}
