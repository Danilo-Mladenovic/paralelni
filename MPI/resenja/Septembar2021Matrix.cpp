#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <iostream>

const int BUF_SIZE = 9; // 1024 * 1024;

bool CheckArrays(int *niz1, int *niz2)
{

	bool flag = true;
	for (size_t i = 0; i < BUF_SIZE; i++)
	{
		printf("niz1[%d] = %d  niz2[%d] = %d\n", i, niz1[i], i, niz2[i]);
		if (niz1[i] != niz2[i])
			flag = false;
	}
	return flag;
}

int main(int argc, char **argv)
{
	int rank, size;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	int Niz[BUF_SIZE];
	for (int i = 0; i < BUF_SIZE; i++)
	{
		Niz[i] = i; // rand() % 100;
	}

	MPI_File fp;

	MPI_File_open(MPI_COMM_WORLD, "file1.dat", MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &fp);

	int offset = (size - rank - 1) * BUF_SIZE * sizeof(int);
	MPI_File_seek(fp, offset, MPI_SEEK_SET);

	MPI_File_write_all(fp, Niz, BUF_SIZE, MPI_INT, MPI_STATUS_IGNORE);

	MPI_File_close(&fp);

	// 2. korak

	int Niz2[BUF_SIZE];
	for (size_t i = 0; i < BUF_SIZE; i++)
	{
		Niz2[i] = 0;
	}

	MPI_File fp2;
	MPI_File_open(MPI_COMM_WORLD, "file1.dat", MPI_MODE_RDONLY, MPI_INFO_NULL, &fp2);

	offset = (size - rank - 1) * BUF_SIZE * sizeof(int);
	MPI_File_read_at(fp2, offset, Niz2, BUF_SIZE, MPI_INT, MPI_STATUS_IGNORE);

	MPI_File_close(&fp2);

	if (CheckArrays(Niz, Niz2))
	{
		printf("NIZOVI JESU ISTI!\n");
	}
	else
	{
		printf("NIZOVI NISU ISTI!\n");
	}
	int m, n;
	printf("Unesite m:\n");
	// scanf("%d", &m);
	// std::cin >> m;

	printf("Unesite n:\n");
	// scanf("%d", &n);
	// std::cin >> n;
	m = 6, n = 9;
	int ndims = 2;
	int gsizes[2] = {m, n};

	printf("Unos korektan!\n m = %d, n = %d", m, n);

	int distribs[2] = {MPI_DISTRIBUTE_BLOCK, MPI_DISTRIBUTE_BLOCK};
	int dargs[2] = {MPI_DISTRIBUTE_DFLT_DARG, MPI_DISTRIBUTE_DFLT_DARG};

	int psizes[2] = {2, 3};
	MPI_Datatype newType;

	MPI_Type_create_darray(size, rank, ndims, gsizes, distribs, dargs, psizes, MPI_ORDER_C, MPI_INT, &newType);
	MPI_Type_commit(&newType);

	MPI_File fp3;
	MPI_File_open(MPI_COMM_WORLD, "file2.dat", MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &fp3);

	MPI_File_set_view(fp3, 0, MPI_INT, newType, "native", MPI_INFO_NULL);

	MPI_File_write_all(fp3, Niz2, BUF_SIZE, MPI_INT, MPI_STATUS_IGNORE);

	MPI_File_close(&fp3);

	MPI_Finalize();

	return 0;
}