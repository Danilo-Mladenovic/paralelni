#include <stdio.h>
#include <mpi.h>

#define N 12
#define Q 4

void maindec20203(int argc, char *argv[])
{
	MPI_Init(&argc, &argv);
	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	int A[N][N], B[N][N], C[N][N], C_seq[N][N], A_local[N / Q][N], B_local[N][N / Q], C_local[N / Q][N / Q], C_partial[N / Q][N];
	int row = rank / Q;
	int column = rank % Q;

	MPI_Comm rowcomm, colcomm;
	MPI_Comm_split(MPI_COMM_WORLD, row, 1, &rowcomm);
	MPI_Comm_split(MPI_COMM_WORLD, column, 1, &colcomm);

	MPI_Datatype vrstaA, vrstaA_adj, deoC, deoC_adj;
	MPI_Type_vector(N, N / Q, N, MPI_INT, &vrstaA);
	MPI_Type_create_resized(vrstaA, 0, N / Q * sizeof(int), &vrstaA_adj);
	MPI_Type_commit(&vrstaA_adj);

	MPI_Type_vector(N / Q, N / Q, N, MPI_INT, &deoC);
	MPI_Type_create_resized(deoC, 0, N / Q * sizeof(int), &deoC_adj);
	MPI_Type_commit(&deoC_adj);

	if (rank == 0)
	{
		for (int i = 0; i < N; i++)
			for (int j = 0; j < N; j++)
			{
				A[i][j] = i + j;
				B[i][j] = i - j;
			}

		printf("C seq:\n");
		for (int i = 0; i < N; i++)
		{
			for (int j = 0; j < N; j++)
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

	if (column == 0)
		MPI_Scatter(A, N * N / Q, MPI_INT, A_local, N * N / Q, MPI_INT, 0, colcomm);
	MPI_Bcast(A_local, N * N / Q, MPI_INT, 0, rowcomm);

	if (row == 0)
		MPI_Scatter(B, 1, vrstaA_adj, B_local, N * N / Q, MPI_INT, 0, rowcomm);
	MPI_Bcast(B_local, N * N / Q, MPI_INT, 0, colcomm);

	for (int i = 0; i < N / Q; i++)
		for (int j = 0; j < N / Q; j++)
		{
			C_local[i][j] = 0;
			for (int k = 0; k < N; k++)
				C_local[i][j] += A_local[i][k] * B_local[k][j];
		}

	MPI_Gather(C_local, (N * N) / (Q * Q), MPI_INT, C_partial, 1, deoC_adj, 0, rowcomm);
	if (column == 0)
		MPI_Gather(C_partial, N * N / Q, MPI_INT, C, N * N / Q, MPI_INT, 0, colcomm);

	if (rank == 0)
	{
		printf("C:\n");
		for (int i = 0; i < N; i++)
		{
			for (int j = 0; j < N; j++)
				printf("%d, ", C[i][j]);
			printf("\n");
		}
		printf("\n");

		for (int i = 0; i < N; i++)
		{
			for (int j = 0; j < N; j++)
			{
				if (C[i][j] != C_seq[i][j])
					printf("Greska na poziciji (%d, %d)\n", i, j);
			}
		}
	}

	MPI_Finalize();
}