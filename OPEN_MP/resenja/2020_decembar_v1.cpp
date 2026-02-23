#include <stdio.h>
#include <omp.h>

#define N 100
#define m 10

void main()
{
	int z, z_seq = m, X[N], Y[m + 2 * N * (N + 1)], X_seq[N];

	for (int i = 0; i < N; i++)
	{
		X[i] = i + 1;
		X_seq[i] = X[i];
	}
	for (int i = 0; i < m + 2 * N * (N + 1); i++)
		Y[i] = i * 2;

	for (int i = 0; i < N; i++)
		for (int j = 0; j < N; j++)
		{
			X_seq[j] += Y[z_seq];
			z_seq += 2;
		}

#pragma omp parallel for
	for (int j = 0; j < N; j++)
		for (int i = 0; i < N; i++)
			X[j] += Y[m + 2 * (N * i + j)];

	z = m + 2 * N * N;
	if (z != z_seq)
		printf("Greska za Z\n");
	for (int i = 0; i < N; i++)
		if (X[i] != X_seq[i])
			printf("Greska za X na lokaciji %d\n", i);
}