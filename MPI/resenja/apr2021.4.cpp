#include <stdio.h>
#include <mpi.h>

#define N 4

void mainapr20214(int argc, char *argv[])
{
	// su za jun 2020.4
	MPI_Init(&argc, &argv);
	int rank_w;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank_w);

	MPI_Comm matrica;
	int dim_size[2] = {N, N}; // N-1, N
	int periods[2] = {0, 1};
	MPI_Cart_create(MPI_COMM_WORLD, 2, dim_size, periods, 1, &matrica);

	int coords[2];
	int rank_c;
	MPI_Comm_rank(matrica, &rank_c);
	MPI_Cart_coords(matrica, rank_c, 2, coords);
	int x = coords[0] + coords[1];

	int src, dst;
	MPI_Cart_shift(matrica, 1, -coords[0], &src, &dst); // coords[0]
	MPI_Sendrecv_replace(&x, 1, MPI_INT, dst, 0, src, 0, matrica, MPI_STATUS_IGNORE);

	printf("Proces koor (%d, %d) : %d\n", coords[0], coords[1], x);

	MPI_Finalize();
}