#include <stdio.h>
#include <mpi.h>

#define N 6

void mainokt20204(int argc, char *argv[])
{
	MPI_Init(&argc, &argv);
	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	int A[N][N], C[N][N];
	if (rank == 0)
	{
		for (int i = 0; i < N; i++)
			for (int j = 0; j < N; j++)
				A[i][j] = i * N + j;

		MPI_Datatype redovi;
		int count = N * N / 4;
		int *displ = new int[count];
		int *blkln = new int[count];
		for (int i = 0; i < N / 2; i++)
			for (int j = 0; j < N / 2; j++)
			{
				int ind = i * N / 2 + j;
				blkln[ind] = 2;
				displ[ind] = 2 * j * N + 2 * i;
			}
		MPI_Type_indexed(count, blkln, displ, MPI_INT, &redovi);
		MPI_Type_commit(&redovi);

		for (int i = 1; i < size; i++)
			MPI_Send(A, 1, redovi, i, 1, MPI_COMM_WORLD);

		delete blkln;
		delete displ;
	}
	else
	{
		for (int i = 0; i < N; i++)
			for (int j = 0; j < N; j++)
				C[i][j] = 0;

		MPI_Datatype obrnuta;
		int count = N * N / 2;
		int *blkln = new int[count];
		int *displ = new int[count];
		int ind = 0;
		for (int i = 1; ind < count; i++)
			for (int j = N; j >= i && ind < count; j--)
			{
				blkln[ind] = 1;
				displ[ind] = j * N - i;
				ind++;
			}
		MPI_Type_indexed(count, blkln, displ, MPI_INT, &obrnuta);
		MPI_Type_commit(&obrnuta);

		MPI_Recv(C, 1, obrnuta, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

		for (int i = 0; i < N; i++)
		{
			for (int j = 0; j < N; j++)
				printf("%d, ", C[i][j]);
			printf("\n");
		}
		printf("\n");

		delete displ;
		delete blkln;
	}

	MPI_Finalize();
}