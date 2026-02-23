#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#define MCW MPI_COMM_WORLD
#define N 16

tyopedef struct
{

   int id;
   char ime[12];
   char prezime[12];
   float prosplata;
} Zaposleni;

void main(int argc, char *argv[])
{
   int rank, size, i, s;

   MPI_Init(&argc, &argv);
   MPI_Comm_rank(MCW, &rank);
   MPI_Comm_size(MCW, &size);

   s = N / size;

   Zaposleni *local_zaposleni;
   local_zaposleni = (Zaposleni *)malloc(s * sizeof(Zaposleni));

   Zaposleni niz_zaposlenih[N];

   if (rank == 0)
   {
      for (i = 0; i < N; i++)
      {
         scanf("%d", &niz_zaposlenih[i].id);
         scanf("%s", &niz_zaposlenih[i].ime);
         scanf("%s", &niz_zaposlenih[i].prezime);
         scanf("%f", &niz_zaposlenih[i].prosplata);
      }
   }

   int count = 4;
   int blocklenghts[] = {1, 12, 12, 1};
   MPI_Aint displacements[count];

   MPI_Aint base_address;
   Zaposleni offset;

   MPI_Address(&offset, &base_address);
   MPI_Address(&offset.id, &displacements[0]);
   MPI_Address(&offset.ime, &displaments[1]);
   MPI_Address(&offset.prezime, &displaments[2]);
   MPI_Address(&offset.prosplata, &displaments[3]);

   for (i = 0; i < count; i++)
   {
      displacements[i] -= base_address;
   }

   MPI_Datatype types[] = {MPI_INT, MPI_CHAR, MPI_CHAR, MPI_FLOAT};

   MPI_Datatype MPI_ZAPOSLENI;

   MPI_Type_struct(count, blocklenghts, displacements, types, &MPI_ZAPOSLENI);

   MPI_Scatter(niz_zaposlenih, s, MPI_ZAPOSLENI, local_zaposleni, s, MPI_ZAPOSLENI, 0, MCW);

   Zaposleni *local_min;

   local_min = (Zaposleni *)malloc(1 * sizeof(Zaposleni));

   local_min->prosplata = FLT_MAX;

   for (i = 0; i < s; i++)
   {
      if (local_zaposleni[i].prosplata < local_min->prosplata)
      {
         local_min->prosplata = local_zaposleni[i].prosplata;
         local_min->id = local_zaposleni[i].id;
      }
   }

   struct
   {

      float prosplata1;
      int id1;
   } global_min, global;

   global_min.prosplata1 = local_min->prosplata;
   global_min.id1 = local_min->id;

   MPI_Reduce(&global_min, &global, 1, MPI_FLOAT_INT, MPI_MINLOC, 0, MCW);

   if (rank == 0)
   {
      printf("%f%d", global.proslplata1, global.id1);
   }

   free(local_zaposleni);

   MPI_Finalize();
}
