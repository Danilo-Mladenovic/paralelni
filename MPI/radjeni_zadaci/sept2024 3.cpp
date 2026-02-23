#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#define MCW MPI_COMM_WORLD
#define r 6
#define m 4
#define k 6

void main(int argc, char *argv[])
{
   int A[r][m], B[m][k], C[r][k], *local_a, *local_c, s, p rank, i, j, z;
   MPI_Datatype newA, newA_r, newC, newC_r;

   MPI_Init(&argc, &argv);
   MPI_Comm_size(MCW, &p);
   MPI_Comm_rank(MCW, &rank);

   s = r / p;

   local_a = (int *)malloc(s * m * sizeof(int));
   local_c = (int *)malloc(s * k * sizeof(int));

   MPI_Type_vector(s, m, p * m, MPI_INT, &newA);
   MPI_Type_commit(&newA);
   MPI_Type - create_resized(newA, 0, m * sizeof(int), &newA_r);
   MPI_Type_commit(&newA_r);

   MPI_Type_vector(s, k, p * k, MPI_INT, &newC);
   MPI_Type_commit(&newC);
   MPI_Type - create_resized(newC, 0, k * sizeof(int), &newC_r);
   MPI_Type_commit(&newC_r);

   if (rank == 0)
   {
      for (i = 0; i < r; i++)
      {
         for (j = 0; j < m; j++)
         {
            A[i][j] = rand() % 100;
         }
      }

      for (i = 0; i < m; i++)
      {
         for (j = 0; j < k; j++)
         {
            B[i][j] = rand() % 20;
         }
      }
   }

   MPI_Scatter(A, 1, newA_r, local_a, s * m, MPI_INT, 0, MCW);
   MPI_Bcast(B, m * k, MPI_INT, 0, MCW);

   for (i = 0; i < s; i++)
   {
      for (j = 0; j < k; j++)
      {
         local_c[i * k + j] = 0;
         for (z = 0; z < m; z++)
         {
            local_c[i * k + j] += local_a[i * m + z] * B[z][j];
         }
      }
   }

   MPI_Gather(local_c, s * k, MPI_INT, C, 1, newC_r, 0, MCW);

   int local_prod[] = {1, 1, 1, 1, 1, 1};
   int global_prod;

   for (i = 0; i < k; i++)
   {
      for (j = 0; j < s; j++)
      {
         local_prod[i] *= local_c[j * k + i];
      }
   }

   MPI_Reduce(local_prod, global_prod, k, MPI_INT, MPI_PROD, 0, MCW);

   if (rank == 0)
   {
      for (i = 0; i < r; i++)
      {
         for (j = 0; j < k; j++)
         {
            printf("%d", C[i][j]);
         }
         printf("\n");
      }
   }

   free(local_a);
   free(local_c);

   MPI_Finalize();
}
