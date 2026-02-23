#include <stdio.h>
#include <mpi.h>

#define M 8
#define Q 4

void mainapr20213(int argc, char *argv[])
{
	MPI_Init(&argc, &argv); // Finalize

	int rank_w, size_w;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank_w);
	MPI_Comm_size(MPI_COMM_WORLD, &size_w);

	int A[M][M], B[M], C[M], C_seq[M], C_local[M / Q], B_local[M / Q], A_local[M / Q][M / Q], C_total[M];
	struct max_loc
	{
		int rank;
		int value;
	};
	max_loc max_loc, max_loc_global;
	int local_max = -1;

	MPI_Datatype deoA, deoB, deoB_adj;

	int count = (M * M) / (Q * Q);
	int *blocklenghts = new int[count]; // delete
	int *displ = new int[count];				// delete
	for (int i = 0; i < count; i++)
		blocklenghts[i] = 1;
	for (int i = 0; i < M / Q; i++)
	{
		for (int j = 0; j < M / Q; j++)
		{
			displ[i * M / Q + j] = M * Q * i + j * Q;
		}
	}
	MPI_Type_indexed(count, blocklenghts, displ, MPI_INT, &deoA);
	MPI_Type_commit(&deoA);

	MPI_Type_vector(M / Q, 1, Q, MPI_INT, &deoB);
	MPI_Type_create_resized(deoB, 0, sizeof(int), &deoB_adj);
	MPI_Type_commit(&deoB_adj);

	if (rank_w == 0)
	{
		for (int i = 0; i < M; i++)
			for (int j = 0; j < M; j++)
				A[i][j] = i + j;
		for (int i = 0; i < M; i++)
			B[i] = i;

		printf("C_seq: ");
		for (int i = 0; i < M; i++)
		{
			C_seq[i] = 0;
			for (int j = 0; j < M; j++)
			{
				C_seq[i] += A[i][j] * B[j];
			}
			printf("%d, ", C_seq[i]);
		}
		printf("\n");

		for (int i = 1; i < size_w; i++)
		{
			int row = i / Q;
			int col = i % Q;
			MPI_Send(&A[row][col], 1, deoA, i, 1, MPI_COMM_WORLD);
		}

		MPI_Request request;
		MPI_Isend(&A[0][0], 1, deoA, 0, 1, MPI_COMM_WORLD, &request);
		MPI_Recv(A_local, (M * M) / (Q * Q), MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	}
	else
		MPI_Recv(A_local, (M * M) / (Q * Q), MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

	MPI_Bcast(A, M * M, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(B, M, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(C_seq, M, MPI_INT, 0, MPI_COMM_WORLD);

	int row = rank_w / Q;
	int column = rank_w % Q;

	for (int i = 0; i < M / Q; i++)
	{
		for (int j = 0; j < M / Q; j++)
		{
			if (A_local[i][j] != A[i * Q + row][j * Q + column])
				printf("Greska u procesu %d za A_local, na poziciji (%d, %d)\n", rank_w, i, j);
		}
	}

	MPI_Comm row_comm, column_comm;
	MPI_Comm_split(MPI_COMM_WORLD, row, 1, &row_comm);
	MPI_Comm_split(MPI_COMM_WORLD, column, 1, &column_comm);

	if (row == 0)
		MPI_Scatter(B, 1, deoB_adj, B_local, M / Q, MPI_INT, 0, row_comm);
	MPI_Bcast(B_local, M / Q, MPI_INT, 0, column_comm);

	for (int i = 0; i < M / Q; i++)
	{
		C_local[i] = 0;
		for (int j = 0; j < M / Q; j++)
		{
			C_local[i] += A_local[i][j] * B_local[j];
		}
		if (C_local[i] > local_max)
			local_max = C_local[i];
	}
	MPI_Gather(C_local, M / Q, MPI_INT, C, 1, deoB_adj, 0, column_comm);
	if (row == 0)
		MPI_Reduce(C, C_total, M, MPI_INT, MPI_SUM, 0, row_comm);

	max_loc.rank = rank_w;
	max_loc.value = local_max;

	MPI_Reduce(&max_loc, &max_loc_global, 1, MPI_2INT, MPI_MAXLOC, 0, MPI_COMM_WORLD);
	MPI_Bcast(&max_loc_global, 1, MPI_2INT, 0, MPI_COMM_WORLD);

	MPI_Request request;
	if (rank_w == 0)
	{
		MPI_Isend(C_total, M, MPI_INT, max_loc_global.rank, 1, MPI_COMM_WORLD, &request);
	}
	if (rank_w == max_loc_global.rank)
	{
		MPI_Recv(C, M, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		printf("C: ");
		for (int i = 0; i < M; i++)
			printf("%d, ", C[i]);
		printf("\n");
	}

	delete blocklenghts;
	delete displ;

	MPI_Finalize();
}