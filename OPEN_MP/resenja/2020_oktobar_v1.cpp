#include <stdio.h>
#include <omp.h>

#define N 10

void mainokt2020()
{
	int x_seq = 1, x, Z[N], Z_seq[N];

	for (int k = 0; k < N; k++)
	{
		Z_seq[k] = k + x_seq;
		x_seq = k;
	}

#pragma omp parallel for
	for (int k = 0; k < N; k++)
	{
		if (k == 0)
			Z[k] = 1;
		else
			Z[k] = k + k - 1;
	}

	x = N - 1;
	if (x != x_seq)
		printf("Greska za x\n");

#pragma omp parallel for
	for (int i = 0; i < N; i++)
		if (Z[i] != Z_seq[i])
			printf("Greska za Z na lokaciji %d\n", i);
}