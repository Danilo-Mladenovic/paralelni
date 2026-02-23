#include <stdio.h>
#include <mpi.h>

#define N 4

void mainapr20204(int argc, char *argv[])
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

		MPI_Datatype complex;
		int count = N * N / 2;
		int *displ = new int[count];
		int *blkln = new int[count];
		int ind = 0;
		int val = 0;
		for (int i = 0; i < N / 4; i++)
		{
			for (int j = 0; j < N / 2; j++)
			{
				blkln[ind] = 1;
				displ[ind] = val;
				val++;
				ind++;
			}
			val += N - 1;
			for (int j = 0; j < N / 2; j++)
			{
				blkln[ind] = 1;
				displ[ind] = val;
				val--;
				ind++;
			}
			val += N + 1;
		}
		for (int i = 0; i < N / 2; i++)
		{
			for (int j = 1; j <= N / 2; j++)
			{
				blkln[ind] = 1;
				displ[ind] = N * (N / 2 - j) + i;
				ind++;
			}
		}
		MPI_Type_indexed(count, blkln, displ, MPI_INT, &complex);
		MPI_Type_commit(&complex);

		MPI_Send(&(A[0][N / 2]), 1, complex, 1, 1, MPI_COMM_WORLD);
		MPI_Send(&(A[N / 2][0]), 1, complex, 2, 1, MPI_COMM_WORLD);
		MPI_Send(&(A[N / 2][N / 2]), 1, complex, 3, 1, MPI_COMM_WORLD);

		delete blkln;
		delete displ;
	}
	else
	{
		for (int i = 0; i < N; i++)
			for (int j = 0; j < N; j++)
				C[i][j] = 0;

		MPI_Datatype dijagonalno;
		int count = N * N / 2;
		int *blkln = new int[count];
		int *displ = new int[count];
		int ind = 0;
		for (int i = 0; ind < count; i++)
			for (int j = 1; j <= N - i && ind < count; j++)
			{
				blkln[ind] = 1;
				displ[ind] = (i + j) * N - j;
				ind++;
			}
		MPI_Type_indexed(count, blkln, displ, MPI_INT, &dijagonalno);
		MPI_Type_commit(&dijagonalno);

		MPI_Recv(C, 1, dijagonalno, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

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