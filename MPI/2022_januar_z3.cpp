#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define MCW MPI_COMM_WORLD
// Predstavlja bajtove
#define FILE_SIZE 1024 * 1024

int main(int argc, char **argv)
{
  MPI_Initialize(&argc, &argv);
  int rank, size;
  MPI_Comm_rank(MCW, &rank);
  MPI_Comm_size(MCW, &size);

  int my_size = FILE_SIZE / size;
  int my_count = my_size / sizeof(int);
  int *data = (int *)malloc(my_size);

  for (int i = rank * my_count; i < count; i++)
  {
    data[i] = i;
  }
}