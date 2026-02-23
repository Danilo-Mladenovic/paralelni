#include <stdio.h>
#include <mpi.h>

#define N 8
#define M 12
#define K 5
#define P 4

void mainapr20203(int argc, char *argv[])
{
	MPI_Init(&argc, &argv);
	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	int A[M][N], B[N][K], C[M][K], C_seq[M][K], A_local[M / P][N], C_local[M / P][K];
	int my_min = INT32_MAX;

	struct min_loc
	{
		int rank;
		int value;
	};
	min_loc my_min_loc, global_min_loc;

	if (rank == 0)
	{
		for (int i = 0; i < M; i++)
			for (int j = 0; j < N; j++)
				A[i][j] = i + j;

		for (int i = 0; i < N; i++)
			for (int j = 0; j < K; j++)
				B[i][j] = i + j;

		printf("C seq:\n");
		for (int i = 0; i < M; i++)
		{
			for (int j = 0; j < K; j++)
			{
				C_seq[i][j] = 0;
				for (int k = 0; k < N; k++)
					C_seq[i][j] += A[i][k] * B[k][j];
				printf("%d, ", C_seq[i][j]);
			}
			printf("\n");
		}
		printf("\n");
	}

	MPI_Datatype vrste, vrste_adj;
	MPI_Type_vector(M / P, N, P * N, MPI_INT, &vrste);
	MPI_Type_create_resized(vrste, 0, N * sizeof(int), &vrste_adj);
	MPI_Type_commit(&vrste_adj);

	MPI_Datatype vrste_C, vrste_C_adj;
	MPI_Type_vector(M / P, K, P * K, MPI_INT, &vrste_C);
	MPI_Type_create_resized(vrste_C, 0, K * sizeof(int), &vrste_C_adj);
	MPI_Type_commit(&vrste_C_adj);

	MPI_Scatter(A, 1, vrste_adj, A_local, M * N / P, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(B, N * K, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(C_seq, M * K, MPI_INT, 0, MPI_COMM_WORLD);

	for (int i = 0; i < M / P; i++)
	{
		for (int j = 0; j < K; j++)
		{
			C_local[i][j] = 0;
			for (int k = 0; k < N; k++)
			{
				C_local[i][j] += A_local[i][k] * B[k][j];
			}
			if (C_local[i][j] < my_min)
				my_min = C_local[i][j];
			if (C_local[i][j] != C_seq[i * P + rank][j])
				printf("Proces %d: Greska u C_local na lokaciji (%d, %d)\n", rank, i, j);
		}
	}

	my_min_loc.rank = rank;
	my_min_loc.value = my_min;
	MPI_Reduce(&my_min_loc, &global_min_loc, 1, MPI_2INT, MPI_MINLOC, 0, MPI_COMM_WORLD);
	MPI_Bcast(&global_min_loc, 1, MPI_2INT, 0, MPI_COMM_WORLD);

	MPI_Gather(C_local, M * K / P, MPI_INT, C, 1, vrste_C_adj, global_min_loc.rank, MPI_COMM_WORLD);

	if (rank == global_min_loc.rank)
	{
		printf("C:\n");
		for (int i = 0; i < M; i++)
		{
			for (int j = 0; j < K; j++)
				printf("%d, ", C[i][j]);
			printf("\n");
		}

		for (int i = 0; i < M; i++)
		{
			for (int j = 0; j < K; j++)
			{
				if (C[i][j] != C_seq[i][j])
					printf("Greska u izracunavaju na lokaciji (%d, %d)\n", i, j);
			}
		}
	}

	MPI_Finalize();
}