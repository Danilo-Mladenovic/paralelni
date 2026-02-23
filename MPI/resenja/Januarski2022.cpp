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

	// Write/Create 10MB file
	int my_size = FILE_SIZE / size,
			numb = my_size / sizeof(int),
			*data = (int *)malloc(my_size);
	for (int i = 0, start = rank * numb; i < numb; i++)
	{
		data[i] = i * rank + start;
	}

	printf("Thread: %d\n", rank);
	for (int i = 0, start = rank * numb; i < numb; i++)
	{
		printf("data[%d] = %d\n", i, data[i]);
	}

	MPI_File fp;
	MPI_File_open(MPI_COMM_WORLD, "podaci.dat", MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &fp);
	MPI_File_write_at_all(fp, (double)rank * my_size, data, numb, MPI_INT, MPI_STATUS_IGNORE);
	MPI_File_close(&fp);
	free(data);

	MPI_File fp;
	MPI_Offset fileSize;

	MPI_File_open(MPI_COMM_WORLD, "podaci.dat", MPI_MODE_RDONLY, MPI_INFO_NULL, &fp);
	MPI_File_get_size(fp, &fileSize);
	int numOfBytes = FILE_SIZE / size;
	int NumofInt = numOfBytes / sizeof(int);
	int *data_in = (int *)malloc(sizeof(int) * NumofInt); // malloc(OVDE MOZE I "numOfBytes")
	printf("Thread: %d BEFORE READ\n", rank);

	for (size_t i = 0; i < NumofInt; i++)
	{
		data_in[i] = -50;
		printf("%d\t", data_in[i]);
	}

	MPI_File_read_shared(fp, data_in, NumofInt, MPI_INT, MPI_STATUS_IGNORE);

	printf("Thread: %d\n", rank);

	for (int i = 0; i < NumofInt; i++)
	{
		printf("%d\t", data_in[i]);
	}

	MPI_File_close(&fp);

	MPI_Datatype newType;
	MPI_Type_vector(2, NumofInt / 2, size * NumofInt / 2, MPI_INT, &newType);
	MPI_Type_commit(&newType);

	MPI_File_open(MPI_COMM_WORLD, "podaci2.dat", MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &fp);

	MPI_File_set_view(fp, rank * NumofInt / 2 * sizeof(int), MPI_INT, newType, "native", MPI_INFO_NULL);

	MPI_File_write_all(fp, data_in, NumofInt, MPI_INT, MPI_STATUS_IGNORE);

	MPI_File_close(&fp);

	MPI_Finalize();

	return 0;
}