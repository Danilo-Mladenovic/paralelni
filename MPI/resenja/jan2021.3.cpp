#include <mpi.h>
#include <stdio.h>

#define N 15
#define Q 5
// 25 procesa

void mainjan20213(int argc, char *argv[])
{
	MPI_Init(&argc, &argv); // Finalize
	int rank_w, size_w;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank_w);
	MPI_Comm_size(MPI_COMM_WORLD, &size_w);

	int column = rank_w % Q;
	int row = rank_w / Q;

	int A[N][N], B[N][N], C[N][N], C_local[N / Q][N / Q], A_local[N / Q][N], B_local[N][N / Q], C_seq[N][N], C_partial[N / Q][N];

	if (rank_w == 0)
	{
		for (int i = 0; i < N; i++)
		{
			for (int j = 0; j < N; j++)
			{
				A[i][j] = i + j;
				B[i][j] = i - j;
			}
		}

		printf("C:\n");
		for (int i = 0; i < N; i++)
		{
			for (int j = 0; j < N; j++)
			{
				C_seq[i][j] = 0;
				for (int k = 0; k < N; k++)
				{
					C_seq[i][j] += A[i][k] * B[k][j];
				}
				printf("%d, ", C_seq[i][j]);
			}
			printf("\n");
		}
	}
	MPI_Bcast(A, N * N, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(B, N * N, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(C_seq, N * N, MPI_INT, 0, MPI_COMM_WORLD);

	MPI_Comm column_comm, row_comm;
	MPI_Comm_split(MPI_COMM_WORLD, column, 1, &column_comm);
	MPI_Comm_split(MPI_COMM_WORLD, row, 1, &row_comm);

	MPI_Datatype row_t, row_adj, column_t, column_adj, piece, piece_adj;

	MPI_Type_vector(N / Q, N, N * Q, MPI_INT, &row_t);
	MPI_Type_create_resized(row_t, 0, N * sizeof(int), &row_adj);
	MPI_Type_commit(&row_adj);

	MPI_Type_vector(N * N / Q, 1, Q, MPI_INT, &column_t);
	MPI_Type_create_resized(column_t, 0, sizeof(int), &column_adj);
	MPI_Type_commit(&column_adj);

	MPI_Type_vector(N * N / (Q * Q), 1, Q, MPI_INT, &piece);
	MPI_Type_create_resized(piece, 0, sizeof(int), &piece_adj);
	MPI_Type_commit(&piece_adj);

	if (column == 0)
		MPI_Scatter(A, 1, row_adj, A_local, N * N / Q, MPI_INT, 0, column_comm);
	MPI_Bcast(A_local, N * N / Q, MPI_INT, 0, row_comm);

	for (int i = 0; i < N / Q; i++)
	{
		for (int j = 0; j < N; j++)
		{
			if (A_local[i][j] != A[i * Q + row][j])
				printf("Greska u procesu %d za A, na lokaciji (%d, %d)\n", rank_w, i, j);
		}
	}

	if (row == 0)
		MPI_Scatter(B, 1, column_adj, B_local, N * N / Q, MPI_INT, 0, row_comm);
	MPI_Bcast(B_local, N * N / Q, MPI_INT, 0, column_comm);

	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N / Q; j++)
		{
			if (B_local[i][j] != B[i][j * Q + column])
				printf("Greska u procesu %d za B, na lokaciji (%d, %d)\n", rank_w, i, j);
		}
	}

	for (int i = 0; i < N / Q; i++)
	{
		for (int j = 0; j < N / Q; j++)
		{
			C_local[i][j] = 0;
			for (int k = 0; k < N; k++)
				C_local[i][j] += A_local[i][k] * B_local[k][j];
			if (C_local[i][j] != C_seq[i * Q + row][j * Q + column])
				printf("Greska u procesu %d za C_local, na lokaciji (%d, %d)\n", rank_w, i, j);
		}
	}

	MPI_Gather(C_local, (N * N) / (Q * Q), MPI_INT, C_partial, 1, piece_adj, 0, row_comm);
	if (column == 0)
		MPI_Gather(C_partial, N * N / Q, MPI_INT, C, 1, row_adj, 0, column_comm);

	if (rank_w == 0)
	{
		printf("Paralelno resenje:\n");
		for (int i = 0; i < N; i++)
		{
			for (int j = 0; j < N; j++)
				printf("%d, ", C[i][j]);
			printf("\n");
		}

		for (int i = 0; i < N; i++)
		{
			for (int j = 0; j < N; j++)
			{
				if (C[i][j] != C_seq[i][j])
					printf("Greska! Lokacija (%d, %d)\n", i, j);
			}
		}
	}

	MPI_Finalize();
}