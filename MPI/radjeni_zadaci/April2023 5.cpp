#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#define MCW MPI_COMM_WORLD
#define n 8
#define p 16

void main(int aragc, char *argv[])
{
   int A[n][n], b[n], c[n], *local_a, *local_b, *part_c, local_c[n], q, s, i, j, rowcolor, colcolor, rank, row_id, col_id;

   MPI_Datatype newA, newb, newb_r;
   MPI_Status st;
   MPI_Comm rowcomm, colcomm;

   MPI_Init(&argc, &argv);
   MPI_Comm_rank(MCW, &rank);

   q = sqrt(p);
   s = n / q;

   local_a = (int *)malloc(s * s * sizeof(int));
   local_b = (int *)malloc(s * sizeof(int));
   part_c = (int *)malloc(s * sizeof(int));
   local_c = (int *)malloc(s * sizeof(int)); // I nacin:

   MPI_Type_vector(s * s, 1, q, MPI_INT, &newA);
   MPI_Type_commit(&newA);

   MPI_Type_vector(s, 1, q, MPI_INT, &newb);
   MPI_Type_commit(&newb);
   MPI_Type_create_resized(newb, 0, 1 * sizeof(int), &newb_r);
   MPI_Type_commit(&newb_r);

   rowcolor = rank / q;
   colcolor = rank % q;

   MPI_Comm_split(MCW, rowcolor, 7, &rowcomm);
   MPI_Comm_split(MCW, colcolor, 9, &colcomm);

   MPI_Comm_rank(rowcomm, &row_id);
   MPI_Comm_rank(colcomm, &col_id);

   if (rank == 0)
   {
      for (i = 0; i < n; i++)
      {
         b[i] = rand() % 20;
         for (j = 0; j < n; j++)
         {
            A[i][j] = rand() % 10;
         }
      }

      for (i = 0; i < s; i++)
      {
         for (j = 0; j < s; j++)
         {
            local_a[i * s + j] = A[i][j * q];
         }
      }

      for (i = 1; i < p; i++)
      {
         MPI_Send(&A[(i / q) * s][(i % q) * 1], 1, newA, i, 0, MCW);
      }
   }

   else
   {
      MPI_Recv(local_a, s * s, MPI_INT, 0, 0, MCW, &st);
   }

   if (col_id == 0) // Ulaze procesi P0,P1,P2,P3 oni imaju col_id 0
   {
      MPI_Scatter(b, 1, newb_r, local_b, s, MPI_INT, 0, rowcomm);
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

   II nacin : MPI_Gather(part_c, s, MPI_INT, local_c, s, MPI_INT, 0, colcomm);

   if (col_id == 0) // P0,P1,P2,P3
   {
      MPI_Reduce(local_c, c, n, MPI_INT, MPI_SUM, 0, rowcomm);
   }

   I nacin :

       MPI_Reduce(part_c, local_c, s, MPI_INT, MPI_SUM, 0, rowcomm);

   if (row_id == 0) // P0,P4,P8,P12
   {
      MPI_Gather(local_c, s, MPI_INT, c, s, MPI_INT, 0, colcomm);
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
   free(local_c); // I nacin:

   MPI_Finalize();
}
