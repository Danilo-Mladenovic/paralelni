#include <stdio.h>
#include <mpi.h>

#define N 8
#define Q 4

void mainjun20203(int argc, char *argv[])
{
	MPI_Init(&argc, &argv);
	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	int A[N][N], B[N], C[N], C_seq[N], A_local[2][Q + 1], B_local[2], C_local[2], C_partial[2], A_partial[2][N];
	int row = rank / Q;
	int col = rank % Q;

	int my_min = INT32_MAX;
	struct min_loc
	{
		int rank;
		int value;
	};
	min_loc my_min_loc, global_min_loc;

	MPI_Comm colcomm, rowcomm;
	MPI_Comm_split(MPI_COMM_WORLD, row, 1, &rowcomm);
	MPI_Comm_split(MPI_COMM_WORLD, col, 1, &colcomm);

	MPI_Datatype B_part, B_part_adj, A_part, A_part_adj;
	MPI_Type_vector(2, 1, Q, MPI_INT, &B_part);
	MPI_Type_create_resized(B_part, 0, sizeof(int), &B_part_adj);
	MPI_Type_commit(&B_part_adj);

	MPI_Type_vector(2, Q + 1, N, MPI_INT, &A_part);
	MPI_Type_commit(&A_part);
	MPI_Type_create_resized(A_part, 0, sizeof(int), &A_part_adj);
	MPI_Type_commit(&A_part_adj);

	if (rank == 0)
	{
		for (int i = 0; i < N; i++)
		{
			for (int j = 0; j < N; j++)
				A[i][j] = i + j;
		}

		for (int i = 0; i < N; i++)
			B[i] = i + 1;

		printf("C seq:\n");
		for (int i = 0; i < N; i++)
		{
			C_seq[i] = 0;
			for (int j = 0; j < N; j++)
			{
				C_seq[i] += A[i][j] * B[j];
			}
			printf("%d, ", C_seq[i]);
		}
		printf("\n");
	}

	// a
	/*if (rank == 0)
	{
		for (int i = 1; i < size; i++)
			MPI_Send(&(A[2 * (i / Q)][i % Q]), 1, A_part, i, 1, MPI_COMM_WORLD);

		MPI_Request request;
		MPI_Isend(&(A[0][0]), 1, A_part, 0, 1, MPI_COMM_WORLD, &request);
	}
	MPI_Recv(A_local, 2 * (Q + 1), MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);*/

	// b
	if (col == 0)
		MPI_Scatter(A, 2 * N, MPI_INT, A_partial, 2 * N, MPI_INT, 0, colcomm);
	MPI_Scatter(A_partial, 1, A_part_adj, A_local, 2 * (Q + 1), MPI_INT, 0, rowcomm);

	MPI_Bcast(A, N * N, MPI_INT, 0, MPI_COMM_WORLD);
	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < Q + 1; j++)
			if (A_local[i][j] != A[row * 2 + i][col + j])
				printf("Proces %d: Greska u A_local na lokaciji (%d, %d)\n", rank, i, j);
	}

	if (row == 0)
		MPI_Scatter(B, 1, B_part_adj, B_local, 2, MPI_INT, 0, rowcomm);
	MPI_Bcast(B_local, 2, MPI_INT, 0, colcomm);

	for (int i = 0; i < 2; i++)
		for (int j = 0; j < Q + 1; j++)
			if (A_local[i][j] < my_min)
				my_min = A_local[i][j];

	C_local[0] = A_local[0][0] * B_local[0] + A_local[0][Q] * B_local[1];
	C_local[1] = A_local[1][0] * B_local[0] + A_local[1][Q] * B_local[1];

	MPI_Reduce(C_local, C_partial, 2, MPI_INT, MPI_SUM, 0, rowcomm);
	if (col == 0)
		MPI_Gather(C_partial, 2, MPI_INT, C, 2, MPI_INT, 0, colcomm);

	my_min_loc.rank = rank;
	my_min_loc.value = my_min;
	MPI_Reduce(&my_min_loc, &global_min_loc, 1, MPI_2INT, MPI_MINLOC, 0, MPI_COMM_WORLD);
	MPI_Bcast(&global_min_loc, 1, MPI_2INT, 0, MPI_COMM_WORLD);

	if (rank == 0)
	{
		MPI_Request request;
		MPI_Isend(C, N, MPI_INT, global_min_loc.rank, 1, MPI_COMM_WORLD, &request);
	}
	if (rank == global_min_loc.rank)
	{
		MPI_Recv(C, N, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		printf("C:\n");
		for (int i = 0; i < N; i++)
			printf("%d, ", C[i]);
		printf("\n");

		for (int i = 0; i < N; i++)
			if (C[i] != C_seq[i])
				printf("Greska na lokaciji %d\n", i);
	}

	/*if (rank == 0)
	{
		printf("C:\n");
		for (int i = 0; i < N; i++)
			printf("%d, ", C[i]);
		printf("\n");

		for (int i = 0; i < N; i++)
			if (C[i] != C_seq[i])
				printf("Greska na lokaciji %d\n", i);
	}*/

	MPI_Finalize();
}