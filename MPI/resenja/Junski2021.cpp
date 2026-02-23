#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>

#define ULL unsigned long long
#define MPI_ULL MPI_UNSIGNED_LONG_LONG

const ULL FILE_SIZE = 32; // 1024 * 1024;

int main(int argc, char **argv)
{
	int rank, size;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	MPI_Datatype newType;
	int count = 14;
	int *blockLengths = (int *)malloc(14 * sizeof(int));
	int *displacements = (int *)malloc(14 * sizeof(int));

	int stride = 0;
	for (int i = 1; i <= count; i++)
	{
		blockLengths[i - 1] = i;
	}

	displacements[0] = 0;
	for (int i = 1; i < count; i++)
	{
		displacements[i] = displacements[i - 1] + size * i + rank;
		printf("Thread %d, disp[%d] = %d\n", rank, i, displacements[i]);
	}

	MPI_Type_indexed(count, blockLengths, displacements, MPI_INT, &newType);
	MPI_Type_commit(&newType);

	MPI_Finalize();

	return 0;
}