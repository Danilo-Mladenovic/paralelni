#include <mpi.h>
#include <stdlib.h>
#include <iostream>

#define N 105

int main(int argc, char **argv)
{
  int rank, step;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  MPI_Offset displacement = sizeof(int) * N;
  int *writeData = (int *)malloc(displacement);
  for (int i = 0; i < N; i++)
    writeData[i] = i;

  MPI_File f1;
  MPI_File_open(MPI_COMM_WORLD, "file1.dat", MPI_MODE_CREATE | MPI_MODE_WRITE, MPI_INFO_NULL, &f1);
  MPI_File_seek(f1, (size - 1 - rank) * displacement, MPI_SEEK_SET);
  MPI_File_write(f1, writeData, N, MPI_INT, MPI_STATUS_IGNORE);
  MPI_File_close(&f1);

  int *readData = (int *)malloc(displacement);
  MPI_File f2;
  MPI_File_open(MPI_COMM_WORLD, "file1.dat", MPI_MODE_READ, MPI_INFO_NULL, &f2);
  MPI_File_read_at(f2, (size - 1 - rank) * displacement, readData, N, MPI_INT, MPI_STATUS_IGNORE);
  MPI_File_close(&f2);

  for (int i = 0; i < N; i++)
    cout << writeData[i] << '||' << readData[i] << endl;

  int n = (sqrt(1 + 8 * N) - 1) / 2;
  int *blocklengths = (int *)malloc(sizeof(int) * n);
  int *displacements = (int *)malloc(sizeof(int) * n);

  blocklengths[0] = 1;
  displacements[0] = 0;

  for (int i = 1; i < n; i++)
  {
    blocklengths[i] = i + 1;
    displacements[i] = displacements[i - 1] + size * i + rank * (i + 1);
  }

  MPI_Datatype fileType;
  MPI_Type_indexed(n, blocklengths, displacements, MPI_INT, &fileType);
  MPI_Type_commit(&fileType);

  free(blocklengths);
  free(displacements);

  MPI_File f3;
  MPI_File_open(MPI_COMM_WORLD, "file2.dat", MPI_MODE_CREATE | MPI_MODE_WRITE, MPI_INFO_NULL, &f3);
  MPI_File_set_view(f3, rank * sizeof(int), MPI_INT, fileType, "native", MPI_INFO_NULL);
  MPI_File_write_all(f3, &readData, N, fileType, MPI_STATUS_IGNORE)
}