#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <math.h>

#define n 16
#define k 8
#define s 4

void main(int argc, char **argv)
{
  int A[n][n], B[n][n], C[n][n], max, loc_max, max_id, loc_id, rank, size

      int *local_a,
      local_b;

  MPI_Init(&rgc, &argv);
  MPI_Comm_rank(&rank);
}