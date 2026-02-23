#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#define MCW MPI_COMM_WORLD
#define m 6
#define k 4
#define l 4

void main(int argc, char *argv[])
{
   int A[k][m], B[m][l], C[k][l], local_c[k][l], *local_a, *local_b, s, rank, size, i, j, z;

   MPI_Datatype newA, newA_r, newB, newB_r;
   MPI_Status st;

   MPI_Init(&argc, &argv);
   MPI_Comm_size(MCW, &size);
   MPI_Comm_rank(MCW, &rank);

   s = m / size;

   local_a = (int *)malloc(k * s * sizeof(int));
   local_b = (int *)malloc(s * l * sizeof(int));

   MPI_Type_vector(k * s, 1, p, MPI_INT, &newA);
   MPI_Type_commit(&newA);
   MPI_Type_create_resized(newA, 0, 1 * sizeof(int), &newA_r);
   MPI_Type_commit(&newA_r);

   MPI_Type_vector(s, l, l * p, MPI_INT, &newB);
   MPI_Type_commit(&newB);
   MPI_Type_create_resized(newB, 0, l * sizeof(int), &newB_r);
   MPI_Type_commit(&newB_r);

   if (rank == 0)
   {
      for (i = 0; i < k; i++)
      {
         for (j = 0; j < m; j++)
         {
            A[i][j] = rand() % 10;
         }
      }

      for (i = 0; i < m; i++)
      {
         for (j = 0; j < l; j++)
         {
            B[i][j] = rand() % 100;
         }
      }

      for (i = 0; i < k; i++)
      {
         for (j = 0; j < s; j++)
         {
            local_a[i * s + j] = A[i][j * size];
         }
      }

      for (i = 0; i < s; i++)
      {
         for (j = 0; j < l; j++)
         {
            local_b[i * l + j] = B[i * size][j];
         }
      }

      for (i = 1; i < size; i++)
      {
         MPI_Send(&A[0][i], 1, newA, i, 7, MCW);
         MPI_Send(&B[i][0], 1, newB, i, 8, MCW);
      }
   }
   else
   {
      MPI_Recv(local_a, k * s, MPI_INT, 0, 7, MCW, &st);
      MPI_Recv(local_b, s * l, MPI_INT, 0, 8, MCW, &st);
   }

 a) MPI_Scatter(A,1,newA_r,local_a,k*s,MPI_INT,0,MCW);
 MPI_Scatter(B, 1, newB_r, local_b, s * l, MPI_INT, 0, MCW);

 for (i = 0; i < k; i++)
 {
    for (j = 0; j < l; j++)
    {
       localc[i][j] = 0;
       for (z = 0; z < s; z++)
          min
          {
             local_c[i][j] += local_a[i * s + z] * local_b[z * l + j];
          }
    }
 }

 struct
 {

    int value;
    int rank;
 } global_min, local_min;

 local_min.value = INT_MAX;
 local_min.rank = rank;

 for (i = 0; i < k; i++)
 {
    for (j = 0; j < s; j++)
    {
       if (local_a[i * s + j] < local_min.value)
       {
          local_min.value = local_a[i * s + j];
       }
    }
 }

 MPI_Reduce(&local_min, &global_min, 1, MPI_2INT, MPI_MINLOC, 0, MCW);
 MPI_Bcast(&global_min, 1, MPI_2INT, 0, MCW);

 MPI_Reduce(local_c, C, k * l, MPI_INT, MPI_SUM, global_min.rank, MCW);

 if (rank == global_min.rank)
 {
    for (i = 0; i < k; i++)
    {
       for (j = 0; j < l; j++)
       {
          printf("%d", C[i][j]);
       }
       printf("\n");
    }
 }

 free(local_a);
 free(local_b);

 MPI_Finalize();
}
