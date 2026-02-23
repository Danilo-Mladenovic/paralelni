April 2021. slican zadatak sa malim izmenama

#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#define MCW MPI_COMM_WORLD
#define p 12
#define up 0
#define down 1

    void main(int argc, char *argv[])
{
   int rank;

   MPI_Init(&argc, &argv);
   MPI_Comm_rank(MCW, &rank);

   MPI_Status st;

   MPI_Comm cartcomm;

   int dim_size[] = {3, 4};
   int periods[] = {0, 1};

   MPI_Cart_create(MCW, 2, dim_size, periods, 0, &cartcomm);

   int novi_rank;
   MPI_Comm_rank(cartcomm, &novi_rank);

   int coords[2];
   MPI_Cart_coords(cartcomm, novi_rank, 2, coords);

   int dest, source;
   in niz[2];

   MPI_Cart_shift(cartcomm, 1, coords[1], &niz[up], &niz[down]);

   source = niz[up];
   dest = niz[down];

   int a = rank;
   MPI_Sendrecv_replace(&a, 1, MPI_INT, dest, 0, source, cartcomm, &st);

   MPI_Finalize();
}
//////////////////////////////////////////////////////////

Kolokvijum I 2023

#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#define MCW MPI_COMM_WORLD
#define p 12
#define m 3
#define n 4

    void main(int argc, char *argv[])
{
   int rank, i;

   MPI_Init(&argc, &argv);
   MPI_Comm_rank(MCW, &rank);

   MPI_Statu st;

   int dim_size[] = {m, n};
   int periods[] = {1, 0};

   MPI_Comm cartcomm;

   MPI_Cart_create(MCW, 2, dim_size, periods, 1, &cartcomm);

   int novi_rank;
   MPI_Comm_rank(cartcomm, &novi_rank);

   int coords[2];
   MPI_Cart_coords(cartcomm, novi_rank, 2, coords);

   int niz[] = {rank, coords[0], coords[1]};
   int local_niz[3];

   int right, left;
   MPI_Cart_shift(cartcomm, 0, 2, &left, &right);

   MPI_Datatype newtype;

   MPI_Type_cotiguous(3, MPI_INT, &newtype);
   MPI_Type_commit(&newtype);

   if (coords[1] % 2 == 0)
   {
      MPI_Sendrecv(niz, 1, newtype, right, 7, local_niz, 1, newtype, left, 7, cartcomm, &st);

      for (i = 0; i < 3; i++)
      {
         local_niz[i] += left;
         printf("%d", local_niz[i]);
      }
   }
   else
   { // procesi u neparnim kolonama

      for (i = 0; i < 3; i++)
      {
         printf("%d", niz[i]);
      }

      MPI_Sendrecv(niz, 1, newtype, right, 8, local_niz, 1, newtype, left, 8, cartcomm, &st);

      for (i = 0; i < 3; i++)
      {
         local_niz[i] += left;
         printf("%d", local_niz[i]);
      }
   }

   MPI_Finalize();
   free(Mina);
}
