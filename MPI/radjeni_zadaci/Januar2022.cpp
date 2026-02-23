#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <math.h>
#define MCW MPI_COMM_WORLD
#define n 6

void main(int argc, char *argv[])
{
   int A[n][n], B[n][n], C[n][n], *local_a, *local_b, *local_c, *part_c, q, rank, p, rowcolor, colcolor, row_id, col_id, i, j, z, s;

   MPI_Datatype newA, newA_r, newB, newB_r, newC, newC_r, newC1, newC1_r;
   MPI_Comm rowcomm, colcomm;
   MPI_Status st;

   MPI_Init(&argc, &argv);
   MPI_Comm_size(MCW, &p);
   MPI_Comm_rank(MCW, &rank);

   q = sqrt(p);
   s = n / q;

   local_a = (int *)malloc(s * n * sizeof(int));
   local_b = (int *)malloc(n * s * sizeof(int));
   local_c = (int *)malloc(s * n * sizeof(int));
   part_c = (int *)malloc(s * s * sizeof(int));

   MPI_Type_vector(s, n, q * n, MPI_INT, &newA);
   MPI_Type_commit(&newA);
   MPI_Type_create_resized(newA, 0, n * sizeof(int), &newA_r);
   MPI_Type_commit(&newA_r);

   MPI_Type_vector(n * s, 1, q, MPI_INT, &newB);
   MPI_Type_commit(&newB);
   MPI_Type_create_resized(newB, 0, 1 * sizeof(int), &newB_r);
   MPI_Type_commit(&newB_r);

   MPI_Type_vector(s * s, 1, q, MPI_INT, &newC);
   MPI_Type_commit(&newC);
   MPI_Type_create_resized(newC, 0, 1 * sizeof(int), &newC_r);
   MPI_Type_commit(&newC_r);

   MPI_Type_vector(s, n, q * n, MPI_INT, &newC1);
   MPI_Type_commit(&newC1);
   MPI_Type_create_resized(newC1, 0, n * sizeof(int), &newC1_r);
   MPI_Type_commit(&newC1_r);

   rowcolor = rank / q;
   colcolor = rank % q;

   MPI_Comm_split(MCW, rowcolor, 7, &rowcomm);
   MPI_Comm_split(MCW, colcolor, 7, &colcomm);

   if (rank == 0)
   {
      for (i = 0; i < n; i++)
      {
         for (j = 0; j < n; j++)
         {
            A[i][j] = rand() % 20;
            B[i][j] = rand() % 10;
         }
      }
b) Point-to-Point operacije   
    for(i=0;i<s;i++)
{
   for (j = 0; j < n; j++)
   {
      local_a[i * n + j] = A[i * q][j];
   }
}

for (i = 0; i < n; i++)
{
   for (j = 0; j < s; j++)
   {
      local_b[i * s + j] = B[i][j * q];
   }
}

for (i = 1; i < p; i++)
{
   MPI_Send(&A[i / q][0], 1, newA, i, 7, MCW);
   MPI_Send(&B[0][i % q], 1, newB, i, 8, MCW);
}
   }

   else
   {
      MPI_Recv(local_a, s * n, MPI_INT, 0, 7, MCW, &st);
      MPI_Recv(local_b, n * s, MPI_INT, 0, 8, MCW, &st);
   }

a) GRupne operacije i komunikatori 
 if(row_id == 0) // P0,P2
{
   MPI_Scatter(A, 1, newA_r, local_a, s * n, MPI_INT, 0, colcomm);
}

MPI_Bcast(local_a, s * n, MPI_INT, 0, rowcomm); // ovde su nulti procesi P0 i P2 i oni imaju podatke odredjenje matrice A i salju ostalim procesima na nivou vrste

if (col_id == 0) // P0 i P1
{
   MPI_Scatter(B, 1, newB_r, local_b, n * s, MPI_INT, 0, rowcomm);
}

MPI_Bcast(local_b, n * s, MPI_INT, 0, colcomm);
P0 i P1 imaju odredjenje podatke iz matrice B i salju ih procesima na nivou kolone svim ostalima

    for (i = 0; i < s; i++)
{
   for (j = 0; j < s; j++)
   {
      part_c[i * s + j] = 0;
      for (z = 0; z < n; z++)
      {
         part_c[i * s + j] += local_a[i * n + z] * local_b[z * s + j]; // C[i][j] += A[i][z] * B[z][j];
      }
   }
}

MPI_Gather(part_c, s * s, MPI_INT, local_c, 1, newC_r, 0, rowcomm);

if (row_id == 0)
{
   MPI_Gather(local_c, s * n, MPI_INT, C, 1, newC1_r, 0, colcomm);
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

free(local_a);
free(local_b);
free(local_c);
free(part_c);

MPI_Finalize();
}
