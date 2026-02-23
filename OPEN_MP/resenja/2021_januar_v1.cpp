#include <stdio.h>
#include <omp.h>

#define N 10

void mainjan2021()
{
	int t = 1, t_seq = 1;
	int A[N], B[N - 1], C[N - 1], A_copy[N], A_seq[N];

	for (int i = 0; i < N; i++)
		A[i] = i + 1;
	for (int i = 0; i < N - 1; i++)
	{
		B[i] = 2 * i;
		C[i] = 3 * (i + 2);
	}

#pragma omp parallel for
	for (int i = 0; i < N; i++)
		A_seq[i] = A[i];

	for (int i = 0; i < N - 1; i++)
	{
		A_seq[i] = A_seq[i + 1] + B[i] * C[i];
		t_seq *= A_seq[i];
	}

#pragma omp parallel for
	for (int i = 0; i < N; i++)
		A_copy[i] = A[i];

#pragma omp parallel for reduction(* : t)
	for (int i = 0; i < N - 1; i++)
	{
		A[i] = A_copy[i + 1] + B[i] * C[i];
		t *= A[i];
	}

#pragma omp parallel for
	for (int i = 0; i < N; i++)
		if (A_seq[i] != A[i])
			printf("Greska na lokaciji %d\n", i);

	if (t != t_seq)
		printf("Greska za t\n");
}