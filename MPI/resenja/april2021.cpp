#include <stdio.h>
#include <math.h>
#include "mpi.h"
#include <stdlib.h>
#define m 8

void print_matrix(int *matrix, int x, int y)
{
	int i = 0, j = 0;

	for (i = 0; i < x; i++)
	{
		for (j = 0; j < y; j++)
		{
			printf("%3d ", *((matrix + i * y) + j));
		}
		printf("\n");
	}
	printf("\n");
}

void print_array(int *array, int x)
{
	for (int i = 0; i < x; i++)
		printf("%3d ", *(array + i));
	printf("\n");
}

void main(int argc, char *argv[])
{
	int A[m][m];
	int B[m];
	int C[m];
	int result_C[m];
	int processWhichPrints;
	int p;
	int q;
	int rank, i, j;
	int local_max, global_max;
	MPI_Datatype subMatrica;
	MPI_Datatype vektor;
	MPI_Status status;
	int offsetI, offsetJ;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &p);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	q = sqrt(p);
	MPI_Type_vector(q + 1, q + 1, m, MPI_INT, &subMatrica);
	MPI_Type_commit(&subMatrica);

	MPI_Type_vector(1, q + 1, 0, MPI_INT, &vektor);
	MPI_Type_commit(&vektor);

	for (i = 0; i < m; i++)
	{
		for (j = 0; j < m; j++)
		{
			A[i][j] = 0;
		}
	}
	for (i = 0; i < m; i++)
	{
		B[i] = 0;
		C[i] = 0;
		result_C[i] = 0;
	}

	if (rank == 0)
	{
		for (i = 0; i < m; i++)
		{
			for (j = 0; j < m; j++)
			{
				A[i][j] = j - i; // rand() % 100;
			}
		}
		for (i = 0; i < m; i++)
			B[i] = j - i; // rand() % 100;

		printf("\nA:\n");
		print_matrix((int *)A, m, m);
		printf("\nB:\n");
		print_array((int *)B, m);

		for (i = 1; i < p; i++)
		{
			offsetI = i / q;
			offsetJ = i % q;
			MPI_Send(&A[offsetI][offsetJ], 1, subMatrica, i, 0, MPI_COMM_WORLD);

			MPI_Send(&B[offsetJ], 1, vektor, i, 0, MPI_COMM_WORLD);
		}
	}
	else
	{
		offsetI = rank / q;
		offsetJ = rank % q;
		MPI_Recv(&A[offsetI][offsetJ], 1, subMatrica, 0, 0, MPI_COMM_WORLD, &status);
		MPI_Recv(&B[offsetJ], 1, vektor, 0, 0, MPI_COMM_WORLD, &status);
	}

	// mnozenje matrica
	for (int i = rank / q; i < 2 * q; i += q)
	{
		for (int j = rank % q; j < 2 * q; j += q)
		{
			C[i] += A[i][j] * B[j];
		}
	}

	// trazenje max
	local_max = INT16_MIN;
	// for (int i = rank / q; i < q + 1; i++) {
	//	for (int j = rank % q; j < q + 1; j++) {
	//		if (A[i][j] > local_max)
	//			local_max = A[i][j];
	//	}
	// }

	for (int j = 0; j < m; j++)
	{
		if (C[j] > local_max)
			local_max = C[j];
	}

	MPI_Reduce(&local_max, &global_max, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);
	MPI_Bcast(&global_max, 1, MPI_INT, 0, MPI_COMM_WORLD);

	MPI_Reduce(&C[0], &result_C[0], m, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

	MPI_Bcast(&result_C, m, MPI_INT, 0, MPI_COMM_WORLD);

	MPI_Barrier(MPI_COMM_WORLD);
	// stampanje rezultata
	if (local_max == global_max)
	{
		printf("My rank is:%d my local_max is: %d\n", rank, local_max);

		printf("A:\n");

		for (int i = 0; i < m; i++)
		{

			for (int j = 0; j < m; j++)
			{
				printf("%3d ", A[i][j]);
			}
			printf("\n");
		}
		printf("B:\n");
		for (int i = 0; i < m; i++)
		{

			printf("%3d ", B[i]);
		}
		printf("\n");

		printf("local_C:\n");
		print_array((int *)C, m);

		printf("result_C:\n");
		for (int i = 0; i < m; i++)
		{

			printf("%3d ", result_C[i]);
		}
		printf("\n");
	}

	MPI_Finalize();
}