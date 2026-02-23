#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#define MCW MPI_COMM_WORLD
#define N 4
#define L 20
#define p 16

void main(int argc, char *argv[])
{
   int A[N][N], rank, i, j;

   MPI_Init(&argc, &argv);
   MPI_Comm_rank(MCW, &rank);

   for (i = 0; i < N; i++)
   {
      for (j = 0; j < N; j++)
      {
         A[i][j] = i * N + j; // da bi nam se po redosledu od 0 do 15 upisivali rankovi procesa u mat procesa
      }
   }

   int members[2 * N + N - 2];

   int count = 0;
   for (i = 0; i < N; i++)
   {
      members[count++] = A[0][i];
   }

   for (i = 1; i < N - 1; i++)
   {
      members[count++] = A[i][N - 1 - i];
   }

   for (i = 0; i < N; i++)
   {
      members[count++] = A[N - 1][i];
   }

   MPI_Group svi, nova_grupa;
   MPI_Comm novi_comm;

   MPI_Comm_group(MCW, &svi);
   MPI_Group_incl(svi, count, members, &nova_grupa);
   MPI_Comm_create(MCW, nova_grupa, &novi_comm);

   int niz[L], *local_niz;
   int m = L / count;

   local_niz = (int *)malloc(m * sizeof(int));

   if (rank == 0)
   {
      for (i = 0; i < L; i++)
      {
         niz[i] = rand() % 20;
      }
   }

   MPI_Scatter(niz, m, MPI_INT, local_niz, m, MPI_INT, 0, novi_comm);

   int local_sum = 0, global_sum;

   for (i = 0; i < m; i++)
   {
      local_sum += local_niz[i];
   }

   MPI_Reduce(&local_sum, &global_sum, 1, MPI_INT, MPI_SUM, 0, novi_comm);

   float art_sredina = global_sum / L;

   if (rank == 0)
   {
      printf("%f", art_sredina);
   }

   free(local_niz);

   MPI_Finalize();
}