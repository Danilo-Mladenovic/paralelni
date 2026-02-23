#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <math.h>
#define MCW MPI_COMM_WORLD
#define p 16
#define n 8

void main(int argc, char *argv[])
{
   int A[n][n], b[n], c[n], *local_a, *local_b, *part_c, local_c[n], q, s, rowcolor, colcolor, row_id, col_id, i, j, rank;

   MPI_Datatype newA, newA_r, newB, newB_r, newC, newC_r;
   MPI_Comm rowcomm, colcomm;

   MPI_Init(&argc, &argv);
   MPI_Comm_rank(MCW, &rank);

   q = sqrt(p);
   s = n / q;

   local_a = (int *)malloc(s * s * sizeof(int));
   local_b = (int *)malloc(s * sizeof(int));
   part_c = (int *)malloc(s * sizeof(int));

   int count = s * s;
   int blocklenghts[] = {1, 1, 1, 1};
   int displacements[] = {0, q, q * n, q * n + q};
   MPI_Type_indexed(count, blocklenghts, displacements, MPI_INT, &newA);
   MPI_Type_commit(&newA);

   // kad posaljemo celu matricu A procesima P0,P4,P8 i P12 tada se gleda resized izvedeni tip podataka

   MPI_Type_create_resized(newA, 0, 1 * sizeof(int), &newA_r);
   MPI_Type_commit(&newA_r);

   MPI_Type_vector(s, 1, q, MPI_INT, &newB);
   MPI_Type_commit(&newB);
   MPI_Type_create_resized(newB, 0, 1 * sizeof(int), &newB_r);
   MPI_Type_commit(&newB_r);

   MPI_Type_vector(s, 1, q, MPI_INT, &newC);
   MPI_Type_commit(&newC);
   MPI_Type_create_resized(newC, 0, 1 * sizeof(int), &newC_r);
   MPI_Type_commit(&newC_r);

   rowcolor = rank / q;
   colcolor = rank % q;

   MPI_Comm_split(MCW, rowcolor, 7, &rowcomm);
   MPI_Comm_split(MCW, colcolor, 7, &colcomm);

   MPI_Comm_rank(rowcomm, &row_id);
   MPI_Comm_rank(colcomm, &col_id);

   if (rank == 0)
   {
      for (i = 0; i < n; i++)
      {
         b[i] = rand() % 10;
         for (j = 0; j < n; j++)
         {
            A[i][j] = rand() % 20;
         }
      }
   }

   if (row_id == 0) // P0,P4,P8,P12 njima saljemo celu matricu A
   {
      MPI_Bcast(A, n * n, MPI_INT, 0, colcomm);
   }

   MPI_Scatter(&A[rank / q][0], 1, newA_r, local_a, s * s, MPI_INT, 0, rowcomm);

   if (col_id == 0) // P0,P1,P2,P3
   {
      MPI_Scatter(b, 1, newB_r, local_b, s, MPI_INT, 0, rowcomm);
   }

   MPI_Bcast(local_b, s, MPI_INT, 0, colcomm);

   for (i = 0; i < s; i++)
   {
      part_c[i] = 0;
      for (j = 0; j < s; j++)
      {
         part_c[i] += local_a[i * s + j] * local_b[j];
      }
   }

   MPI_Gather(part_c, s, MPI_INT, local_c, 1, newC_r, 0, colcomm);

   if (col_id == 0) // P0 P1 P2 P3  P0 je nula u rowcomm u odnosu na sve njih
   {
      MPI_Reduce(local_c, c, n, MPI_INT, MPI_SUM, 0, rowcomm);
   }

   if (rank == 0)
   {
      for (i = 0; i < n; i++)
      {
         printf("%d", c[i]);
      }
   }

   free(local_a);
   free(local_b);
   free(part_c);

   MPI_Finalize();
}